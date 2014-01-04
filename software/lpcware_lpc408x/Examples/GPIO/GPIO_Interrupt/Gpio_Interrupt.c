/**********************************************************************
* $Id$      Gpio_Interrupt.c        2011-06-02
*//**
* @file     Gpio_Interrupt.c
* @brief    This example used to test GPIO interrupt function
* @version  1.0
* @date     02. June. 2011
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
#include "lpc_gpio.h"
#include "lpc_pinsel.h"
#include "bsp.h"


/** @defgroup GPIO_Interrupt    GPIO Interrupt
 * @ingroup GPIO_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/

#define LED1_PORT   (BRD_LED_1_CONNECTED_PORT)
#define LED1_BYTE   ((uint32_t)BRD_LED_1_CONNECTED_PIN / 8)
#define LED1_BIT    (1 << ((uint32_t)BRD_LED_1_CONNECTED_PIN % 8))

#define LED2_PORT   (BRD_LED_2_CONNECTED_PORT)
#define LED2_BYTE   ((uint32_t)BRD_LED_2_CONNECTED_PIN / 8)
#define LED2_BIT    (1 << ((uint32_t)BRD_LED_2_CONNECTED_PIN % 8))


/************************** PRIVATE FUNCTIONS *************************/
void GPIO_IRQHandler(void);

void delay (void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       External interrupt 3 handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void GPIO_IRQHandler(void)
{
    int j;

    if(GPIO_GetIntStatus(BRD_PIO_USED_INTR_PORT, BRD_PIO_USED_INTR_PIN, 1))
    {
        GPIO_ClearInt(BRD_PIO_USED_INTR_PORT, BRD_PIO_USED_INTR_MASK);

        for (j = 0; j < 10; j++)
        {
            FIO_ByteSetValue(LED2_PORT, LED2_BYTE, LED2_BIT);
            delay();

            FIO_ByteClearValue(LED2_PORT, LED2_BYTE, LED2_BIT);
            delay();
        }
    }
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Delay function
 * @param[in]   None
 * @return      None
 **********************************************************************/
void delay (void)
{
  volatile unsigned int i;

  for (i = 0; i < 0x1000000; i++)
  {

  }
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry (void)
{
    GPIO_Init();
    
    FIO_ByteSetDir(LED1_PORT, LED1_BYTE, LED1_BIT, GPIO_DIRECTION_OUTPUT);

    FIO_ByteSetDir(LED2_PORT, LED2_BYTE, LED2_BIT, GPIO_DIRECTION_OUTPUT);

    // Turn off all LEDs
    FIO_ByteClearValue(LED1_PORT, LED1_BYTE, LED1_BIT);
    FIO_ByteClearValue(LED2_PORT, LED2_BYTE, LED2_BIT);

    // Enable GPIO interrupt that connects with ADC potentiometer
    GPIO_IntCmd(BRD_PIO_USED_INTR_PORT, BRD_PIO_USED_INTR_MASK, 1);

    NVIC_SetPriority(GPIO_IRQn, 1);
    NVIC_EnableIRQ(GPIO_IRQn);

    while (1)
    {
        FIO_ByteSetValue(LED1_PORT, LED1_BYTE, LED1_BIT);
        delay();

        FIO_ByteClearValue(LED1_PORT, LED1_BYTE, LED1_BIT);
        delay();
    }
    
}


/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
int main(void)
{
    c_entry();
    return 0;
}

/*
 * @}
*/
