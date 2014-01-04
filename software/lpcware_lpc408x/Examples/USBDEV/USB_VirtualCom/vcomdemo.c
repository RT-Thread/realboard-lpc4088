/*----------------------------------------------------------------------------
 *      Name:    vcomdemo.c
 *      Purpose: USB virtual COM port Demo
 *      Version: V1.20
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC microcontroller devices only. Nothing else
 *      gives you the right to use this software.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */

#ifdef _USB_DEV_VIRTUAL_COM
#include "LPC407x_8x_177x_8x.h"
#include "lpc_types.h"
#include "usbreg.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "cdc.h"
#include "cdcuser.h"
#include "serial.h"
#include "vcomdemo.h"


/** @defgroup USBDEV_VirtualCom USB Virtual COM Port Device
 * @ingroup USBDEV_Examples
 * @{
 */

/** @defgroup USBDEV_UsbHw  USB-VirtualCOM USB Hardware
 * @ingroup USBDEV_VirtualCom
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_SerialFunc USB-VirtualCOM Serial Function
 * @ingroup USBDEV_VirtualCom
 * @{
 */

/**
 * @}
 */

/** @defgroup USBDEV_UsbCore    USB-VirtualCOM USB Core
 * @ingroup USBDEV_VirtualCom
 * @{
 */

/**
 * @}
 */

/** @defgroup USBDEV_UsbDesc    USB-VirtualCOM USB Descriptors
 * @ingroup USBDEV_VirtualCom
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_UsbReg USB-VirtualCOM USB Register
 * @ingroup USBDEV_VirtualCom
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_UsbUser    USB-VirtualCOM USB User
 * @ingroup USBDEV_VirtualCom
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_UsbCfg USB-VirtualCOM USB Configuration
 * @ingroup USBDEV_VirtualCom
 * @{
 */

/**
 * @}
 */

/** @defgroup USBDEV_Cdc    USB-VirtualCOM CDC
 * @ingroup USBDEV_VirtualCom
 * @{
 */

/**
 * @}
 */
 

/** @defgroup USBDEV_CdcUser    USB-VirtualCOM CDC User
 * @ingroup USBDEV_VirtualCom
 * @{
 */

/**
 * @}
 */



/*----------------------------------------------------------------------------
 Initialises the VCOM port.
 Call this function before using VCOM_putchar or VCOM_getchar
 *---------------------------------------------------------------------------*/
void VCOM_Init(void) 
{
#if PORT_NUM
    CDC_Init (1);
#else
    CDC_Init (0);
#endif
}


/*----------------------------------------------------------------------------
  Reads character from serial port buffer and writes to USB buffer
 *---------------------------------------------------------------------------*/
void VCOM_Serial2Usb(void) 
{
    static char serBuf [USB_CDC_BUFSIZE];
    int  numBytesRead, numAvailByte;

    ser_AvailChar (&numAvailByte);
    
    if (numAvailByte > 0) 
    {
        if (CDC_DepInEmpty) 
        {
            numBytesRead = ser_Read (&serBuf[0], &numAvailByte);

            CDC_DepInEmpty = 0;
            USB_WriteEP (CDC_DEP_IN, (unsigned char *)&serBuf[0], numBytesRead);
        }
    }

}

/*----------------------------------------------------------------------------
  Reads character from USB buffer and writes to serial port buffer
 *---------------------------------------------------------------------------*/
void VCOM_Usb2Serial(void) 
{
  static char serBuf [32];
         int  numBytesToRead, numBytesRead, numAvailByte;

  CDC_OutBufAvailChar (&numAvailByte);
  
  if (numAvailByte > 0) 
  {
      numBytesToRead = numAvailByte > 64 ? 64 : numAvailByte;
      numBytesRead = CDC_RdOutBuf (&serBuf[0], &numBytesToRead);
#if PORT_NUM
      ser_Write (1, &serBuf[0], &numBytesRead);
#else
      ser_Write (0, &serBuf[0], &numBytesRead);
#endif
      /* reenable endpoint interrupt to receive other data */
      LPC_USB->DevIntEn |= EP_SLOW_INT;
  }

}


/*----------------------------------------------------------------------------
  checks the serial state and initiates notification
 *---------------------------------------------------------------------------*/
void VCOM_CheckSerialState (void) 
{
    unsigned short temp;
    static unsigned short serialState;

    temp = CDC_GetSerialState();
    if (serialState != temp) 
    {
        serialState = temp;
        CDC_NotificationIn();                  // send SERIAL_STATE notification
    }
}

/*----------------------------------------------------------------------------
  Main Program
 *---------------------------------------------------------------------------*/
int main (void) 
{
    VCOM_Init();                              // VCOM Initialization

    USB_Init();                               // USB Initialization
    USB_Connect(TRUE);                        // USB Connect

    while (!USB_Configuration) ;              // wait until USB is configured

    while (1) 
    {                               // Loop forever
        VCOM_Serial2Usb();                      // read serial port and initiate USB event
        VCOM_CheckSerialState();
        VCOM_Usb2Serial();
    } // end while
} // end main ()

/**
 * @}
 */
#endif /*_USB_DEV_VIRTUAL_COM*/
