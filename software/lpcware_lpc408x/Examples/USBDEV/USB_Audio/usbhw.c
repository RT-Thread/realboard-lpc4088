/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    usbhw.c
 * Purpose: USB Hardware Layer Module for NXP's LPC17xx MCU
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
 *          V1.20 Added USB_ClearEPBuf
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _USB_DEV_AUDIO
#include "LPC407x_8x_177x_8x.h"
#include "lpc_types.h"
#include "bsp.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbreg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbuser.h"



/** @addtogroup USBDEV_AudioUsbHw
 * @{
 */
 
#if defined   (  __IAR_SYSTEMS_ICC__  )
#pragma language=save
#pragma language=extended
#endif

#define EP_MSK_CTRL 0x0001      /* Control Endpoint Logical Address Mask */
#define EP_MSK_BULK 0xC924      /* Bulk Endpoint Logical Address Mask */
#define EP_MSK_INT  0x4492      /* Interrupt Endpoint Logical Address Mask */
#define EP_MSK_ISO  0x1248      /* Isochronous Endpoint Logical Address Mask */


#if USB_DMA
#if defined (__CC_ARM)
#pragma arm section zidata = "USB_RAM"
uint32_t UDCA[USB_EP_NUM];                     /* UDCA in USB RAM */

uint32_t DD_NISO_Mem[4*DD_NISO_CNT];           /* Non-Iso DMA Descriptor Memory */
uint32_t DD_ISO_Mem [5*DD_ISO_CNT];            /* Iso DMA Descriptor Memory */
#pragma arm section zidata
#elif defined ( __ICCARM__ )
#pragma location = "USB_RAM"
uint32_t UDCA[USB_EP_NUM];                     /* UDCA in USB RAM */
#pragma location = "USB_RAM"
uint32_t DD_NISO_Mem[4*DD_NISO_CNT];           /* Non-Iso DMA Descriptor Memory */
#pragma location = "USB_RAM"
uint32_t DD_ISO_Mem [5*DD_ISO_CNT];            /* Iso DMA Descriptor Memory */
#else
uint32_t UDCA[USB_EP_NUM]__attribute__((section ("USB_RAM")));                     /* UDCA in USB RAM */

uint32_t DD_NISO_Mem[4*DD_NISO_CNT]__attribute__((section ("USB_RAM")));           /* Non-Iso DMA Descriptor Memory */
uint32_t DD_ISO_Mem [5*DD_ISO_CNT]__attribute__((section ("USB_RAM")));            /* Iso DMA Descriptor Memory */
#endif /*__GNUC__*/
uint32_t udca[USB_EP_NUM];                     /* UDCA saved values */

uint32_t DDMemMap[2];                          /* DMA Descriptor Memory Usage */

#endif


/*
 *  Get Endpoint Physical Address
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    Endpoint Physical Address
 */

uint32_t EPAdr (uint32_t EPNum) {
  uint32_t val;

  val = (EPNum & 0x0F) << 1;
  if (EPNum & 0x80) {
    val += 1;
  }
  return (val);
}


/*
 *  Write Command
 *    Parameters:      cmd:   Command
 *    Return Value:    None
 */

void WrCmd (uint32_t cmd) {

  LPC_USB->DevIntClr = CCEMTY_INT;
  LPC_USB->CmdCode = cmd;
  while ((LPC_USB->DevIntSt & CCEMTY_INT) == 0);
}


/*
 *  Write Command Data
 *    Parameters:      cmd:   Command
 *                     val:   Data
 *    Return Value:    None
 */

void WrCmdDat (uint32_t cmd, uint32_t val) {

  LPC_USB->DevIntClr = CCEMTY_INT;
  LPC_USB->CmdCode = cmd;
  while ((LPC_USB->DevIntSt & CCEMTY_INT) == 0);
  LPC_USB->DevIntClr = CCEMTY_INT;
  LPC_USB->CmdCode = val;
  while ((LPC_USB->DevIntSt & CCEMTY_INT) == 0);
}


/*
 *  Write Command to Endpoint
 *    Parameters:      cmd:   Command
 *                     val:   Data
 *    Return Value:    None
 */

void WrCmdEP (uint32_t EPNum, uint32_t cmd){

  LPC_USB->DevIntClr = CCEMTY_INT;
  LPC_USB->CmdCode = CMD_SEL_EP(EPAdr(EPNum));
  while ((LPC_USB->DevIntSt & CCEMTY_INT) == 0);
  LPC_USB->DevIntClr = CCEMTY_INT;
  LPC_USB->CmdCode = cmd;
  while ((LPC_USB->DevIntSt & CCEMTY_INT) == 0);
}


/*
 *  Read Command Data
 *    Parameters:      cmd:   Command
 *    Return Value:    Data Value
 */

uint32_t RdCmdDat (uint32_t cmd) {

  LPC_USB->DevIntClr = CCEMTY_INT | CDFULL_INT;
  LPC_USB->CmdCode = cmd;
  while ((LPC_USB->DevIntSt & CDFULL_INT) == 0);
  return (LPC_USB->CmdData);
}


/*
 *  USB Initialize Function
 *   Called by the User to initialize USB
 *    Return Value:    None
 */

void USB_Init (void) {
  /* if 1, use port 1 as device, if 0, use port2 as device. */
#if  (USB_PORT == 1)
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
  LPC_IOCON->P0_27 = 0x2;
  LPC_IOCON->P0_28 = 0x2;
#endif  

  LPC_IOCON->P0_29 &= ~0x07;    /* P0.29 D1+, P0.30 D1- */
  LPC_IOCON->P0_29 |= 0x1;
  LPC_IOCON->P0_30 &= ~0x07;
  LPC_IOCON->P0_30 |= 0x1;

  LPC_IOCON->P2_9  &= ~0x07;    /* USB_SoftConnect */
  LPC_IOCON->P2_9  |= 0x1;

  LPC_IOCON->P1_30  &= ~0x07;   /* USB_VBUS */
  LPC_IOCON->P1_30  |= 0x2;

  LPC_IOCON->P1_18  &= ~0x07;   /* USB_LED */
  LPC_IOCON->P1_18  |= 0x1;

  LPC_SC->PCONP |= (1UL<<31);                /* USB PCLK -> enable USB Per.       */

  LPC_USB->USBClkCtrl =  0x12|(1<<2);                 /* Dev, AHB clock enable */
  while ((LPC_USB->USBClkSt & 0x12) != 0x12);
#else
  LPC_IOCON->P0_31 &= ~0x07;    /* P0.31 D2+, D2- is dedicated pin.  */
  LPC_IOCON->P0_31 |= 0x1;

  LPC_IOCON->P0_14  &= ~0x07;    /* USB_SoftConnect */
  LPC_IOCON->P0_14  |= 0x3;

  LPC_IOCON->P1_30  &= ~0x07;   /* USB_VBUS */
  LPC_IOCON->P1_30  |= 0x2;

  LPC_IOCON->P0_13  &= ~0x07;   /* USB_LED */
  LPC_IOCON->P0_13  |= 0x1;

  LPC_SC->PCONP |= (1UL<<31);                /* USB PCLK -> enable USB Per.       */

  LPC_USB->USBClkCtrl = 0x1A;                /* Dev, OTG, AHB clock enable */
  while ((LPC_USB->USBClkSt & 0x1A) != 0x1A);

  /* Port Select register when USB device is configured. */
  LPC_USB->StCtrl = 0x3;

  LPC_USB->USBClkCtrl = 0x12;                /* Disable OTG clock */
  while ((LPC_USB->USBClkSt & 0x12) != 0x12);
 #endif

  NVIC_EnableIRQ(USB_IRQn);               /* enable USB interrupt */

#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
  LPC_USB->I2C_CTL |= (1<<8);
  while(LPC_USB->I2C_CTL & (1<<8));
#endif

  USB_Reset();
  USB_SetAddress(0);
  return;
}


/*
 *  USB Connect Function
 *   Called by the User to Connect/Disconnect USB
 *    Parameters:      con:   Connect/Disconnect
 *    Return Value:    None
 */

void USB_Connect (uint32_t con) {
  WrCmdDat(CMD_SET_DEV_STAT, DAT_WR_BYTE(con ? DEV_CON : 0));
  #if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
  LPC_USB->I2C_STS |= (1<<0);
  LPC_USB->I2C_TX = 0x15A;             // Send ISP1301 address, R/W=0
  LPC_USB->I2C_TX = 0x006;  // Send OTG Control (Clear/Set) register address
  LPC_USB->I2C_TX = 0x201;             // Set DP_PULLUP bit, send STOP condition
  while(!LPC_USB->I2C_STS &(1<<0));
#endif
}


/*
 *  USB Reset Function
 *   Called automatically on USB Reset
 *    Return Value:    None
 */

void USB_Reset (void) {
#if USB_DMA
  uint32_t n;
#endif

  LPC_USB->EpInd = 0;
  LPC_USB->MaxPSize = USB_MAX_PACKET0;
  LPC_USB->EpInd = 1;
  LPC_USB->MaxPSize = USB_MAX_PACKET0;
  while ((LPC_USB->DevIntSt & EP_RLZED_INT) == 0);

  LPC_USB->EpIntClr  = 0xFFFFFFFF;
  LPC_USB->EpIntEn   = 0xFFFFFFFF ^ USB_DMA_EP;
  LPC_USB->DevIntClr = 0xFFFFFFFF;
  LPC_USB->DevIntEn  = DEV_STAT_INT    | EP_SLOW_INT    |
               (USB_SOF_EVENT   ? FRAME_INT : 0) |
               (USB_ERROR_EVENT ? ERR_INT   : 0);

#if USB_DMA
  LPC_USB->UDCAH   = USB_RAM_ADR;
  LPC_USB->DMARClr = 0xFFFFFFFF;
  LPC_USB->EpDMADis  = 0xFFFFFFFF;
  LPC_USB->EpDMAEn   = USB_DMA_EP;
  LPC_USB->EoTIntClr = 0xFFFFFFFF;
  LPC_USB->NDDRIntClr = 0xFFFFFFFF;
  LPC_USB->SysErrIntClr = 0xFFFFFFFF;
  LPC_USB->DMAIntEn  = 0x00000007;
  DDMemMap[0] = 0x00000000;
  DDMemMap[1] = 0x00000000;
  for (n = 0; n < USB_EP_NUM; n++) {
    udca[n] = 0;
    UDCA[n] = 0;
  }
#endif
}


/*
 *  USB Suspend Function
 *   Called automatically on USB Suspend
 *    Return Value:    None
 */

void USB_Suspend (void) {
  /* Performed by Hardware */
}


/*
 *  USB Resume Function
 *   Called automatically on USB Resume
 *    Return Value:    None
 */

void USB_Resume (void) {
  /* Performed by Hardware */
}


/*
 *  USB Remote Wakeup Function
 *   Called automatically on USB Remote Wakeup
 *    Return Value:    None
 */

void USB_WakeUp (void) {

  if (USB_DeviceStatus & USB_GETSTATUS_REMOTE_WAKEUP) {
    WrCmdDat(CMD_SET_DEV_STAT, DAT_WR_BYTE(DEV_CON));
  }
}


/*
 *  USB Remote Wakeup Configuration Function
 *    Parameters:      cfg:   Enable/Disable
 *    Return Value:    None
 */

void USB_WakeUpCfg (uint32_t cfg) {
  /* Not needed */
}


/*
 *  USB Set Address Function
 *    Parameters:      adr:   USB Address
 *    Return Value:    None
 */

void USB_SetAddress (uint32_t adr) {
  WrCmdDat(CMD_SET_ADDR, DAT_WR_BYTE(DEV_EN | adr)); /* Don't wait for next */
  WrCmdDat(CMD_SET_ADDR, DAT_WR_BYTE(DEV_EN | adr)); /*  Setup Status Phase */
}


/*
 *  USB Configure Function
 *    Parameters:      cfg:   Configure/Deconfigure
 *    Return Value:    None
 */

void USB_Configure (uint32_t cfg) {

  WrCmdDat(CMD_CFG_DEV, DAT_WR_BYTE(cfg ? CONF_DVICE : 0));

  LPC_USB->ReEp = 0x00000003;
  while ((LPC_USB->DevIntSt & EP_RLZED_INT) == 0);
  LPC_USB->DevIntClr = EP_RLZED_INT;
}


/*
 *  Configure USB Endpoint according to Descriptor
 *    Parameters:      pEPD:  Pointer to Endpoint Descriptor
 *    Return Value:    None
 */

void USB_ConfigEP (USB_ENDPOINT_DESCRIPTOR *pEPD) {
  uint32_t num;

  num = EPAdr(pEPD->bEndpointAddress);
  LPC_USB->ReEp |= (1 << num);
  LPC_USB->EpInd = num;
  LPC_USB->MaxPSize = pEPD->wMaxPacketSize;
  while ((LPC_USB->DevIntSt & EP_RLZED_INT) == 0);
  LPC_USB->DevIntClr = EP_RLZED_INT;
}


/*
 *  Set Direction for USB Control Endpoint
 *    Parameters:      dir:   Out (dir == 0), In (dir <> 0)
 *    Return Value:    None
 */

void USB_DirCtrlEP (uint32_t dir) {
  /* Not needed */
}


/*
 *  Enable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_EnableEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(0));
}


/*
 *  Disable USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DisableEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(EP_STAT_DA));
}


/*
 *  Reset USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ResetEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(0));
}


/*
 *  Set Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_SetStallEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(EP_STAT_ST));
}


/*
 *  Clear Stall for USB Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ClrStallEP (uint32_t EPNum) {
  WrCmdDat(CMD_SET_EP_STAT(EPAdr(EPNum)), DAT_WR_BYTE(0));
}


/*
 *  Clear USB Endpoint Buffer
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_ClearEPBuf (uint32_t EPNum) {
  WrCmdEP(EPNum, CMD_CLR_BUF);
}


/*
 *  Read USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *    Return Value:    Number of bytes read
 */

uint32_t USB_ReadEP (uint32_t EPNum, uint8_t *pData) {
  uint32_t cnt, n;

  LPC_USB->Ctrl = ((EPNum & 0x0F) << 2) | CTRL_RD_EN;

  do {
    cnt = LPC_USB->RxPLen;
  } while ((cnt & PKT_RDY) == 0);
  cnt &= PKT_LNGTH_MASK;

  for (n = 0; n < (cnt + 3) / 4; n++) {
    *((__packed uint32_t *)pData) = LPC_USB->RxData;
    pData += 4;
  }
  LPC_USB->Ctrl = 0;

  if (((EP_MSK_ISO >> EPNum) & 1) == 0) {   /* Non-Isochronous Endpoint */
    WrCmdEP(EPNum, CMD_CLR_BUF);
  }
  return (cnt);
}


/*
 *  Write USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *                     cnt:   Number of bytes to write
 *    Return Value:    Number of bytes written
 */

uint32_t USB_WriteEP (uint32_t EPNum, uint8_t *pData, uint32_t cnt) {
  uint32_t n;

  LPC_USB->Ctrl = ((EPNum & 0x0F) << 2) | CTRL_WR_EN;

  LPC_USB->TxPLen = cnt;

  for (n = 0; n < (cnt + 3) / 4; n++) {
    LPC_USB->TxData = *((__packed uint32_t *)pData);
    pData += 4;
  }
  LPC_USB->Ctrl = 0;
  WrCmdEP(EPNum, CMD_VALID_BUF);
  return (cnt);
}

#if USB_DMA

/* DMA Descriptor Memory Layout */
const uint32_t DDAdr[2] = { DD_NISO_ADR, DD_ISO_ADR };
const uint32_t DDSz [2] = { 16,          20         };


/*
 *  Setup USB DMA Transfer for selected Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                     pDD: Pointer to DMA Descriptor
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t USB_DMA_Setup(uint32_t EPNum, USB_DMA_DESCRIPTOR *pDD) {
  uint32_t num, ptr, nxt, iso, n;
  uint32_t *tmp;

  iso = pDD->Cfg.Type.IsoEP;                /* Iso or Non-Iso Descriptor */
  num = EPAdr(EPNum);                       /* Endpoint's Physical Address */

  ptr = 0;                                  /* Current Descriptor */
  nxt = udca[num];                          /* Initial Descriptor */
  while (nxt) {                             /* Go through Descriptor List */
    ptr = nxt;                              /* Current Descriptor */
    if (!pDD->Cfg.Type.Link) {              /* Check for Linked Descriptors */
      n = (ptr - DDAdr[iso]) / DDSz[iso];   /* Descriptor Index */
      DDMemMap[iso] &= ~(1 << n);           /* Unmark Memory Usage */
    }
    nxt = *((uint32_t *)ptr);                  /* Next Descriptor */
  }

  for (n = 0; n < 32; n++) {                /* Search for available Memory */
    if ((DDMemMap[iso] & (1 << n)) == 0) {
      break;                                /* Memory found */
    }
  }
  if (n == 32) return (FALSE);              /* Memory not available */

  DDMemMap[iso] |= 1 << n;                  /* Mark Memory Usage */
  nxt = DDAdr[iso] + n * DDSz[iso];         /* Next Descriptor */

  if (ptr && pDD->Cfg.Type.Link) {
    *((uint32_t *)(ptr + 0))  = nxt;           /* Link in new Descriptor */
    *((uint32_t *)(ptr + 4)) |= 0x00000004;    /* Next DD is Valid */
  } else {
    udca[num] = nxt;                        /* Save new Descriptor */
    UDCA[num] = nxt;                        /* Update UDCA in USB */
  }

  /* Fill in DMA Descriptor */
  tmp = ((uint32_t *)nxt);
  *tmp++ =  0;                 /* Next DD Pointer */
  *tmp++ =  pDD->Cfg.Type.ATLE |
                       (pDD->Cfg.Type.IsoEP << 4) |
                       (pDD->MaxSize <<  5) |
                       (pDD->BufLen  << 16);
  *tmp++ =  pDD->BufAdr;
  *tmp++ =  pDD->Cfg.Type.LenPos << 8;
  if (iso) {
    *tmp =  pDD->InfoAdr;
  }

  return (TRUE); /* Success */
}


/*
 *  Enable USB DMA Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DMA_Enable (uint32_t EPNum) {
  LPC_USB->EpDMAEn = 1 << EPAdr(EPNum);
}


/*
 *  Disable USB DMA Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void USB_DMA_Disable (uint32_t EPNum) {
  LPC_USB->EpDMADis = 1 << EPAdr(EPNum);
}

uint32_t USB_DMA_DD_Retired (uint32_t EPNum) {
  uint32_t ptr, val;
  ptr = UDCA[EPAdr(EPNum)];                 /* Current Descriptor */
  if (ptr == 0) 
    return (USB_DMA_INVALID);
  val = *((uint32_t *)(ptr + 3*4));         /* Status Information */
  return (val&0x01);                       /* Current Count */
}


/*
 *  Get USB DMA Endpoint Status
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    DMA Status
 */

uint32_t USB_DMA_Status (uint32_t EPNum) {
  uint32_t ptr, val;
          
  ptr = UDCA[EPAdr(EPNum)];                 /* Current Descriptor */
  if (ptr == 0) 
    return (USB_DMA_INVALID);

  val = *((uint32_t *)(ptr + 3*4));            /* Status Information */
  switch ((val >> 1) & 0x0F) {
    case 0x00:                              /* Not serviced */
      return (USB_DMA_IDLE);
    case 0x01:                              /* Being serviced */
      return (USB_DMA_BUSY);
    case 0x02:                              /* Normal Completition */
      return (USB_DMA_DONE);
    case 0x03:                              /* Data Under Run */
      return (USB_DMA_UNDER_RUN);
    case 0x08:                              /* Data Over Run */
      return (USB_DMA_OVER_RUN);
    case 0x09:                              /* System Error */
      return (USB_DMA_ERROR);
  }

  return (USB_DMA_UNKNOWN);
}


/*
 *  Get USB DMA Endpoint Current Buffer Address
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    DMA Address (or -1 when DMA is Invalid)
 */

uint32_t USB_DMA_BufAdr (uint32_t EPNum) {
  uint32_t ptr, val;

  ptr = UDCA[EPAdr(EPNum)];                 /* Current Descriptor */
  if (ptr == 0)
  {
    return ((uint32_t)(-1));                /* DMA Invalid */
  }

  val = *((uint32_t *)(ptr + 2*4));         /* Buffer Address */
  return (val);                             /* Current Address */
}


/*
 *  Get USB DMA Endpoint Current Buffer Count
 *   Number of transfered Bytes or Iso Packets
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    DMA Count (or -1 when DMA is Invalid)
 */

uint32_t USB_DMA_BufCnt (uint32_t EPNum) {
  uint32_t ptr, val;

  ptr = UDCA[EPAdr(EPNum)];                 /* Current Descriptor */
  if (ptr == 0)
  { 
    return ((uint32_t)(-1));                /* DMA Invalid */
  }
  val = *((uint32_t *)(ptr + 3*4));         /* Status Information */
  return (val >> 16);                       /* Current Count */
}


#endif /* USB_DMA */


/*
 *  Get USB Last Frame Number
 *    Parameters:      None
 *    Return Value:    Frame Number
 */

uint32_t USB_GetFrame (void) {
  uint32_t val;

  WrCmd(CMD_RD_FRAME);
  val = RdCmdDat(DAT_RD_FRAME);
  val = val | (RdCmdDat(DAT_RD_FRAME) << 8);

  return (val);
}


/*
 *  USB Interrupt Service Routine
 */

void USB_IRQHandler (void) {
  uint32_t disr, val, n, m;
  uint32_t episr, episrCur;

  disr = LPC_USB->DevIntSt;       /* Device Interrupt Status */

  /* Device Status Interrupt (Reset, Connect change, Suspend/Resume) */
  if (disr & DEV_STAT_INT) {
    LPC_USB->DevIntClr = DEV_STAT_INT;
    WrCmd(CMD_GET_DEV_STAT);
    val = RdCmdDat(DAT_GET_DEV_STAT);       /* Device Status */
    if (val & DEV_RST) {                    /* Reset */
      USB_Reset();
#if   USB_RESET_EVENT
      USB_Reset_Event();
#endif
    }
    if (val & DEV_CON_CH) {                 /* Connect change */
#if   USB_POWER_EVENT
      USB_Power_Event(val & DEV_CON);
#endif
    }
    if (val & DEV_SUS_CH) {                 /* Suspend/Resume */
      if (val & DEV_SUS) {                  /* Suspend */
        USB_Suspend();
#if     USB_SUSPEND_EVENT
        USB_Suspend_Event();
#endif
      } else {                              /* Resume */
        USB_Resume();
#if     USB_RESUME_EVENT
        USB_Resume_Event();
#endif
      }
    }
    goto isr_end;
  }

#if USB_SOF_EVENT
  /* Start of Frame Interrupt */
  if (disr & FRAME_INT) {
    USB_SOF_Event();
  }
#endif

#if USB_ERROR_EVENT
  /* Error Interrupt */
  if (disr & ERR_INT) {
    WrCmd(CMD_RD_ERR_STAT);
    val = RdCmdDat(DAT_RD_ERR_STAT);
    USB_Error_Event(val);
  }
#endif

  /* Endpoint's Slow Interrupt */
  if (disr & EP_SLOW_INT) {
    episrCur = 0;
    episr    = LPC_USB->EpIntSt;
    for (n = 0; n < USB_EP_NUM; n++) {      /* Check All Endpoints */
      if (episr == episrCur) break;         /* break if all EP interrupts handled */
      if (episr & (1 << n)) {
        episrCur |= (1 << n);
        m = n >> 1;
  
        LPC_USB->EpIntClr = (1 << n);
        while ((LPC_USB->DevIntSt & CDFULL_INT) == 0);
        val = LPC_USB->CmdData;
  
        if ((n & 1) == 0) {                 /* OUT Endpoint */
          if (n == 0) {                     /* Control OUT Endpoint */
            if (val & EP_SEL_STP) {         /* Setup Packet */
              if (USB_P_EP[0]) {
                USB_P_EP[0](USB_EVT_SETUP);
                continue;
              }
            }
          }
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_OUT);
          }
        } else {                            /* IN Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_IN);
          }
        }
      }
    }
    LPC_USB->DevIntClr = EP_SLOW_INT;
  }

#if USB_DMA

  if (LPC_USB->DMAIntSt & 0x00000001) {          /* End of Transfer Interrupt */
    val = LPC_USB->EoTIntSt;
    for (n = 2; n < USB_EP_NUM; n++) {      /* Check All Endpoints */
      if (val & (1 << n)) {
        m = n >> 1;
        if ((n & 1) == 0) {                 /* OUT Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_OUT_DMA_EOT);
          }
        } else {                            /* IN Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_IN_DMA_EOT);
          }
        }
      }
    }
    LPC_USB->EoTIntClr = val;
  }

  if (LPC_USB->DMAIntSt & 0x00000002) {          /* New DD Request Interrupt */
    val = LPC_USB->NDDRIntSt;
    for (n = 2; n < USB_EP_NUM; n++) {      /* Check All Endpoints */
      if (val & (1 << n)) {
        m = n >> 1;
        if ((n & 1) == 0) {                 /* OUT Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_OUT_DMA_NDR);
          }
        } else {                            /* IN Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_IN_DMA_NDR);
          }
        }
      }
    }
    LPC_USB->NDDRIntClr = val;
  }

  if (LPC_USB->DMAIntSt & 0x00000004) {          /* System Error Interrupt */
    val = LPC_USB->SysErrIntSt;
    for (n = 2; n < USB_EP_NUM; n++) {      /* Check All Endpoints */
      if (val & (1 << n)) {
        m = n >> 1;
        if ((n & 1) == 0) {                 /* OUT Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_OUT_DMA_ERR);
          }
        } else {                            /* IN Endpoint */
          if (USB_P_EP[m]) {
            USB_P_EP[m](USB_EVT_IN_DMA_ERR);
          }
        }
      }
    }
    LPC_USB->SysErrIntClr = val;
  }

#endif /* USB_DMA */

isr_end:
  return;
}

#if defined   (  __IAR_SYSTEMS_ICC__  )
#pragma language=restore
#endif
/**
 * @}
 */
#endif /*_USB_DEV_AUDIO*/

