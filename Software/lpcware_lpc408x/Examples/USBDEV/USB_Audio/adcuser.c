/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    ADCUSER.C
 *      Purpose: Audio Device Class Custom User Module
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
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _USB_DEV_AUDIO

#include "lpc_types.h"

#include "usb.h"
#include "audio.h"
#include "usbcfg.h"
#include "usbcore.h"
#include "adcuser.h"

#include "usbaudio.h"

/** @addtogroup USBDEV_AudioUsbUser
 * @{
 */
 
#if defined   (  __IAR_SYSTEMS_ICC__  )
#pragma language=save
#pragma language=extended
#endif

uint16_t VolCur = 0x0100;     /* Volume Current Value */
const uint16_t VolMin = 0x0000;     /* Volume Minimum Value */
const uint16_t VolMax = 0x0100;     /* Volume Maximum Value */
const uint16_t VolRes = 0x0004;     /* Volume Resolution */

/*
 *  Audio Device Class Interface Get Request Callback
 *   Called automatically on ADC Interface Get Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t ADC_IF_GetRequest (void) {

/*
  Interface = SetupPacket.wIndex.WB.L;
  EntityID  = SetupPacket.wIndex.WB.H;
  Request   = SetupPacket.bRequest;
  Value     = SetupPacket.wValue.W;
  ...
*/

  if (SetupPacket.wIndex.W == 0x0200) {
    /* Feature Unit: Interface = 0, ID = 2 */
    if (SetupPacket.wValue.WB.L == 0) {
      /* Master Channel */
      switch (SetupPacket.wValue.WB.H) {
        case AUDIO_MUTE_CONTROL:
          switch (SetupPacket.bRequest) {
            case AUDIO_REQUEST_GET_CUR:
              EP0Buf[0] = Mute;
              return (TRUE);
          }
          break;
        case AUDIO_VOLUME_CONTROL:
          switch (SetupPacket.bRequest) {
            case AUDIO_REQUEST_GET_CUR:
              *((__packed uint16_t *)EP0Buf) = VolCur;
              return (TRUE);
            case AUDIO_REQUEST_GET_MIN:
              *((__packed uint16_t *)EP0Buf) = VolMin;
              return (TRUE);
            case AUDIO_REQUEST_GET_MAX:
              *((__packed uint16_t *)EP0Buf) = VolMax;
              return (TRUE);
            case AUDIO_REQUEST_GET_RES:
              *((__packed uint16_t *)EP0Buf) = VolRes;
              return (TRUE);
          }
          break;
      }
    }
  }
  return (FALSE);  /* Not Supported */
}


/*
 *  Audio Device Class Interface Set Request Callback
 *   Called automatically on ADC Interface Set Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t ADC_IF_SetRequest (void) {

/*
  Interface = SetupPacket.wIndex.WB.L;
  EntityID  = SetupPacket.wIndex.WB.H;
  Request   = SetupPacket.bRequest;
  Value     = SetupPacket.wValue.W;
  ...
*/

  if (SetupPacket.wIndex.W == 0x0200) {
    /* Feature Unit: Interface = 0, ID = 2 */
    if (SetupPacket.wValue.WB.L == 0) {
      /* Master Channel */
      switch (SetupPacket.wValue.WB.H) {
        case AUDIO_MUTE_CONTROL:
          switch (SetupPacket.bRequest) {
            case AUDIO_REQUEST_SET_CUR:
              Mute = EP0Buf[0];
              return (TRUE);
          }
          break;
        case AUDIO_VOLUME_CONTROL:
          switch (SetupPacket.bRequest) {
            case AUDIO_REQUEST_SET_CUR:
              VolCur = *((__packed uint16_t *)EP0Buf);
              return (TRUE);
          }
          break;
      }
    }
  }
  return (FALSE);  /* Not Supported */
}


/*
 *  Audio Device Class EndPoint Get Request Callback
 *   Called automatically on ADC EndPoint Get Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t ADC_EP_GetRequest (void) {

/*
  EndPoint = SetupPacket.wIndex.WB.L;
  Request  = SetupPacket.bRequest;
  Value    = SetupPacket.wValue.W;
  ...
*/
  return (FALSE);  /* Not Supported */
}


/*
 *  Audio Device Class EndPoint Set Request Callback
 *   Called automatically on ADC EndPoint Set Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t ADC_EP_SetRequest (void) {

/*
  EndPoint = SetupPacket.wIndex.WB.L;
  Request  = SetupPacket.bRequest;
  Value    = SetupPacket.wValue.W;
  ...
*/
  return (FALSE);  /* Not Supported */
}
#if defined   (  __IAR_SYSTEMS_ICC__  )
#pragma language=restore
#endif

/**
 * @}
 */
#endif /*_USB_DEV_AUDIO*/
 
