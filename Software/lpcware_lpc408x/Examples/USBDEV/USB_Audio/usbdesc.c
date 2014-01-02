/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    usbdesc.c
 * Purpose: USB Descriptors
 * Version: V1.20
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
 *----------------------------------------------------------------------------
 * History:
 *          V1.20 Changed string descriptor handling
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/
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
#include "usbdesc.h"


/** @addtogroup USBDEV_AudioUsbDesc
 * @{
 */


 
/* USB Standard Device Descriptor */
const uint8_t USB_DeviceDescriptor[] = {
  USB_DEVICE_DESC_SIZE,              /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0200), /* 2.00 */          /* bcdUSB */
  0x00,                              /* bDeviceClass */
  0x00,                              /* bDeviceSubClass */
  0x00,                              /* bDeviceProtocol */
  USB_MAX_PACKET0,                   /* bMaxPacketSize0 */
  WBVAL(0x1FC9),                     /* idVendor */
  WBVAL(0x4002),                     /* idProduct */
  WBVAL(0x0100), /* 1.00 */          /* bcdDevice */
  0x01,                              /* iManufacturer */
  0x02,                              /* iProduct */
  0x03,                              /* iSerialNumber */
  0x01                               /* bNumConfigurations: one possible configuration*/
};

/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
const uint8_t USB_ConfigDescriptor[] = {
/* Configuration 1 */
  USB_CONFIGUARTION_DESC_SIZE,          /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE,    /* bDescriptorType */
  WBVAL(                                /* wTotalLength */
    USB_CONFIGUARTION_DESC_SIZE         +
    USB_INTERFACE_DESC_SIZE             +
    AUDIO_CONTROL_INTERFACE_DESC_SZ(1)  +
    AUDIO_INPUT_TERMINAL_DESC_SIZE      +
    AUDIO_FEATURE_UNIT_DESC_SZ(1,1)     +
    AUDIO_OUTPUT_TERMINAL_DESC_SIZE     +
    USB_INTERFACE_DESC_SIZE             +
    USB_INTERFACE_DESC_SIZE             +
    AUDIO_STREAMING_INTERFACE_DESC_SIZE +
    AUDIO_FORMAT_TYPE_I_DESC_SZ(1)      +
    AUDIO_STANDARD_ENDPOINT_DESC_SIZE   +
    AUDIO_STREAMING_ENDPOINT_DESC_SIZE
  ),
  0x02,                                 /* bNumInterfaces */
  0x01,                                 /* bConfigurationValue */
  0x00,                                 /* iConfiguration */
  USB_CONFIG_BUS_POWERED,               /* bmAttributes */
  USB_CONFIG_POWER_MA(100),             /* bMaxPower */
/* Interface 0, Alternate Setting 0, Audio Control */
  USB_INTERFACE_DESC_SIZE,              /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  0x00,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOCONTROL,          /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
/* Audio Control Interface */
  AUDIO_CONTROL_INTERFACE_DESC_SZ(1),   /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_HEADER,                 /* bDescriptorSubtype */
  WBVAL(0x0100), /* 1.00 */             /* bcdADC */
  WBVAL(                                /* wTotalLength */
    AUDIO_CONTROL_INTERFACE_DESC_SZ(1) +
    AUDIO_INPUT_TERMINAL_DESC_SIZE     +
    AUDIO_FEATURE_UNIT_DESC_SZ(1,1)    +
    AUDIO_OUTPUT_TERMINAL_DESC_SIZE
  ),
  0x01,                                 /* bInCollection */
  0x01,                                 /* baInterfaceNr */
/* Audio Input Terminal */
  AUDIO_INPUT_TERMINAL_DESC_SIZE,       /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_INPUT_TERMINAL,         /* bDescriptorSubtype */
  0x01,                                 /* bTerminalID */
  WBVAL(AUDIO_TERMINAL_USB_STREAMING),  /* wTerminalType */
  0x00,                                 /* bAssocTerminal */
  0x01,                                 /* bNrChannels */
  WBVAL(AUDIO_CHANNEL_M),               /* wChannelConfig */
  0x00,                                 /* iChannelNames */
  0x00,                                 /* iTerminal */
/* Audio Feature Unit */
  AUDIO_FEATURE_UNIT_DESC_SZ(1,1),      /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_FEATURE_UNIT,           /* bDescriptorSubtype */
  0x02,                                 /* bUnitID */
  0x01,                                 /* bSourceID */
  0x01,                                 /* bControlSize */
  AUDIO_CONTROL_MUTE |
  AUDIO_CONTROL_VOLUME,                 /* bmaControls(0) */
  0x00,                                 /* bmaControls(1) */
  0x00,                                 /* iTerminal */
/* Audio Output Terminal */
  AUDIO_OUTPUT_TERMINAL_DESC_SIZE,      /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_CONTROL_OUTPUT_TERMINAL,        /* bDescriptorSubtype */
  0x03,                                 /* bTerminalID */
  WBVAL(AUDIO_TERMINAL_SPEAKER),        /* wTerminalType */
  0x00,                                 /* bAssocTerminal */
  0x02,                                 /* bSourceID */
  0x00,                                 /* iTerminal */
/* Interface 1, Alternate Setting 0, Audio Streaming - Zero Bandwith */
  USB_INTERFACE_DESC_SIZE,              /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x00,                                 /* bAlternateSetting */
  0x00,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
/* Interface 1, Alternate Setting 1, Audio Streaming - Operational */
  USB_INTERFACE_DESC_SIZE,              /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  0x01,                                 /* bInterfaceNumber */
  0x01,                                 /* bAlternateSetting */
  0x01,                                 /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,               /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,        /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,             /* bInterfaceProtocol */
  0x00,                                 /* iInterface */
/* Audio Streaming Interface */
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,  /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_STREAMING_GENERAL,              /* bDescriptorSubtype */
  0x01,                                 /* bTerminalLink */
  0x01,                                 /* bDelay */
  WBVAL(AUDIO_FORMAT_PCM),              /* wFormatTag */
/* Audio Type I Format */
  AUDIO_FORMAT_TYPE_I_DESC_SZ(1),       /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,      /* bDescriptorType */
  AUDIO_STREAMING_FORMAT_TYPE,          /* bDescriptorSubtype */
  AUDIO_FORMAT_TYPE_I,                  /* bFormatType */
  0x01,                                 /* bNrChannels */
  0x02,                                 /* bSubFrameSize */
  16,                                   /* bBitResolution */
  0x01,                                 /* bSamFreqType */
  B3VAL(32000),                         /* tSamFreq */
/* Endpoint - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE,    /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,         /* bDescriptorType */
  USB_ENDPOINT_OUT(3),                  /* bEndpointAddress */
  USB_ENDPOINT_TYPE_ISOCHRONOUS,        /* bmAttributes */
  WBVAL(64),                            /* wMaxPacketSize */
  0x01,                                 /* bInterval */
  0x00,                                 /* bRefresh */
  0x00,                                 /* bSynchAddress */
/* Endpoint - Audio Streaming */
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,   /* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,       /* bDescriptorType */
  AUDIO_ENDPOINT_GENERAL,               /* bDescriptor */
  0x00,                                 /* bmAttributes */
  0x00,                                 /* bLockDelayUnits */
  WBVAL(0x0000),                        /* wLockDelay */
/* Terminator */
  0                                     /* bLength */
};

/* USB String Descriptor (optional) */
const uint8_t USB_StringDescriptor[] = {
/* Index 0x00: LANGID Codes */
  0x04,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0409), /* US English */    /* wLANGID */
/* Index 0x01: Manufacturer */
  (13*2 + 2),                        /* bLength (13 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'N',0,
  'X',0,
  'P',0,
  ' ',0,
  'S',0,
  'e',0,
  'm',0,
  'i',0,
  'c',0,
  'o',0,
  'n',0,
  'd',0,
  ' ',0,
/* Index 0x02: Product */
  (20*2 + 2),                        /* bLength ( 20 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'N',0,
  'X',0,
  'P',0,
  ' ',0,
  'L',0,
  'P',0,
  'C',0,
  '1',0,
  '7',0,
  'x',0,
  'x',0,
  ' ',0,
  'S',0,
  'p',0,
  'e',0,
  'a',0,
  'k',0,
  'e',0,
  'r',0,
  ' ',0,
/* Index 0x03: Serial Number */
  (12*2 + 2),                        /* bLength (12 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'D',0,
  'E',0,
  'M',0,
  'O',0,
  '0',0,
  '0',0,
  '0',0,
  '0',0,
  '0',0,
  '0',0,
  '0',0,
  '0',0,
};

/**
 * @}
 */
#endif /*_USB_DEV_AUDIO*/

