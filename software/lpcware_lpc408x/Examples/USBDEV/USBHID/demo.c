/*----------------------------------------------------------------------------
 *      Name:    DEMO.C
 *      Purpose: USB HID Demo
 *      Version: V1.20
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC family microcontroller devices only. Nothing 
 *      else gives you the right to use this software.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */

#ifdef _USB_DEV_HID
#include "LPC407x_8x_177x_8x.h"                        /* LPC17xx definitions */

#include "lpc_types.h"
#include "lpc_gpio.h"
#include "lpc_timer.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "demo.h"
#include "pca9532.h"

uint8_t InReport;                              /* HID Input Report    */

uint8_t OutReport;                             /* HID Out Report      */

/* Example group ----------------------------------------------------------- */
/** @defgroup USBDEV_USBHID USB HID Device
 * @ingroup USBDEV_Examples
 * @{
 */

/** @defgroup USBDEV_HIDUsbHw   USB-HID USB Hardware
 * @ingroup USBDEV_USBHID
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_HIDUsbCore USB-HID USB Core
 * @ingroup USBDEV_USBHID
 * @{
 */

/**
 * @}
 */

/** @defgroup USBDEV_HIDUsbDesc USB-HID USB Descriptors
 * @ingroup USBDEV_USBHID
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_HIDUsbUser USB-HID USB User
 * @ingroup USBDEV_USBHID
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_HIDUsbCfg  USB-HID USB Configuration
 * @ingroup USBDEV_USBHID
 * @{
 */

/**
 * @}
 */


/*
 *  Get HID Input Report -> InReport
 */

void GetInReport (void) {
  uint32_t kbd_val;

  kbd_val = GPIO_ReadValue(KBD_PORT_NUM) & KBD_MASK;

  InReport = 0x00;
  if ((kbd_val & KBD_UP)     == 0) InReport |= 0x01;  /* up     pressed means 0 */
  if ((kbd_val & KBD_LEFT)   == 0) InReport |= 0x02;  /* left   pressed means 0 */
  if ((kbd_val & KBD_RIGHT)  == 0) InReport |= 0x04;  /* right  pressed means 0 */
  if ((kbd_val & KBD_SELECT) == 0) InReport |= 0x08;  /* select pressed means 0 */
  if ((kbd_val & KBD_DOWN)   == 0) InReport |= 0x10;  /* down   pressed means 0 */
}


/*
 *  Set HID Output Report <- OutReport
 */

void SetOutReport (void) {

    uint32_t cnt = 0;
    
    pca9532_Configure_st_t pca9532Cfg;
    pca9532Cfg.led_blinking_freq_0 = 30;
    pca9532Cfg.led_freq0_unit = PCA9532_CALCULATING_TIME_IN_SECOND;
    pca9532Cfg.duty_cycle_0 = 0;//percent//not used

    pca9532Cfg.led_blinking_freq_1 = 0;
    pca9532Cfg.led_freq1_unit = PCA9532_CALCULATING_TIME_IN_SECOND;
    pca9532Cfg.duty_cycle_1 = 0; //not used

    for (cnt = 0; cnt < 8; cnt++)
    {
        pca9532Cfg.led_settings[cnt] = PCA9532_LED_LEVEL_DEFAULT;

        if(OutReport & (1<<cnt))
        {
            pca9532Cfg.led_settings[8 + cnt] = PCA9532_LED_LEVEL_ON;
        }
        else
        {
            pca9532Cfg.led_settings[8 + cnt] = PCA9532_LED_LEVEL_OFF;
        }
    }

    Pca9532_LedOutputControl(&pca9532Cfg);  
}

/* Main Program */

int main (void) {
  Pca9532_Init(200000);

  USB_Init();                           /* USB Initialization */
  USB_Connect(TRUE);                    /* USB Connect */
  while (1)                            /* Loop forever */
  {
  }
}
#endif /*_USB_DEV_HID*/
