/*----------------------------------------------------------------------------
 * Name:    usbmain.c
 * Purpose: USB Audio Class Demo
 * Version: V1.20
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
#ifdef _USB_DEV_AUDIO

#include "LPC407x_8x_177x_8x.h"
#include "lpc_types.h"
#include "bsp.h"
#include "lpc_clkpwr.h"
#include "lpc_adc.h"
#include "lpc_dac.h"
#include "lpc_timer.h"
#include "lpc_pinsel.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbaudio.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup USBDEV_USBAudio   USB Audio Device
 * @ingroup USBDEV_Examples
 * @{
 */

/** @defgroup USBDEV_AudioUsbHw USB-Audio Hardware
 * @ingroup USBDEV_USBAudio
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_AudioUsbCore  USB-Audio Core
 * @ingroup USBDEV_USBAudio
 * @{
 */

/**
 * @}
 */

/** @defgroup USBDEV_AudioUsbDesc USB-Audio Descriptors
 * @ingroup USBDEV_USBAudio
 * @{
 */

/**
 * @}
 */


/** @defgroup USBDEV_AudioUsbUser USB-Audio User
 * @ingroup USBDEV_USBAudio
 * @{
 */

/**
 * @}
 */

/** @defgroup USBDEV_AudioUsbReg USB-Audio Register
 * @ingroup USBDEV_USBAudio
 * @{
 */

/**
 * @}
 */



/** @defgroup USBDEV_AudioUsbCfg    USB-Audio Configuration
 * @ingroup USBDEV_USBAudio
 * @{
 */

/**
 * @}
 */

//Thi minimum value is as amplitude to make all the signed not to be less than 0 (zero)
#define AMPLI_MIDDLE_VALUE  (0x01 << 9)
#define AMPLI_MIN_VALUE      0x256

uint8_t  Mute;                                 /* Mute State */
uint32_t Volume;                               /* Volume Level */

#if USB_DMA
uint32_t *InfoBuf = (uint32_t *)(DMA_BUF_ADR);
int16_t *DataBuf = (int16_t *)(DMA_BUF_ADR + 4*P_C);
#else
uint32_t InfoBuf[P_C];
int16_t DataBuf[B_S];                         /* Data Buffer */
#endif

uint16_t  DataOut = 0;                              /* Data Out Index */
uint16_t  DataIn = 0;                               /* Data In Index */

uint8_t   DataRun = 0;                              /* Data Stream Run State */
uint16_t  PotVal = 0;                               /* Potenciometer Value */
uint32_t  Tick = 0;                                 /* Time Tick */

/*
 * Get potentiometer Value
 */

void get_potval (void) {
  uint16_t val;

  ADC_StartCmd(LPC_ADC, ADC_START_NOW);

  while (!(ADC_ChannelGetStatus(LPC_ADC, BRD_ADC_PREPARED_CHANNEL, ADC_DATA_DONE)));

  val = ADC_ChannelGetData(LPC_ADC, BRD_ADC_PREPARED_CHANNEL);
          
  PotVal = ((val >> 4) & 0xF8)  +            /* Extract potentiometer Value */
           ((val >> 3) & 0x08);
}


/*
 * Timer Counter Interrupt Service Routine
 *   executed each 31.25us (32kHz frequency)
 */
void TIMER_IRQHandler(void) 
{
  int32_t  val = 0;
  int16_t  in_val;
  uint32_t cnt = 0;

   if (DataRun) {                            /* Data Stream is running */
    in_val = DataBuf[DataOut];               /* Get Audio Sample */
    cnt = (DataIn - DataOut) & (B_S -1);     /* Buffer Data Count */
    if (cnt == (B_S - P_C*P_S)) {           /* Too much Data in Buffer */
      DataOut++;                            /* Skip one Sample */
    }
    if (cnt > (P_C*P_S)) {                  /* Still enough Data in Buffer */
      DataOut++;                            /* Update Data Out Index */
    }

    DataOut &= B_S - 1;                     /* Adjust Buffer Out Index */
    
    val  = in_val* Volume;                  /* Apply Volume Level */
    val >>= 16;

    val >>= 6;                                /* 16 bit --> 10 bit */
    val &= 0x3FF;
    val += AMPLI_MIDDLE_VALUE;
    
  } else {
    val = AMPLI_MIDDLE_VALUE;                   /* DAC Middle Point */
  }
            
   if (Mute) {
    val = AMPLI_MIDDLE_VALUE;                   /* DAC Middle Point */
  }

  DAC_UpdateValue(0, val);
  
  if ((Tick++ & 0x03FF) == 0) {             /* On every 1024th Tick */
    get_potval();                           /* Get Potenciometer Value */
    if (VolCur == 0x8000) {                 /* Check for Minimum Level */
      Volume = 0;                           /* No Sound */
    } else {
      Volume = VolCur * PotVal;             /* Chained Volume Level */
    }
  }
  TIM_ClearIntPending(BRD_TIMER_USED, TIM_MR0_INT);
}

 void TIMER0_IRQHandler(void) 
 {
    TIMER_IRQHandler();
 }
 void TIMER2_IRQHandler(void) 
 {
    TIMER_IRQHandler();
 }

/*****************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{
  uint32_t pclk;
  TIM_TIMERCFG_Type TIM_ConfigStruct;
  TIM_MATCHCFG_Type TIM_MatchConfigStruct ;

  PINSEL_ConfigPin (BRD_ADC_PREPARED_CH_PORT, 
                    BRD_ADC_PREPARED_CH_PIN, 
                    BRD_ADC_PREPARED_CH_FUNC_NO);
  PINSEL_SetAnalogPinMode(BRD_ADC_PREPARED_CH_PORT,BRD_ADC_PREPARED_CH_PIN,ENABLE);

  ADC_Init(LPC_ADC, DATA_FREQ);
  ADC_IntConfig(LPC_ADC, BRD_ADC_PREPARED_INTR, DISABLE);
  ADC_ChannelCmd(LPC_ADC, BRD_ADC_PREPARED_CHANNEL, ENABLE);

  DAC_Init(0);
  DAC_UpdateValue(0, AMPLI_MIDDLE_VALUE);

  TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_TICKVAL;
  TIM_ConfigStruct.PrescaleValue    = 1;
  TIM_Init(BRD_TIMER_USED, TIM_TIMER_MODE, &TIM_ConfigStruct);
  
  // use channel 0, MR0
  TIM_MatchConfigStruct.MatchChannel = 0;
  // Enable interrupt when MR0 matches the value in TC register
  TIM_MatchConfigStruct.IntOnMatch   = TRUE;
  //Enable reset on MR0: TIMER will reset if MR0 matches it
  TIM_MatchConfigStruct.ResetOnMatch = TRUE;
  //Stop on MR0 if MR0 matches it
  TIM_MatchConfigStruct.StopOnMatch  = FALSE; 
  //Toggle MR0.0 pin if MR0 matches it
  TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_TOGGLE;
  // Set Match value
  pclk =  CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
  TIM_MatchConfigStruct.MatchValue   = pclk/ DATA_FREQ - 1;

  // Set configuration
  TIM_ConfigMatch(BRD_TIMER_USED, &TIM_MatchConfigStruct);

  NVIC_EnableIRQ(BRD_TIM_INTR_USED);

  // To start timer
  TIM_Cmd(BRD_TIMER_USED, ENABLE);

  USB_Init();               /* USB Initialization */
  USB_Connect(TRUE);        /* USB Connect */


  /********* The main Function is an endless loop ***********/ 
  while( 1 );
}
#endif /*_USB_DEV_AUDIO*/
/******************************************************************************
**                            End Of File
******************************************************************************/
