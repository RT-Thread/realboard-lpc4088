/*
 * File      : ohci.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2011, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2012-01-10     Yi Qiu      first version
 */

#include <rtthread.h>
#include <drivers/usb_host.h>
#include <rtdevice.h>
#include "board.h"

/*
    The HcControl register defines the operating modes for the Host Controller.  Most of the fields in
    this register are modified only by the Host Controller Driver, except
    HostControllerFunctionalState and RemoteWakeupConnected.
*/
#define CTRL_CBSR                0x00000003UL        /* ControlBulkServiceRatio        */
#define CTRL_PLE                0x00000004UL        /* PeriodicListEnable            */
#define CTRL_IE                    0x00000008UL        /* IsochronousEnable            */
#define CTRL_CLE                0x00000010UL        /* ControlListEnable            */
#define CTRL_BLE                0x00000020UL        /* BulkListEnable                */
#define CTRL_HCFS                0x000000C0UL        /* HostControllerFunctionalState    */
#define CTRL_HCFS_RESET            0x00000000UL        /* 00b: USBRESET             */
#define CTRL_HCFS_RESUME        0x00000040UL        /* 01b: USBRESUME             */
#define CTRL_HCFS_OPER            0x00000080UL        /* 10b: USBOPERATIONAL         */
#define CTRL_HCFS_SUSPEND        0x000000C0UL        /* 11b: USBSUSPEND             */
#define CTRL_IR                    0x00000100UL        /* InterruptRouting            */
#define CTRL_RWC                0x00000200UL        /* RemoteWakeupConnected    */
#define CTRL_RWE                0x00000400UL        /* RemoteWakeupEnable        */

#define CS_HCR                    0x00000001UL    /* HostControllerReset            */
#define CS_CLF                    0x00000002UL    /* ControlListFilled                */
#define CS_BLF                    0x00000004UL    /* BulkListFilled                */
#define CS_OCR                    0x00000008UL    /* OwnershipChangeRequest    */
#define CS_SOC                    0x00030000UL    /* SchedulingOverrunCount        */

/*
    The HcRhDescriptorA register is the first register of two describing the characteristics of the
    Root Hub. Reset values are implementation-specific.  The descriptor length (11), descriptor type
    (TBD), and hub controller current (0) fields of the hub Class Descriptor are emulated by the
    HCD.  All other fields are located in the HcRhDescriptorA and HcRhDescriptorB registers.
*/
#define RHD_NDP                    0x00000000UL    /* NumberDownstreamPorts     */
#define RHD_NPS                    0x00000100UL    /* NoPowerSwitching             */
#define RHD_PSM                    0x00000200UL    /* PowerSwitchingMode            */
#define RHD_DT                    0x00000400UL    /* DeviceType                */
#define RHD_OCPM                0x00000800UL    /* OverCurrentProtectionMode    */
#define RHD_NOCP                0x00001000UL    /* NoOverCurrentProtection        */
#define RHD_POTPGT                0x00000000UL    /* PowerOnToPowerGoodTime    */

/*
    The HcRhStatus register is divided into two parts.  The lower word of a Dword represents the
    Hub Status field  and the upper word represents the Hub Status Change field.  Reserved bits
    should always be written '0'.
*/

#define RHS_LPS                    0x00000001UL    /* R: Local Power Status -W: Clear Global Power    */
#define RHS_LPSC                0x00010000UL    /* R: Local Power Status Change - W: Set Global Power */
#define RHS_DRWE                   0x00008000UL    /* W: Set Remote Wakeup Enable                 */

/*
    This register provides status on various events that cause hardware interrupts.  When an event
    occurs, Host Controller sets the corresponding bit in this register.  When a bit becomes set, a
    hardware interrupt is generated if the interrupt is enabled in the HcInterruptEnable register (see
    Section 7.1.5) and the MasterInterruptEnable bit is set.  The Host Controller Driver may clear
    specific bits in this register by writing ¡®1¡¯ to bit positions to be cleared.  The Host Controller
    Driver may not set any of these bits.  The Host Controller will never clear the bit.
*/
#define INT_SO                    0x00000001UL    /* Scheduling Overrun            */
#define INT_WD                    0x00000002UL    /* Writeback DoneHead            */
#define INT_SOF                    0x00000004UL    /* Start of Frame                */
#define INT_RD                    0x00000008UL    /* Resume Detect                */
#define INT_UE                    0x00000010UL    /* Unrecoverable error            */
#define INT_FNO                    0x00000020UL    /* Frame Number Overflow        */
#define INT_RHSC                0x00000040UL    /* Root Hub Status Change        */
#define INT_OC                    0x40000000UL    /* Ownership Change            */
#define INT_MIE                    0x80000000UL    /* Master Interrupt Enable        */
#define INT_ALL                    0xC000007FUL    /* All interrupts                */

#define LSTHRESH                0x00000628UL
#define PERIODIC_START            0x00002a27UL    /* 10% off from FRAME_INTERVAL */
#define FRAME_INTERVAL            0x00002edfUL    /* Reset default value */
#define FMINTERVAL_DEFAULT        ((((6 * (FRAME_INTERVAL - 210)) / 7) << 16) | FRAME_INTERVAL)

/* TD info field */
#define TD_CC                    0xf0000000
#define TD_EC                    0x0C000000
#define TD_T                    0x03000000
#define TD_T_DATA0                0x02000000
#define TD_T_DATA1                0x03000000
#define TD_T_TOGGLE                0x00000000
#define TD_R                    0x00040000
#define TD_DI                    0x00E00000
#define TD_DP                    0x00180000
#define TD_DP_SETUP                0x00000000
#define TD_DP_IN                0x00100000
#define TD_DP_OUT                0x00080000
#define TD_ISO                    0x00010000
#define TD_DEL                    0x00020000

/* CC Codes */
#define TD_CC_NOERROR            0x00000000
#define TD_CC_CRC                0x00000001
#define TD_CC_BITSTUFFING        0x00000002
#define TD_CC_DATATOGGLEM        0x00000003
#define TD_CC_STALL                0x00000004
#define TD_DEVNOTRESP            0x00000005
#define TD_PIDCHECKFAIL            0x00000006
#define TD_UNEXPECTEDPID        0x00000007
#define TD_DATAOVERRUN            0x00000008
#define TD_DATAUNDERRUN            0x00000009
#define TD_BUFFEROVERRUN        0x0000000C
#define TD_BUFFERUNDERRUN        0x0000000D
#define TD_NOTACCESSED            0x0000000F

struct ohci_regs
{
    volatile rt_uint32_t revision;                /* HcRevision register            */
    volatile rt_uint32_t control;                /* HcControl register             */
    volatile rt_uint32_t cmdstatus;                /* HcCommandStatus register    */
    volatile rt_uint32_t intrstatus;            /* HcInterruptStatus register        */
    volatile rt_uint32_t intrenable;            /* HcInterruptEnable register    */
    volatile rt_uint32_t intrdisable;            /* HcInterruptDisable register    */

    volatile rt_uint32_t hcca;                    /* HcHCCA register             */
    volatile rt_uint32_t ed_periodcurrent;        /* HcPeriodCurrentED register     */
    volatile rt_uint32_t ed_controlhead;        /* HcControlHeadED register     */
    volatile rt_uint32_t ed_controlcurrent;        /* HcControlCurrentED register     */
    volatile rt_uint32_t ed_bulkhead;            /* HcBulkHeadED register         */
    volatile rt_uint32_t ed_bulkcurrent;        /* HcBulkCurrentED register     */
    volatile rt_uint32_t donehead;                /* HcDoneHead register         */

    volatile rt_uint32_t fminterval;            /* HcFmInterval register         */
    volatile rt_uint32_t fmremaining;            /* HcFmRemaining register         */
    volatile rt_uint32_t fmnumber;                /* HcFmNumber register         */
    volatile rt_uint32_t periodicstart;            /* HcPeriodicStart register         */
    volatile rt_uint32_t lsthresh;                /* HcLSThreshold register         */

    volatile rt_uint32_t roothub_a;                /* HcRhDescriptorA register         */
    volatile rt_uint32_t roothub_b;                /* HcRhDescriptorB register         */
    volatile rt_uint32_t roothub_status;        /* HcRhStatus register             */
    volatile rt_uint32_t roothub_portstatus[15];/* HcRhPortStatus register         */
};

struct hcca
{
    volatile rt_uint32_t inttbl[32];
    volatile rt_uint16_t fmno;
    volatile rt_uint16_t pad1;
    volatile rt_uint32_t donehead;
    volatile rt_uint8_t reserved[116];
};

typedef union
{
    struct
    {
        volatile rt_uint32_t func_addr        : 7;
        volatile rt_uint32_t ep_number        : 4;
        volatile rt_uint32_t dir             : 2;
        volatile rt_uint32_t speed            : 1;
        volatile rt_uint32_t skip            : 1;
        volatile rt_uint32_t format            : 1;
        volatile rt_uint32_t max_packet_size : 11;
        volatile rt_uint32_t resevered        : 4;
    } bits;
    volatile rt_uint32_t value;
} ed_control;

typedef union
{
    struct
    {
        volatile rt_uint32_t resevered        : 18;
        volatile rt_uint32_t buffer_rounding : 1;
        volatile rt_uint32_t dir_pid         : 2;
        volatile rt_uint32_t delay_interrupt : 3;
        volatile rt_uint32_t data_toggle     : 2;
        volatile rt_uint32_t error_count     : 2;
        volatile rt_uint32_t condition_code    : 4;
    } bits;
    volatile rt_uint32_t value;
} td_control;

struct ohci_ed
{
    /* host controllor descriptor */
    ed_control ctl;
    volatile rt_uint32_t tailp;
    volatile rt_uint32_t headp;
    volatile rt_uint32_t nexted;

    /* host controllor driver */
    rt_uint8_t ep_attr : 4;            /* endpoint attribute */
    rt_uint8_t status : 4;            /* ed status */
    rt_uint8_t err_code;            /* direction */
    rt_uint8_t interval;
    rt_uint8_t reserved;

    rt_uint32_t pipe;
    rt_int32_t xfer_len;
};

struct ohci_td
{
    /* host controllor descriptor */
    td_control ctl;
    volatile rt_uint32_t cbp;
    volatile rt_uint32_t nexttd;
    volatile rt_uint32_t be;

    /* host controllor driver */
    struct ohci_ed *ed;
    rt_uint32_t data;
    rt_uint8_t status;
    rt_uint8_t index;
    rt_uint8_t reserved[2];
};

#define OHCI_ED_NODE_SIZE            32
#define OHCI_TD_NODE_SIZE            32

struct ohci_data
{
    /* ohci's hcca memory, should align with 256 bytes */
    volatile struct hcca *hcca;

    /* data node memory pool */
    struct rt_mempool ed_mp;
    struct rt_mempool td_mp;

    /* the ed for control transfer */
    struct ohci_ed *ctl_ed;

    /* ohci host controller register */
    volatile struct ohci_regs *hc;

    struct uhub roothub;

    rt_uint8_t *tx_buffer;
    rt_uint8_t *rx_buffer;

    rt_uint8_t int_req;

    /* to notify transfer done */
    struct rt_completion ctl_comp;
    struct rt_completion bulk_comp;
    struct rt_completion int_comp;

    /* to lock critical resourse */
    struct rt_semaphore sem_lock;
};

void ohci_config(rt_uint32_t regbase, rt_uint32_t rambase, rt_uint32_t port_num);
void ohci_usbh_register(const char *name);
void ohci_irq_handler(void);

