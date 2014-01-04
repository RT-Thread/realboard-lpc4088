/**********************************************************************
* $Id$      Systick_Stclk.c         2011-06-02
*//**
* @file     Systick_Stclk.c
* @brief    This example describes how to configure System Tick to use
*           with external clock source STCLK
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
#include "lpc_systick.h"
#include "lpc_pinsel.h"
#include "lpc_timer.h"


/* Example group ----------------------------------------------------------- */
/** @defgroup SysTick_StCtclk   Systick STCLK
 * @ingroup SysTick_Examples
 * @{
 */
 

#define PIO_PORT_USED       (2)
#define PIO_PIN_USED        (10)

#define PIO_PIN_VALUE       (1 << PIO_PIN_USED)

/************************** PRIVATE VARIABLES *************************/
FunctionalState Cur_State = ENABLE;
Bool IO_State = FALSE;

TIM_TIMERCFG_Type TIM_ConfigStruct;
TIM_MATCHCFG_Type TIM_MatchConfigStruct;


/************************** PRIVATE FUNCTIONS *************************/
void SysTick_Handler(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       SysTick interrupt handler
 * @param       None
 * @return      None
 ***********************************************************************/
void SysTick_Handler(void)
{
    //Clear System Tick counter flag
    SYSTICK_ClearCounterFlag();

    //toggle P0.0
    if (Cur_State == ENABLE)
    {
        //pull-down pin
        GPIO_ClearValue(PIO_PORT_USED, PIO_PIN_VALUE);
        Cur_State = DISABLE;
    }
    else
    {
        GPIO_SetValue(PIO_PORT_USED, PIO_PIN_VALUE);
        Cur_State = ENABLE;
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
    
    //control buffer transceiver to enable output for P3.26
    GPIO_SetDir(4, (1<<29),1);
    GPIO_ClearValue (4, (1<<29));
    GPIO_SetDir(4, (1<<24), 1);
    GPIO_ClearValue(4, (1<<24));

    // Conifg P1.28 as MAT0.0
    PINSEL_ConfigPin(1, 28, 3);

    /* P3.26 as STCLK */
    PINSEL_ConfigPin(3, 26, 4);

    // Initialize timer 0, prescale count time of 10uS
    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue  = 10;

    // use channel 0, MR0
    TIM_MatchConfigStruct.MatchChannel = 0;
    // Disable interrupt when MR0 matches the value in TC register
    TIM_MatchConfigStruct.IntOnMatch   = TRUE;
    //Enable reset on MR0: TIMER will reset if MR0 matches it
    TIM_MatchConfigStruct.ResetOnMatch = TRUE;
    //Stop on MR0 if MR0 matches it
    TIM_MatchConfigStruct.StopOnMatch  = FALSE;
    //Toggle MR0.0 pin if MR0 matches it
    TIM_MatchConfigStruct.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
    // Set Match value, count value of 10 (10 * 10uS = 100uS --> 10KHz)
    TIM_MatchConfigStruct.MatchValue   = 10;

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);

    TIM_ConfigMatch(LPC_TIM0, &TIM_MatchConfigStruct);

    TIM_Cmd(LPC_TIM0, ENABLE);

    GPIO_SetDir(PIO_PORT_USED, PIO_PIN_VALUE, 1);

    //Use P0.0 to test System Tick interrupt
    /* Initialize System Tick with 10ms time interval
     * Frequency input = 10kHz /2 = 5kHz
     * Time input = 10ms
     */
    SYSTICK_ExternalInit(5000, 10);

    //Enable System Tick interrupt
    SYSTICK_IntCmd(ENABLE);

    //Enable System Tick Counter
    SYSTICK_Cmd(ENABLE);

    while(1);
    
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



/**
 * @}
 */
