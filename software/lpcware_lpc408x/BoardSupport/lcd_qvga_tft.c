/**********************************************************************
* $Id$      lcd.c           2012-03-13
*//**
* @file     lcd.c
* @brief    Contains all functions to control LCD controller using SPI
* @version  1.0
* @date     13. March. 2012
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
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */

#include "lpc_ssp.h"
#include "lpc_pinsel.h"
#include "lpc_timer.h"
#include "lpc_gpio.h"

#if (_CUR_USING_LCD == _RUNNING_LCD_QVGA_TFT)
#if defined(_SSP)&& defined (_GPIO) && defined(_TIM)
/** @defgroup LCD_QVGA  QVGA TFT LCD
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/
#define DC_CMD   (GPIO_ClearValue(LCD_DC_PORT_NUM, (1<<LCD_DC_PIN_NUM))) 
#define DC_DATA  (GPIO_SetValue(LCD_DC_PORT_NUM, (1<<LCD_DC_PIN_NUM))) 

#define SSP_PORT LCD_SSP_CTRL
#define SSP_CLOCK 1000000

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/*********************************************************************//**
 * @brief       Pin configuration to communicate with LCD Controller.
 * @param[in]   None
 * @return      None
 **********************************************************************/
static void pinConfig(void)
{
  /*  (CS) */
  GPIO_SetDir(LCD_CS_PORT_NUM,(1<<LCD_CS_PIN_NUM),1);

  /* (DC) */
  GPIO_SetDir(LCD_DC_PORT_NUM,(1<<LCD_DC_PIN_NUM),1);

  // PIN config SSP
#ifdef CORE_M4
  PINSEL_ConfigPin(5, 2, 2);
  PINSEL_ConfigPin(5, 3, 2);
  PINSEL_ConfigPin(5, 1, 2);
  PINSEL_ConfigPin(5, 0, 2);
#else
  PINSEL_ConfigPin(0, 15, 2);
  PINSEL_ConfigPin(0, 16, 2);
  PINSEL_ConfigPin(0, 17, 2);
  PINSEL_ConfigPin(0, 18, 2);
#endif
}

/*********************************************************************//**
 * @brief       Write to a LCD register using SPI.
 * @param[in]   address   Register address
 * @param[in]   data      Data which will be written to the given register
 * @return      None
 **********************************************************************/

static void
writeToReg(uint16_t addr, uint16_t data)
{
  uint8_t buf[2];
  SSP_DATA_SETUP_Type sspCfg;
  
  DC_CMD;

  buf[0] = 0;
  buf[1] = (addr & 0xff);

  sspCfg.tx_data = buf;
  sspCfg.rx_data = NULL;
  sspCfg.length  = 2; 

  SSP_ReadWrite (SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  DC_DATA;
  buf[0] = (data >> 8);
  buf[1] = (data & 0xff);
  SSP_ReadWrite (SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  DC_CMD;

  buf[0] = (0);
  buf[1] = (0x22);
  SSP_ReadWrite (SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);
}
/*********************************************************************//**
 * @brief       Initialize SSD1289 LCD Controller.
 * @param[in]   None
 * @return      None
 **********************************************************************/
static void ssd1289_init(void)
{
  writeToReg (0x00,0x0001);
  TIM_Waitms(15);
  writeToReg (0x03,0x6E3E); //0xAEAC
  writeToReg (0x0C,0x0007);
  writeToReg (0x0D,0x000E); //0x000F
  writeToReg (0x0E,0x2C00); //0x2900
  writeToReg (0x1E,0x00AE); //0x00B3
  TIM_Waitms(15);
  writeToReg (0x07,0x0021);
  TIM_Waitms(50);
  writeToReg (0x07,0x0023);
  TIM_Waitms(50);
  writeToReg (0x07,0x0033);
  TIM_Waitms(50);
  
  writeToReg (0x01,0x2B3F);
  writeToReg (0x02,0x0600);
  writeToReg (0x10,0x0000);
  TIM_Waitms(15);
  writeToReg (0x11,0xC5B0); //0x65b0
  TIM_Waitms(20);
  writeToReg (0x05,0x0000);
  writeToReg (0x06,0x0000);
  writeToReg (0x16,0xEF1C);
  writeToReg (0x17,0x0003);
  writeToReg (0x07,0x0233);
  writeToReg (0x0B,0x5312);
  writeToReg (0x0F,0x0000);
  writeToReg (0x25,0xE000); 
  TIM_Waitms(20);
  writeToReg (0x41,0x0000);
  writeToReg (0x42,0x0000);
  writeToReg (0x48,0x0000);
  writeToReg (0x49,0x013F);
  writeToReg (0x44,0xEF00);
  writeToReg (0x45,0x0000);
  writeToReg (0x46,0x013F);
  writeToReg (0x4A,0x0000);
  writeToReg (0x4B,0x0000);
  TIM_Waitms(20);
  writeToReg (0x30,0x0707);
  writeToReg (0x31,0x0704);
  writeToReg (0x32,0x0005); //0x0204
  writeToReg (0x33,0x0402); //0x0201
  writeToReg (0x34,0x0203);
  writeToReg (0x35,0x0204);
  writeToReg (0x36,0x0204);
  writeToReg (0x37,0x0401); //0x0502
  writeToReg (0x3A,0x0302);
  writeToReg (0x3B,0x0500);
  TIM_Waitms(20);
  writeToReg (0x22,0x0000);
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/*********************************************************************//**
 * @brief       Initialize LCD Controller.
 * @param[in]   None
 * @return      None
 **********************************************************************/

void InitLcdController (void)
{
  SSP_CFG_Type SSP_ConfigStruct;
  TIM_TIMERCFG_Type TIM_ConfigStruct;
  
  TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
  TIM_ConfigStruct.PrescaleValue  = 1;
      
  // Set configuration for Tim_config and Tim_MatchConfig
  TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
    
  pinConfig();  
  
  // initialize SSP configuration structure to default
  SSP_ConfigStructInit(&SSP_ConfigStruct);

  // set clock rate
  SSP_ConfigStruct.ClockRate = SSP_CLOCK;
  
  // Initialize SSP peripheral with parameter given in structure above
  SSP_Init(SSP_PORT, &SSP_ConfigStruct);

  // Enable SSP peripheral
  SSP_Cmd(SSP_PORT, ENABLE);

  TIM_Waitms(200);
  
  /* initialize LCD controller */
  ssd1289_init();

  SSP_Cmd(SSP_PORT, DISABLE);
  SSP_DeInit(SSP_PORT);
  TIM_DeInit(LPC_TIM0);
}

/**
 * @}
 */
#endif
#endif

