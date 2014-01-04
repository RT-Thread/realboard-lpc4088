/**********************************************************************
* $Id$      Timer_Capture.c         2011-06-02
*//**
* @file     Timer_Capture.c
* @brief    This example describes how to use TIMER capture function
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
#include "lpc_gpio.h"
#include "bsp.h"

/** @defgroup Timer_Capture Timer Capture
 * @ingroup TIMER_Examples
 * @{
 */
 

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" Timer Capture Example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" Use timer 0 to take a snapshot of the timer value when an input signal \n\r"
" on CAP0.0 transitions \n\r"
"********************************************************************************\n\r";

// UART Configuration structure variable
UART_CFG_Type UARTConfigStruct;
uint32_t MatchCount;

//timer init
TIM_TIMERCFG_Type TIM_ConfigStruct;
TIM_MATCHCFG_Type TIM_MatchConfigStruct ;
TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;
uint8_t volatile timer0_flag = FALSE, timer1_flag = FALSE;

/************************** PRIVATE FUNCTIONS *************************/
/* Interrupt service routines */
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
    if (TIM_GetIntStatus(BRD_TIMER_USED, TIM_CR0_INT))
    {
        TIM_ClearIntPending(BRD_TIMER_USED, TIM_CR0_INT);
        
        _DBG("Time capture: ");

        _DBH32(TIM_GetCaptureValue(BRD_TIMER_USED, TIM_COUNTER_INCAP0));_DBG_("");
    }
}

/*********************************************************************//**
 * @brief       TIMER2 interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void TIMER2_IRQHandler(void)
{
    if (TIM_GetIntStatus(BRD_TIMER_USED, TIM_CR0_INT))
    {
        TIM_ClearIntPending(BRD_TIMER_USED, TIM_CR0_INT);
        
        _DBG("Time capture: ");

        _DBH32(TIM_GetCaptureValue(BRD_TIMER_USED, TIM_COUNTER_INCAP0));_DBG_("");
    }
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

    //Config pin as CAP0.0 function
    PINSEL_ConfigPin(BRD_TIM_CAP_LINKED_PORT, BRD_TIM_CAP_LINKED_PIN, 3);

    // Initialize timer 0, prescale count time of 1000000uS = 1S
    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue  = 1000000;

    // use channel 0, CAPn.0
    TIM_CaptureConfigStruct.CaptureChannel = 0;
    // Enable capture on CAPn.0 rising edge
    TIM_CaptureConfigStruct.RisingEdge = ENABLE;
    // Enable capture on CAPn.0 falling edge
    TIM_CaptureConfigStruct.FallingEdge = ENABLE;
    // Generate capture interrupt
    TIM_CaptureConfigStruct.IntOnCaption = ENABLE;


    // Set configuration for Tim_config and Tim_MatchConfig
    TIM_Init(BRD_TIMER_USED, TIM_TIMER_MODE, &TIM_ConfigStruct);
    TIM_ConfigCapture(BRD_TIMER_USED, &TIM_CaptureConfigStruct);
    TIM_ResetCounter(BRD_TIMER_USED);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(BRD_TIM_INTR_USED, ((0x01<<3)|0x01));

    /* Enable interrupt for timer 0 */
    NVIC_EnableIRQ(BRD_TIM_INTR_USED);

    // To start timer 0
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
