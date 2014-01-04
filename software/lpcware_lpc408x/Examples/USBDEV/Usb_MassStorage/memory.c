/*----------------------------------------------------------------------------
 *      Name:    MEMORY.C
 *      Purpose: USB Mass Storage Demo
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
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */

#ifdef _USB_DEV_MASS_STORAGE
#include "LPC407x_8x_177x_8x.h"

#include "lpc_types.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "mscuser.h"

#include "memory.h"
//#include "lpc_nvic.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup USBDEV_USBMassStorage USB Mass Storage Device
 * @ingroup USBDEV_Examples
 * @{
 */

/** @defgroup USBDEV_MscUsbHw   USB-MSD USB Hardware
 * @ingroup USBDEV_USBMassStorage
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_MscUsbCore USB-MSD USB Core
 * @ingroup USBDEV_USBMassStorage
 * @{
 */

/**
 * @}
 */

/** @defgroup USBDEV_MscUsbDesc USB-MSD USB Descriptors
 * @ingroup USBDEV_USBMassStorage
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_MscUsbReg  USB-MSD USB Register
 * @ingroup USBDEV_USBMassStorage
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_MscUsbUser USB-MSD USB User
 * @ingroup USBDEV_USBMassStorage
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_MscUsbCfg  USB-MSD USB Configuration
 * @ingroup USBDEV_USBMassStorage
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_MscConf    USB-MSD MSC Configuration
 * @ingroup USBDEV_USBMassStorage
 * @{
 */

/**
 * @}
 */



extern uint8_t Memory[MSC_MemorySize];         /* MSC Memory in RAM */


/* Main Program */

int main (void) {
    uint32_t n;

    for (n = 0; n < MSC_ImageSize; n++) {     /* Copy Initial Disk Image */
        Memory[n] = DiskImage[n];               /*   from Flash to RAM     */
    }

    USB_Init();                               /* USB Initialization */
    USB_Connect(TRUE);                        /* USB Connect */

    while (1);                                /* Loop forever */
}


/**
 * @}
 */
#endif /*_USB_DEV_MASS_STORAGE*/
