/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    USBUSER.C
 *      Purpose: USB Custom User Module
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC family microcontroller devices only. Nothing
 *      else gives you the right to use this software.
 *
 *      Copyright (c) 2005-2009 Keil Software.
 *---------------------------------------------------------------------------*/
#include <rtthread.h>
#include <drivers/usb_device.h>
#include "LPC407x_8x_177x_8x.h"
#include "lpc_types.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbuser.h"


/** @addtogroup USBDEV_MscUsbUser
 * @{
 */

static struct udcd __lpc_usbd;

/*
 *  USB Power Event Callback
 *   Called automatically on USB Power Event
 *    Parameter:       power: On(TRUE)/Off(FALSE)
 */

#if USB_POWER_EVENT
void USB_Power_Event(uint32_t  power)
{
}
#endif


/*
 *  USB Reset Event Callback
 *   Called automatically on USB Reset Event
 */

#if USB_RESET_EVENT
void USB_Reset_Event(void)
{
    rt_usbd_reset_handler(&__lpc_usbd);
}
#endif


/*
 *  USB Suspend Event Callback
 *   Called automatically on USB Suspend Event
 */

#if USB_SUSPEND_EVENT
void USB_Suspend_Event(void)
{
}
#endif


/*
 *  USB Resume Event Callback
 *   Called automatically on USB Resume Event
 */

#if USB_RESUME_EVENT
void USB_Resume_Event(void)
{
}
#endif


/*
 *  USB Remote Wakeup Event Callback
 *   Called automatically on USB Remote Wakeup Event
 */

#if USB_WAKEUP_EVENT
void USB_WakeUp_Event(void)
{
}
#endif


/*
 *  USB Start of Frame Event Callback
 *   Called automatically on USB Start of Frame Event
 */

#if USB_SOF_EVENT
void USB_SOF_Event(void)
{
    rt_usbd_sof_handler();
}
#endif


/*
 *  USB Error Event Callback
 *   Called automatically on USB Error Event
 *    Parameter:       error: Error Code
 */

#if USB_ERROR_EVENT
void USB_Error_Event(uint32_t error)
{
}
#endif


/*
 *  USB Set Configuration Event Callback
 *   Called automatically on USB Set Configuration Request
 */

#if USB_CONFIGURE_EVENT
void USB_Configure_Event(void)
{
    /* add your code here */
}
#endif


/*
 *  USB Set Interface Event Callback
 *   Called automatically on USB Set Interface Request
 */

#if USB_INTERFACE_EVENT
void USB_Interface_Event(void)
{
}
#endif


/*
 *  USB Set/Clear Feature Event Callback
 *   Called automatically on USB Set/Clear Feature Request
 */

#if USB_FEATURE_EVENT
void USB_Feature_Event(void)
{
}
#endif


#define P_EP(n) ((USB_EP_EVENT & (1 << (n))) ? USB_EndPoint##n : NULL)

/* USB Endpoint Events Callback Pointers */
void (* const USB_P_EP[16])(uint32_t event) =
{
    P_EP(0),
    P_EP(1),
    P_EP(2),
    P_EP(3),
    P_EP(4),
    P_EP(5),
    P_EP(6),
    P_EP(7),
    P_EP(8),
    P_EP(9),
    P_EP(10),
    P_EP(11),
    P_EP(12),
    P_EP(13),
    P_EP(14),
    P_EP(15),
};


/*
 *  USB Endpoint 1 Event Callback
 *   Called automatically on USB Endpoint 1 Event
 *    Parameter:       event
 */

void USB_EndPoint1(uint32_t event)
{
    switch (event)
    {
    case USB_EVT_OUT:
        rt_usbd_ep_out_handler(&__lpc_usbd, 0x1, 0);
        break;
    case USB_EVT_IN:
        rt_usbd_ep_in_handler(&__lpc_usbd, 0x81);
        break;
    }
}


/*
 *  USB Endpoint 2 Event Callback
 *   Called automatically on USB Endpoint 2 Event
 *    Parameter:       event
 */

void USB_EndPoint2(uint32_t event)
{
    switch (event)
    {
    case USB_EVT_OUT:
        rt_usbd_ep_out_handler(&__lpc_usbd, 0x2, 0);
        break;
    case USB_EVT_IN:
        rt_usbd_ep_in_handler(&__lpc_usbd, 0x82);
        break;
    }
}


/*
 *  USB Endpoint 3 Event Callback
 *   Called automatically on USB Endpoint 3 Event
 *    Parameter:       event
 */

void USB_EndPoint3(uint32_t event)
{
    switch (event)
    {
    case USB_EVT_OUT:
        rt_usbd_ep_out_handler(&__lpc_usbd, 0x3, 0);
        break;
    case USB_EVT_IN:
        rt_usbd_ep_in_handler(&__lpc_usbd, 0x83);
        break;
    }
}


/*
 *  USB Endpoint 4 Event Callback
 *   Called automatically on USB Endpoint 4 Event
 *    Parameter:       event
 */

void USB_EndPoint4(uint32_t event)
{
    switch (event)
    {
    case USB_EVT_OUT:
        rt_usbd_ep_out_handler(&__lpc_usbd, 0x4, 0);
        break;
    case USB_EVT_IN:
        rt_usbd_ep_in_handler(&__lpc_usbd, 0x84);
        break;
    }
}

/*
 *  USB Endpoint 5 Event Callback
 *   Called automatically on USB Endpoint 5 Event
 *    Parameter:       event
 */

void USB_EndPoint5(uint32_t event)
{
}


/*
 *  USB Endpoint 6 Event Callback
 *   Called automatically on USB Endpoint 6 Event
 *    Parameter:       event
 */

void USB_EndPoint6(uint32_t event)
{
}


/*
 *  USB Endpoint 7 Event Callback
 *   Called automatically on USB Endpoint 7 Event
 *    Parameter:       event
 */

void USB_EndPoint7(uint32_t event)
{
}


/*
 *  USB Endpoint 8 Event Callback
 *   Called automatically on USB Endpoint 8 Event
 *    Parameter:       event
 */

void USB_EndPoint8(uint32_t event)
{
}


/*
 *  USB Endpoint 9 Event Callback
 *   Called automatically on USB Endpoint 9 Event
 *    Parameter:       event
 */

void USB_EndPoint9(uint32_t event)
{
}


/*
 *  USB Endpoint 10 Event Callback
 *   Called automatically on USB Endpoint 10 Event
 *    Parameter:       event
 */

void USB_EndPoint10(uint32_t event)
{
}


/*
 *  USB Endpoint 11 Event Callback
 *   Called automatically on USB Endpoint 11 Event
 *    Parameter:       event
 */

void USB_EndPoint11(uint32_t event)
{
}


/*
 *  USB Endpoint 12 Event Callback
 *   Called automatically on USB Endpoint 12 Event
 *    Parameter:       event
 */

void USB_EndPoint12(uint32_t event)
{
}


/*
 *  USB Endpoint 13 Event Callback
 *   Called automatically on USB Endpoint 13 Event
 *    Parameter:       event
 */

void USB_EndPoint13(uint32_t event)
{
}


/*
 *  USB Endpoint 14 Event Callback
 *   Called automatically on USB Endpoint 14 Event
 *    Parameter:       event
 */

void USB_EndPoint14(uint32_t event)
{
}


/*
 *  USB Endpoint 15 Event Callback
 *   Called automatically on USB Endpoint 15 Event
 *    Parameter:       event
 */

void USB_EndPoint15(uint32_t event)
{
}

void USB_EndPoint0(uint32_t event)
{
    switch (event)
    {
    case USB_EVT_SETUP:
        rt_usbd_ep0_setup_handler(&__lpc_usbd, RT_NULL);
        break;
    case USB_EVT_OUT:
        rt_usbd_ep0_out_handler(&__lpc_usbd, 0);
        break;
    case USB_EVT_IN:
        rt_usbd_ep0_in_handler(&__lpc_usbd);
        break;
    default:
        break;
    }
}

static void __delay(void)
{
    int i;

    for (i = 0; i < 1000; i++);
}

static struct ep_id __ep_pool[] =
{
    {0x0,  USB_EP_ATTR_CONTROL,     USB_DIR_INOUT,  64, ID_ASSIGNED},
    {0x1,  USB_EP_ATTR_INT,         USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x2,  USB_EP_ATTR_BULK,        USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0x2,  USB_EP_ATTR_BULK,        USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x4,  USB_EP_ATTR_BULK,        USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0x4,  USB_EP_ATTR_BULK,        USB_DIR_IN,     64, ID_UNASSIGNED},
    {0x6,  USB_EP_ATTR_BULK,        USB_DIR_OUT,    64, ID_UNASSIGNED},
    {0x6,  USB_EP_ATTR_BULK,        USB_DIR_IN,     64, ID_UNASSIGNED},
    {0xFF, USB_EP_ATTR_TYPE_MASK,   USB_DIR_MASK,   0,  ID_ASSIGNED},
};

static rt_err_t __ep_set_stall(rt_uint8_t address)
{
    RT_ASSERT(address != 0);

    RT_DEBUG_LOG(RT_DEBUG_USB, ("ep set_stall, address 0x%x\n", address));

    USB_SetStallEP(address);

    return RT_EOK;
}

static rt_err_t __ep_clear_stall(rt_uint8_t address)
{
    RT_ASSERT(address != 0);

    RT_DEBUG_LOG(RT_DEBUG_USB, ("ep clear_stall, address 0x%x\n", address));

    USB_ClrStallEP(address);

    return RT_EOK;
}

static rt_err_t __set_address(rt_uint8_t address)
{
    USB_SetAddress(address);

    return RT_EOK;
}

static rt_err_t __set_config(rt_uint8_t address)
{
    USB_Configure(TRUE);

    return RT_EOK;
}

static rt_err_t __ep_enable(uep_t ep)
{
    RT_ASSERT(ep != RT_NULL);
    RT_ASSERT(ep->ep_desc != RT_NULL);

    USB_ConfigEP(ep->ep_desc);
    USB_EnableEP(EP_ADDRESS(ep));
    USB_ResetEP(EP_ADDRESS(ep));

    return RT_EOK;
}

static rt_err_t __ep_disable(uep_t ep)
{
    RT_ASSERT(ep != RT_NULL);
    RT_ASSERT(ep->ep_desc != RT_NULL);

    USB_DisableEP(EP_ADDRESS(ep));

    return RT_EOK;
}

static rt_size_t __ep_read(rt_uint8_t address, void *buffer)
{
    rt_size_t size = 0;

    RT_ASSERT(buffer != RT_NULL);

    size = USB_ReadEP(address, buffer);

    return size;
}

static rt_size_t __ep_write(rt_uint8_t address, void *buffer, rt_size_t size)
{
    USB_WriteEP(address, buffer, size);

    return size;
}

static rt_err_t __ep0_send_status(void)
{
    USB_WriteEP(0x80, RT_NULL, 0);

    /*It's a controller limition */
    __delay();

    return RT_EOK;
}

static rt_err_t __suspend(void)
{
    return RT_EOK;
}

static rt_err_t __wakeup(void)
{
    return RT_EOK;
}

static rt_err_t __init(rt_device_t device)
{
    USB_Init();
    USB_Connect(TRUE);

    return RT_EOK;
}

static struct udcd_ops __lpc_usbd_ops =
{
    __set_address,
    __set_config,
    __ep_set_stall,
    __ep_clear_stall,
    __ep_enable,
    __ep_disable,
    RT_NULL,
    __ep_read,
    __ep_write,
    __ep0_send_status,
    __suspend,
    __wakeup,
};

rt_err_t lpc_usbd_register(void)
{
    rt_memset((void *)&__lpc_usbd, 0, sizeof(struct udcd));

    __lpc_usbd.parent.type = RT_Device_Class_USBDevice;
    __lpc_usbd.parent.init = __init;

    __lpc_usbd.ops = &__lpc_usbd_ops;

    /* Register endpoint infomation */
    __lpc_usbd.ep_pool = __ep_pool;
    __lpc_usbd.ep0.id = &__ep_pool[0];

    rt_device_register(&__lpc_usbd.parent, "usbd", 0);

    return RT_EOK;
}

/**
 * @}
 */
