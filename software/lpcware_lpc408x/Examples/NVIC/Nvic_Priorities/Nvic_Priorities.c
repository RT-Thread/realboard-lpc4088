/**********************************************************************
* $Id$      Nvic_Priorities.c   2011-06-02
*//**
* @file     Nvic_Priorities.c
* @brief    This example used to test NVIC Grouping Priority function
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
#include "lpc_gpio.h"
#include "lpc_nvic.h"
#include "lpc_systick.h"
#include "lpc_exti.h"
#include "lpc_pinsel.h"
#include "lpc_adc.h"
#include "bsp.h"
#include "debug_frmwrk.h"


/** @defgroup NVIC_Priorities   NVIC Priorities
 * @ingroup NVIC_Examples
 * @{
 */

/************************** PRIVATE DEFINTIONS*************************/
/* Interrupt mode
 * - 0: Tail-chaining interrupt
 * - 1: Late-arriving interrupt
 */
#define INT_MODE    0//0

/************************** PRIVATE FUNCTIONS *************************/
void EINT0_IRQHandler(void);

void delay (void);


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       External interrupt 0 handler
 *              This interrupt occurs when pressing button INT0
 * @param       None
 * @return      None
 ***********************************************************************/
void EINT0_IRQHandler(void)
{
    uint8_t i;

    EXTI_ClearEXTIFlag(EXTI_EINT0);

    for (i = 0; i < 10; i++)
    {
        GPIO_SetValue(BRD_LED_1_CONNECTED_PORT, BRD_LED_1_CONNECTED_MASK);

        delay();

        GPIO_ClearValue(BRD_LED_1_CONNECTED_PORT, BRD_LED_1_CONNECTED_MASK);

        delay();
    }
}

/*********************************************************************//**
 * @brief       ADC interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void ADC_IRQHandler(void)
{
    int cnt = 0;

    NVIC_DisableIRQ(ADC_IRQn);

    for (cnt = 0; cnt < 10; cnt++)
    {
        GPIO_SetValue(BRD_LED_2_CONNECTED_PORT, BRD_LED_2_CONNECTED_MASK);

        delay();

        GPIO_ClearValue(BRD_LED_2_CONNECTED_PORT, BRD_LED_2_CONNECTED_MASK);

        delay();
    }
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       delay function
 * @param[in]   none
 * @return      None
 **********************************************************************/
void delay (void)
{
    volatile unsigned int i;

    for (i = 0; i < 0x200000; i++)
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
    EXTI_InitTypeDef EXTICfg;
    
    GPIO_Init();

    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    PINSEL_ConfigPin(2, 10,1);

    ADC_Init(LPC_ADC, 100);
    ADC_IntConfig(LPC_ADC, BRD_ADC_PREPARED_INTR, ENABLE);
    ADC_ChannelCmd(LPC_ADC, BRD_ADC_PREPARED_CHANNEL, ENABLE);

    GPIO_SetDir(BRD_LED_1_CONNECTED_PORT, BRD_LED_1_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);
    GPIO_SetDir(BRD_LED_2_CONNECTED_PORT, BRD_LED_2_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);

    /* Initialize External 0 interrupt */
    EXTI_Init();

    EXTICfg.EXTI_Line = EXTI_EINT0;
    /* edge sensitive */
    EXTICfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    EXTICfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;

    EXTI_Config(&EXTICfg);

#if (INT_MODE == 0) //same group, different sub-levels (Tail-chaining example)
    //sets group priorities: 8 - subpriorities: 3
    NVIC_SetPriorityGrouping(4);

    //000:10 (bit 7:3)  assign eint0 to group 0, sub-priority 2 within group 0
    NVIC_SetPriority(EINT0_IRQn, 2);

    NVIC_SetPriority(ADC_IRQn, 0x01);
#else //different group - (Late-arriving example)  ==================================================
    //sets group priorities: 8 - subpriorities: 3
    NVIC_SetPriorityGrouping(4);

    //000:00 (bit 7:3) assign eint0 to group 0, sub-priority 0 within group 0
    NVIC_SetPriority(EINT0_IRQn, 0);

    NVIC_SetPriority(ADC_IRQn, 0x04);
#endif

    NVIC_EnableIRQ(EINT0_IRQn);

    /* Enable ADC in NVIC */
    NVIC_EnableIRQ(ADC_IRQn);

    while(1)
    {
        // Start conversion
        ADC_StartCmd(LPC_ADC, ADC_START_NOW);

        /* Enable ADC in NVIC */
        NVIC_EnableIRQ(ADC_IRQn);
    }
}

/* With ARM and GHS toolsets, the entry point is main() - this will
 allow the linker to generate wrapper code to setup stacks, allocate
 heap area, and initialize and copy code and data segments. For GNU
 toolsets, the entry point is through __start() in the crt0_gnu.asm
 file, and that startup code will setup stacks and data */
int main(void) {
    c_entry();
    return 0;
}



/**
 * @}
 */
