/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    HIDUSER.C
 *      Purpose: HID Custom User Module
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
#include "lpc_types.h"

#include "usb.h"
#include "hid.h"
#include "usbcfg.h"
#include "usbcore.h"
#include "hiduser.h"

#include "demo.h"


/** @addtogroup USBDEV_AudioUsbUser
 * @{
 */
uint8_t HID_Protocol;
uint8_t HID_IdleTime[HID_REPORT_NUM];


/*
 *  HID Get Report Request Callback
 *   Called automatically on HID Get Report Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_GetReport (void) {

  /* ReportID = SetupPacket.wValue.WB.L; */
  switch (SetupPacket.wValue.WB.H) {
    case HID_REPORT_INPUT:
      GetInReport();
      EP0Buf[0] = InReport;
      break;
    case HID_REPORT_OUTPUT:
      return (FALSE);          /* Not Supported */
    case HID_REPORT_FEATURE:
      /* EP0Buf[] = ...; */
      /* break; */
      return (FALSE);          /* Not Supported */
  }
  return (TRUE);
}


/*
 *  HID Set Report Request Callback
 *   Called automatically on HID Set Report Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_SetReport (void) {

  /* ReportID = SetupPacket.wValue.WB.L; */
  switch (SetupPacket.wValue.WB.H) {
    case HID_REPORT_INPUT:
      return (FALSE);          /* Not Supported */
    case HID_REPORT_OUTPUT:
      OutReport = EP0Buf[0];
      SetOutReport();
      break;
    case HID_REPORT_FEATURE:
      return (FALSE);          /* Not Supported */
  }
  return (TRUE);
}


/*
 *  HID Get Idle Request Callback
 *   Called automatically on HID Get Idle Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_GetIdle (void) {

  EP0Buf[0] = HID_IdleTime[SetupPacket.wValue.WB.L];
  return (TRUE);
}


/*
 *  HID Set Idle Request Callback
 *   Called automatically on HID Set Idle Request
 *    Parameters:      None (global SetupPacket)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_SetIdle (void) {

  HID_IdleTime[SetupPacket.wValue.WB.L] = SetupPacket.wValue.WB.H;

  /* Idle Handling if needed */
  /* ... */

  return (TRUE);
}


/*
 *  HID Get Protocol Request Callback
 *   Called automatically on HID Get Protocol Request
 *    Parameters:      None (global SetupPacket)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_GetProtocol (void) {

  EP0Buf[0] = HID_Protocol;
  return (TRUE);
}


/*
 *  HID Set Protocol Request Callback
 *   Called automatically on HID Set Protocol Request
 *    Parameters:      None (global SetupPacket)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t HID_SetProtocol (void) {

  HID_Protocol = SetupPacket.wValue.WB.L;

  /* Protocol Handling if needed */
  /* ... */

  return (TRUE);
}
/**
 * @}
 */
#endif /*_USB_DEV_HID*/
