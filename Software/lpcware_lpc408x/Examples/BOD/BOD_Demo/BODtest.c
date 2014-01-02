/**********************************************************************
* $Id$      lpc_bod.c           2011-12-09
*//**
* @file     lpc_bod.c
* @brief    This is an example for how to use BOD.
* @version  1.0
* @date     09 December. 2011
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
#include "LPC407x_8x_177x_8x.h"
#include "lpc_types.h"
#include "lpc_bod.h"
#include "lpc_gpio.h"
#include "lpc_clkpwr.h"
#include "bsp.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup BOD_Demo      BOD Demotrastion
 * @ingroup BOD_Examples
 * @{
 */
#define INT_LED_PORT    (BRD_LED_1_CONNECTED_PORT)
#define INT_LED_BYTE    ((uint32_t)BRD_LED_1_CONNECTED_PIN / 8)
#define INT_LED_BIT (1 << ((uint32_t)BRD_LED_1_CONNECTED_PIN % 8))

#define RESET_LED_PORT  (BRD_LED_2_CONNECTED_PORT)
#define RESET_LED_BYTE  ((uint32_t)BRD_LED_2_CONNECTED_PIN / 8)
#define RESET_LED_BIT   (1 << ((uint32_t)BRD_LED_2_CONNECTED_PIN % 8))

/*********************************************************************//**
 * @brief       Delay function
 * @param[in]   None
 * @return      None
 **********************************************************************/
void delay (uint32_t delayCnt)
{
  volatile unsigned int i;

  for (i = 0; i < delayCnt; i++)
  {

  }
}
/*********************************************************************//**
 * @brief       BOD interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void BOD_IRQHandler (void) 
{  
    uint8_t j;
    for(j=0; j<5;j++)
    {
    FIO_ByteSetValue(INT_LED_PORT, INT_LED_BYTE, INT_LED_BIT);
    delay(10000000);

    FIO_ByteClearValue(INT_LED_PORT, INT_LED_BYTE, INT_LED_BIT);
    delay(10000000);
    }
}
/*********************************************************************//**
 * @brief       Main entry
 * @param[in]   None
 * @return      None
 **********************************************************************/
int main (void)
{               
  BOD_Config_Type Config;
      
  GPIO_Init();
  FIO_ByteSetDir(INT_LED_PORT, INT_LED_BYTE, INT_LED_BIT, GPIO_DIRECTION_OUTPUT);
  FIO_ByteSetDir(RESET_LED_PORT, RESET_LED_BYTE, RESET_LED_BIT, GPIO_DIRECTION_OUTPUT);

  // LEDs ON
  FIO_ByteClearValue(INT_LED_PORT, INT_LED_BYTE, INT_LED_BIT);
  FIO_ByteClearValue(RESET_LED_PORT, RESET_LED_BYTE, RESET_LED_BIT);

  Config.Enabled = ENABLE;
  Config.PowerReduced = DISABLE;
  Config.ResetOnVoltageDown = ENABLE;
  BOD_Init(&Config);
  
  if(BOD_ResetSourceStatus())
  {
    BOD_ResetSourceClr();
        
    while(1)
    {
        FIO_ByteSetValue(RESET_LED_PORT, RESET_LED_BYTE, RESET_LED_BIT);
        delay(10000000);

        FIO_ByteClearValue(RESET_LED_PORT, RESET_LED_BYTE, RESET_LED_BIT);
        delay(10000000);
    }
     }
     else
     {
         while(1);
     }
}
/**
 * @}
 */





