/**********************************************************************
* $Id$      Timer_MatchInterrupt.c          2011-06-02
*//**
* @file     Timer_MatchInterrupt.c
* @brief    This example describes how to use TIMER in interrupt mode
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
#include "lpc_timer.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"
#include "bsp.h"


/** @defgroup TIMER_MatchInterrupt      Timer Match Interrupt
 * @ingroup TIMER_Examples
 * @{
 */
 
 
/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" Timer Match Interrupt demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" Use timer x toggle MATx.0 at frequency 1Hz\n\r"
"********************************************************************************\n\r";

//timer init
TIM_TIMERCFG_Type TIM_ConfigStruct;
TIM_MATCHCFG_Type TIM_MatchConfigStruct ;
uint8_t volatile timer0_flag = FALSE, timer1_flag = FALSE;
FunctionalState LEDStatus = ENABLE;

/************************** PRIVATE FUNCTION *************************/
/* Interrupt service routine */
void TIMER0_IRQHandler(void);
void TIMER2_IRQHandler(void);

void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       TIMER0 interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void TIMER0_IRQHandler(void)
{
    if (TIM_GetIntStatus(BRD_TIMER_USED, TIM_MR0_INT)== SET)
    {
        _DBG_("Match interrupt occur...");
    }
    
    TIM_ClearIntPending(BRD_TIMER_USED, TIM_MR0_INT);
}

/*********************************************************************//**
 * @brief       TIMER0 interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void TIMER2_IRQHandler(void)
{
    if (TIM_GetIntStatus(BRD_TIMER_USED, TIM_MR0_INT)== SET)
    {
        _DBG_("Match interrupt occur...");
    }
    
    TIM_ClearIntPending(BRD_TIMER_USED, TIM_MR0_INT);
}


/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Print menu
 * @param[in]   None
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG(menu1);
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main TIMER program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    // print welcome screen
    print_menu();

    // Conifg P1.28 as MAT0.0
    PINSEL_ConfigPin(BRD_TIM_CAP_LINKED_PORT, BRD_TIM_CAP_LINKED_PIN, 3);

    // Initialize timer 0, prescale count time of 100uS
    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue  = 100;

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
    // Set Match value, count value of 10000 (10000 * 100uS = 1000000us = 1s --> 1 Hz)
    TIM_MatchConfigStruct.MatchValue   = 10000;

    // Set configuration for Tim_config and Tim_MatchConfig
    TIM_Init(BRD_TIMER_USED, TIM_TIMER_MODE, &TIM_ConfigStruct);
    TIM_ConfigMatch(BRD_TIMER_USED, &TIM_MatchConfigStruct);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(BRD_TIM_INTR_USED, ((0x01<<3)|0x01));

    /* Enable interrupt for timer 0 */
    NVIC_EnableIRQ(BRD_TIM_INTR_USED);

    // To start timer
    TIM_Cmd(BRD_TIMER_USED, ENABLE);

    while (1);

}

/* Support required entry point for other toolchain */
int main (void)
{
    c_entry();
    return 0;
}



/**
 * @}
 */
