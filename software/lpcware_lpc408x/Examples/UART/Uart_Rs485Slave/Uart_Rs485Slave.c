/**********************************************************************
* $Id$      Uart_Rs485Slave.c           2011-06-02
*//**
* @file     Uart_Rs485Slave.c
* @brief    This example used to test RS485 functionality on UART1 of
*           LPC1768.In this case, RS485 function on UART1 acts as SLave
*           on RS485 bus.
* @version  1.0
* @date     02. June. 2011
* @author   NXP MCU SW Application Team
* 
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/
#include "bsp.h"
#include "lpc_uart.h"
#include "lpc_pinsel.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup UART_RS485_Slave  UART RS485 Slave
 * @ingroup UART_RS485_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
/*
 * These following defines can be modified:
 * - RECEIVER_ALWAYS_EN (0/1)
 * - AUTO_SLVADDR_DETECT (0/1) in case RECEIVER_ALWAYS_EN is set to 0
 */

/* Receiver always be enabled to receive any data frame on RS485 bus,
 * regardless that frame is data frame or slave address frame (9-bit).
 * - When receiving a data frame, slave will display that data frame content
 * via UART0.
 * - When receiving a slave address frame (9bit mode), line error interrupt
 * - 0: Receiver is not always enabled, only slave address frame can trigger
 * an interrupt event to allow slave handle.
 * - 1: Receiver always be enabled */
#define RECEIVER_ALWAYS_EN  0

#define UART_TEST_NUM       2
#define _LPC_UART       UART_2
#define _UART_IRQ       UART2_IRQn
#define _UART_IRQHander     UART2_IRQHandler


#if (RECEIVER_ALWAYS_EN == 0)
/* Enable/Disable Auto Slave Address Detection
 * - In case of '0': any received data bytes will be ignored and will not
 * be stored in the RXFIFO. When an address byte is detected (parity bit
 * = '1') it will be placed into the RXFIFO and an Rx Data Ready Interrupt
 * will be generated. The interrupt handler can then read the address byte
 * and decide whether or not to enable the receiver to accept the following data.
 * - In case of '1': any received byte will be discarded if it is either a
 * data byte OR an address byte which fails to match the slave address configured
 * when initializing RS485. When a matching address character is detected it will
 * be pushed onto the RXFIFO along with the parity bit, and the receiver will
 * be automatically enabled (RS485CTRL bit 1 will be cleared by hardware).
 * The receiver will also generate an Rx Data Ready Interrupt */
#define AUTO_SLVADDR_DETECT 1
#define SLAVE_ADDR 'A'
#endif

/* buffer size definition */
#define UART_RING_BUFSIZE 256

/* Buf mask */
#define __BUF_MASK (UART_RING_BUFSIZE-1)
/* Check buf is full or not */
#define __BUF_IS_FULL(head, tail) ((tail&__BUF_MASK)==((head+1)&__BUF_MASK))
/* Check buf will be full in next receiving or not */
#define __BUF_WILL_FULL(head, tail) ((tail&__BUF_MASK)==((head+2)&__BUF_MASK))
/* Check buf is empty */
#define __BUF_IS_EMPTY(head, tail) ((head&__BUF_MASK)==(tail&__BUF_MASK))
/* Reset buf */
#define __BUF_RESET(bufidx) (bufidx=0)
#define __BUF_INCR(bufidx)  (bufidx=(bufidx+1)&__BUF_MASK)

/************************** PRIVATE TYPES *************************/
/** @brief UART Ring buffer structure */
typedef struct
{
    __IO uint32_t tx_head;                /*!< UART Tx ring buffer head index */
    __IO uint32_t tx_tail;                /*!< UART Tx ring buffer tail index */
    __IO uint32_t rx_head;                /*!< UART Rx ring buffer head index */
    __IO uint32_t rx_tail;                /*!< UART Rx ring buffer tail index */
    __IO uint8_t  tx[UART_RING_BUFSIZE];  /*!< UART Tx data ring buffer */
    __IO uint8_t  rx[UART_RING_BUFSIZE];  /*!< UART Rx data ring buffer */
} UART_RING_BUFFER_T;

/************************** PRIVATE VARIABLES *************************/
#if (RECEIVER_ALWAYS_EN)
uint8_t menu1[] =
"Hello NXP Semiconductors \n\r" \
"RS485 demo in Slave mode \n\r" \
"Slave's Receiver always enabled \n\r";
#else
#if (AUTO_SLVADDR_DETECT == 0)
uint8_t menu1[] =
"Hello NXP Semiconductors \n\r" \
"RS485 demo in Slave mode \n\r" \
"Slave's Receiver is not always enabled - Auto Address Detection is disabled\n\r";
#else
uint8_t menu1[] =
"Hello NXP Semiconductors \n\r" \
"RS485 demo in Slave mode \n\r" \
"Slave's Receiver is not always enabled - Auto Address Detection is enabled\n\r";
#endif
#endif

uint8_t send_msg[] = "Sending... \n\r";
uint8_t recv_msg[] = "Receive: ";
uint8_t p_err_menu[] = "Parity error";
uint8_t addr_menu[] = " - Slv Addr Frm received\n\r";
uint8_t addr_acc[] = " - Slave Addr accepted\n\r";
uint8_t addr_una[] = " - Slave Addr unaccepted\n\r";
uint8_t addr_auto[] = "Slave Addr detected!\n\r";
uint8_t f_err_menu[] = "Frame error \n\r";
uint8_t nextline[] = "\n\r\n\r";

uint8_t ack_msg[] = "ACK";
uint8_t terminator = 13;

// UART Ring buffer
UART_RING_BUFFER_T rb;

/************************** PRIVATE FUNCTIONS *************************/
void UART1_IRQHandler(void);
void UART2_IRQHandler(void);

void UART_IntReceive(void);
void UART_IntErr(uint8_t bLSErrType);

uint32_t UARTReceive(UART_ID_Type UartID, uint8_t *rxbuf, uint8_t buflen);
void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/

/*********************************************************************//**
 * @brief       UART1 interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void _UART_IRQHander(void)
{
    uint32_t intsrc, tmp, tmp1;

    /* Determine the interrupt source */
    intsrc = UART_GetIntId(_LPC_UART);
    
    tmp = intsrc & UART_IIR_INTID_MASK;

    // Receive Line Status
    if (tmp == UART_IIR_INTID_RLS)
    {
        // Check line status
        tmp1 = UART_GetLineStatus(_LPC_UART);

        // Mask out the Receive Ready and Transmit Holding empty status
        tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE \
                                | UART_LSR_BI | UART_LSR_RXFE);

        // If any error exist
        if (tmp1) 
        {
            UART_IntErr(tmp1);
        }
    }

    // Receive Data Available or Character time-out
    if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI))
    {
        UART_IntReceive();
    }

}



/********************************************************************//**
 * @brief       UART receive function (ring buffer used)
 * @param[in]   None
 * @return      None
 *********************************************************************/
void UART_IntReceive(void)
{
#if (RECEIVER_ALWAYS_EN)
    uint8_t tmpc;
    uint32_t rLen;

    while(1)
    {
        // Call UART read function in UART driver
        rLen = UART_Receive(_LPC_UART, &tmpc, 1, NONE_BLOCKING);

        // If data received
        if (rLen)
        {
            /* Check if buffer is more space
            * If no more space, remaining character will be trimmed out
            */
            if (!__BUF_IS_FULL(rb.rx_head,rb.rx_tail))
            {
                rb.rx[rb.rx_head] = tmpc;
                __BUF_INCR(rb.rx_head);
            }
        }
        // no more data
        else 
        {
            break;
        }
    }
#else

#if (AUTO_SLVADDR_DETECT == 0)
    uint8_t tmpc;
    uint32_t rLen;

    while(1)
    {
        // Call UART read function in UART driver
        rLen = UART_Receive(_LPC_UART, &tmpc, 1, NONE_BLOCKING);

        // If data received
        if (rLen)
        {
            /* Check if buffer is more space
            * If no more space, remaining character will be trimmed out
            */
            if (!__BUF_IS_FULL(rb.rx_head,rb.rx_tail))
            {
                rb.rx[rb.rx_head] = tmpc;
                __BUF_INCR(rb.rx_head);
            }
        }
        // no more data
        else 
        {
            break;
        }
    }
#else
    uint8_t tmpc;
    uint32_t rLen;

    while(1)
    {
        // Call UART read function in UART driver
        rLen = UART_Receive(_LPC_UART, &tmpc, 1, NONE_BLOCKING);

        // If data received
        if (rLen)
        {
            /* Check if buffer is more space
            * If no more space, remaining character will be trimmed out
            */
            if (!__BUF_IS_FULL(rb.rx_head,rb.rx_tail))
            {
                rb.rx[rb.rx_head] = tmpc;
                __BUF_INCR(rb.rx_head);
            }
        }
        // no more data
        else 
        {
            break;
        }
    }
#endif
#endif
}


/*********************************************************************//**
 * @brief       UART Line Status Error
 * @param[in]   bLSErrType  UART Line Status Error Type
 * @return      None
 **********************************************************************/
void UART_IntErr(uint8_t bLSErrType)
{
    uint8_t tmp;
#if (RECEIVER_ALWAYS_EN)
    uint8_t tmpc;

    if (bLSErrType & UART_LSR_PE)
    {
        // Parity error means the latest frame receive is slave address frame,
        // Value of slave address is read and trimmed out.
        UART_Send(UART_0, p_err_menu, sizeof(p_err_menu), BLOCKING);

        UART_Send(UART_0, addr_menu, sizeof(addr_menu), BLOCKING);

        UART_Receive(_LPC_UART, &tmpc, 1, NONE_BLOCKING);
    }

    if (bLSErrType & UART_LSR_FE)
    {
        UART_Send(UART_0, f_err_menu, sizeof(f_err_menu), BLOCKING);
    }
#else
#if (AUTO_SLVADDR_DETECT == 0)
    uint8_t tmp;

    // Check if this interrupt caused by parity error,
    // that means the last received frame is address frame,
    // if this address is matched with its own address,
    // continue to receive following data frame.

    if (bLSErrType & UART_LSR_PE)
    {
        UART_Receive(_LPC_UART, &tmp, 1, NONE_BLOCKING);

        UART_Send(UART_0, p_err_menu, sizeof(p_err_menu), BLOCKING);

        if (tmp == SLAVE_ADDR)
        {
            UART_RS485ReceiverCmd(UART_RS485, ENABLE);
            
            UART_Send(UART_0, addr_acc, sizeof(addr_acc), BLOCKING);
        } 
        else 
        {
            // Disable receiver
            UART_RS485ReceiverCmd(UART_RS485, DISABLE);
            
            UART_Send(UART_0, addr_una, sizeof(addr_una), BLOCKING);
        }
    }
#else

    // Check if this interrupt caused by parity error,
    // that means the last received frame is address frame,
    // if this address is matched with its own address,
    // continue to receive following data frame.
    if (bLSErrType & UART_LSR_PE)
    {
        UART_Receive(_LPC_UART, &tmp, 1, NONE_BLOCKING);

        UART_Send(UART_0, addr_auto, sizeof(addr_auto), BLOCKING);
    }
#endif
#endif
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       UART read function for interrupt mode (using ring buffers)
 * @param[in]   UartID  Selected UART peripheral used to send data,
 *              should be UART0
 * @param[out]  rxbuf Pointer to Received buffer
 * @param[in]   buflen Length of Received buffer
 * @return      Number of bytes actually read from the ring buffer
 **********************************************************************/
uint32_t UARTReceive(UART_ID_Type UartID, uint8_t *rxbuf, uint8_t buflen)
{
    uint8_t *data = (uint8_t *) rxbuf;
    uint32_t bytes = 0;

    /* Temporarily lock out UART receive interrupts during this
    read so the UART receive interrupt won't cause problems
    with the index values */
    UART_IntConfig(UartID, UART_INTCFG_RBR, DISABLE);

    /* Loop until receive buffer ring is empty or
    until max_bytes expires */
    while ((buflen > 0) && (!(__BUF_IS_EMPTY(rb.rx_head, rb.rx_tail))))
    {
        /* Read data from ring buffer into user buffer */
        *data = rb.rx[rb.rx_tail];
        data++;

        /* Update tail pointer */
        __BUF_INCR(rb.rx_tail);

        /* Increment data count and decrement buffer size count */
        bytes++;
        buflen--;
    }

    /* Re-enable UART interrupts */
    UART_IntConfig(UartID, UART_INTCFG_RBR, ENABLE);

    return bytes;
}

/*********************************************************************//**
 * @brief       Print Welcome menu
 * @param[in]   none
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    UART_Send(UART_0, menu1, sizeof(menu1), BLOCKING);
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main UART-RS485 program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    // UART Configuration structure variable
    UART_CFG_Type UARTConfigStruct;
    
    // UART FIFO configuration Struct variable
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;

    // RS485 configuration
    UART1_RS485_CTRLCFG_Type rs485cfg;
    uint32_t idx, len;
    uint8_t buffer[10];
    volatile uint32_t tmp;

    // UART0 section ----------------------------------------------------
    // Initialize UART0 pin connect

    PINSEL_ConfigPin(0, 2, 1);

    PINSEL_ConfigPin(0, 3, 1);

    /* Initialize UART Configuration parameter structure to default state:
    * Baudrate = 115200 bps
    * 8 data bit
    * 1 Stop bit
    * None parity
    */
    UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize UART0 peripheral with given to corresponding parameter
    UART_Init(UART_0, &UARTConfigStruct);

    /* Initialize FIFOConfigStruct to default state:
    *               - FIFO_DMAMode = DISABLE
    *               - FIFO_Level = UART_FIFO_TRGLEV0
    *               - FIFO_ResetRxBuf = ENABLE
    *               - FIFO_ResetTxBuf = ENABLE
    *               - FIFO_State = ENABLE
    */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig(UART_0, &UARTFIFOConfigStruct);

    // Enable UART Transmit
    UART_TxCmd(UART_0, ENABLE);

    // print welcome screen
    print_menu();

#if (_CURR_USING_OEM_BRD == LPC4088_OEM_BOARD)
    /*
     * Initialize UART2 pin connect
     * P4.22: U2_TXD
     * P4.23: U2_RXD
     */
    PINSEL_ConfigPin(4,22,2);
    PINSEL_ConfigPin(4,23,2);
#else
    // UART1 - RS485 section -------------------------------------------------
    // Initialize UART1 pin connect

    //TXD2
    PINSEL_ConfigPin(0, 10, 1);

    //RXD2
    PINSEL_ConfigPin(0, 11, 1);
#endif
    //OE2: UART OE2 Output Enable for UART2
    PINSEL_ConfigPin(1, 19, 6); 

    /* Initialize UART Configuration parameter structure to default state:
    * Baudrate = 9600 bps
    * 8 data bit
    * 1 Stop bit
    * Parity: None
    * Note: Parity will be enabled later in UART_RS485Config() function.
    */
    UART_ConfigStructInit(&UARTConfigStruct);
    UARTConfigStruct.Baud_rate = 9600;

    // Initialize UART0 peripheral with given to corresponding parameter
    UART_Init(_LPC_UART, &UARTConfigStruct);

    /* Initialize FIFOConfigStruct to default state:
    *               - FIFO_DMAMode = DISABLE
    *               - FIFO_Level = UART_FIFO_TRGLEV0
    *               - FIFO_ResetRxBuf = ENABLE
    *               - FIFO_ResetTxBuf = ENABLE
    *               - FIFO_State = ENABLE
    */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig(_LPC_UART, &UARTFIFOConfigStruct);

    // Configure RS485
    /*
    * - Auto Direction in Tx/Rx driving is enabled
    * - Direction control pin is set to DTR1
    * - Direction control pole is set to "1" that means direction pin
    * will drive to high state before transmit data.
    * - Multidrop mode is enable
    * - Auto detect address is disabled
    * - Receive state is enable
    */
    rs485cfg.AutoDirCtrl_State = ENABLE;
    rs485cfg.DirCtrlPin = UART_RS485_DIRCTRL_DTR;
    rs485cfg.DirCtrlPol_Level = SET;
    rs485cfg.DelayValue = 50;
    rs485cfg.NormalMultiDropMode_State = ENABLE;
#if AUTO_SLVADDR_DETECT
    rs485cfg.AutoAddrDetect_State = ENABLE;
    rs485cfg.MatchAddrValue = SLAVE_ADDR;
#else
    rs485cfg.AutoAddrDetect_State = DISABLE;
#endif

#if RECEIVER_ALWAYS_EN
    rs485cfg.Rx_State = ENABLE;
#else
    rs485cfg.Rx_State = DISABLE;
#endif

    UART_RS485Config(_LPC_UART, &rs485cfg);

    /* Enable UART Rx interrupt */
    UART_IntConfig(_LPC_UART, UART_INTCFG_RBR, ENABLE);

    /* Enable UART line status interrupt */
    UART_IntConfig(_LPC_UART, UART_INTCFG_RLS, ENABLE);

    // Priorities settings for UART RS485: here we use UART2 for RS485 communication
    // They should be changed if using another UART
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(_UART_IRQ, ((0x01<<3)|0x01));

    /* Enable Interrupt for UART0 channel */
    NVIC_EnableIRQ(_UART_IRQ);
    
    // Enable UART Transmit
    UART_TxCmd(_LPC_UART, ENABLE);

    // for testing...
    while (1)
    {       
        len = 0;

        while (len == 0)
        {
            len = UARTReceive(_LPC_UART, buffer, sizeof(buffer));
        }

        /* Got some data */
        idx = 0;

        while (idx < len)
        {
            if (buffer[idx] == 13)
            {
                for (tmp = 0; tmp < 1000000; tmp++);
                
                UART_RS485SendData(_LPC_UART, ack_msg, sizeof(ack_msg));

                UART_Send(UART_0, nextline, sizeof(nextline), BLOCKING);

                UART_RS485SendData(_LPC_UART, &terminator, 1);
            } 
            else 
            {
                /* Echo it back */
                UART_Send(UART_0, &buffer[idx], 1, BLOCKING);
            }
            
            idx++;
        }
    }

}

/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
int main(void)
{
    c_entry();
    return 0;
}


/**
 * @}
 */
