/**********************************************************************
* $Id$      Timer_FreqMeasure.c         2011-06-02
*//**
* @file     Timer_FreqMeasure.c
* @brief    This example describes how to use TIMER to measure the 
*           the frequency of the signal input
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


/* Example group ----------------------------------------------------------- */
/** @defgroup TIMER_FreqMeasure     Timer Frequency Measurement
 * @ingroup TIMER_Examples
 * @{
 */
 

#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
#define _SIGNAL_GEN_TIM         (LPC_TIM3)
#define _SIGNAL_GEN_TIM_INTR    (TIMER3_IRQn)

#define TIM_MAT_LINKED_PORT     (0)
#define TIM_MAT_LINKED_PIN      (10)

#define _MEASURE_TIM            (LPC_TIM2)
#define _MEASURE_TIM_IRQHandler TIMER2_IRQHandler
#define _MEASURE_TIM_INTR       (TIMER2_IRQn)

#define TIM_CAP_LINKED_PORT     (0)
#define TIM_CAP_LINKED_PIN      (4)

#else
#define _SIGNAL_GEN_TIM         (LPC_TIM2)
#define _SIGNAL_GEN_TIM_INTR    (TIMER2_IRQn)

//@ J3.18
#define TIM_MAT_LINKED_PORT     (0)
#define TIM_MAT_LINKED_PIN      (6)

#define _MEASURE_TIM            (LPC_TIM0)
#define _MEASURE_TIM_IRQHandler TIMER0_IRQHandler
#define _MEASURE_TIM_INTR       (TIMER0_IRQn)

//@ J5.35
#define TIM_CAP_LINKED_PORT     (1)
#define TIM_CAP_LINKED_PIN      (26)

#endif



/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
"Hello NXP Semiconductors \n\r"
"Timer Frequency Measurement example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" Use timer 0 to measure input signal frequency through its CAP0.0 \n\r"
" Use timer 2 to generate different frequency signals \n\r"
"********************************************************************************\n\r";

#define NO_MEASURING_SAMPLE             (5)

TIM_TIMERCFG_Type TIM_ConfigStruct;
TIM_CAPTURECFG_Type TIM_CaptureConfigStruct;

__IO BOOL_8 first_capture,done;
__IO uint32_t capture;
__IO uint8_t count=0;
/************************** PRIVATE FUNCTIONS *************************/
/* Interrupt service routines */
void _MEASURE_TIM_IRQHandler(void);

void print_menu(void);
/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       TIMER interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void _MEASURE_TIM_IRQHandler(void)
{
    if (TIM_GetIntStatus(_MEASURE_TIM, TIM_CR0_INT))
    {
        TIM_ClearIntPending(_MEASURE_TIM, TIM_CR0_INT);

        if(first_capture == TRUE)
        {
            TIM_Cmd(_MEASURE_TIM, DISABLE);

            TIM_ResetCounter(_MEASURE_TIM);

            TIM_Cmd(_MEASURE_TIM, ENABLE);

            count++;

            if(count == NO_MEASURING_SAMPLE)
                first_capture = FALSE; //stable
        }
        else
        {
            count = 0; //reset count for next use

            done = TRUE;

            capture = TIM_GetCaptureValue(_MEASURE_TIM, TIM_COUNTER_INCAP0);
        }
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
    TIM_MATCHCFG_Type TIM_MatchConfigStruct;
    uint8_t idx;
    uint16_t tem;
    uint32_t freq,temcap;

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

    //Config P1.26 as CAP0
    PINSEL_ConfigPin(TIM_CAP_LINKED_PORT, TIM_CAP_LINKED_PIN, 3);

    // Configure P0.6 as MAT2.0
    PINSEL_ConfigPin(TIM_MAT_LINKED_PORT, TIM_MAT_LINKED_PIN, 3);

    while(1)
    {
        // Initialize timer 0, prescale count time of 1uS
        TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
        TIM_ConfigStruct.PrescaleValue  = 1;

        // use channel 0, CAPn.0
        TIM_CaptureConfigStruct.CaptureChannel = 0;
        // Enable capture on CAPn.0 rising edge
        TIM_CaptureConfigStruct.RisingEdge = ENABLE;
        // Enable capture on CAPn.0 falling edge
        TIM_CaptureConfigStruct.FallingEdge = DISABLE;
        // Generate capture interrupt
        TIM_CaptureConfigStruct.IntOnCaption = ENABLE;

        // Set configuration for Tim_config and Tim_MatchConfig
        TIM_Init(_MEASURE_TIM, TIM_TIMER_MODE,&TIM_ConfigStruct);

        TIM_ConfigCapture(_MEASURE_TIM, &TIM_CaptureConfigStruct);

        TIM_ResetCounter(_MEASURE_TIM);

        idx = 0;
        freq = 0;
        tem = 0;

        while(idx < 3)
        {
            if(idx == 0)
                _DBG("\n\rPlease input frequency (from 1 to 999 hz):");

            tem = _DG;

            switch(tem)
            {
                case '0':

                case'1':

                case '2':

                case '3':

                case '4':

                case '5':

                case '6':

                case '7':

                case'8':

                case '9':
                {
                    tem = tem - 0x30;

                    idx++;

                    if(idx == 1)
                    {
                        tem = tem * 100;
                    }
                    else if (idx == 2)
                    {
                        tem = tem * 10;
                    }

                    freq = freq + tem;

                    if(idx == 3)
                    {
                        //Show the freq input
                        _DBD16(freq);
                    }

                    tem = 0;

                    break;
                }

                default:
                {
                    _DBG("...Please input digits from 0 to 9 only!");
                    idx = 0;
                    tem = 0;
                    freq = 0;
                    break;
                }
            }
        }

        TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
        TIM_ConfigStruct.PrescaleValue  = 1;//1us
        TIM_Init(_SIGNAL_GEN_TIM, TIM_TIMER_MODE,&TIM_ConfigStruct);

        // use channel 0, MR0
        TIM_MatchConfigStruct.MatchChannel = 0;
        // Disable interrupt when MR0 matches the value in TC register
        TIM_MatchConfigStruct.IntOnMatch   = FALSE;
        //Enable reset on MR0: TIMER will reset if MR0 matches it
        TIM_MatchConfigStruct.ResetOnMatch = TRUE;
        //Stop on MR0 if MR0 matches it
        TIM_MatchConfigStruct.StopOnMatch  = FALSE;
        //Toggle MR0.0 pin if MR0 matches it
        TIM_MatchConfigStruct.ExtMatchOutputType =TIM_EXTMATCH_TOGGLE;
        // Set Match value
        TIM_MatchConfigStruct.MatchValue   = 500000/freq;
        TIM_ConfigMatch(_SIGNAL_GEN_TIM,&TIM_MatchConfigStruct);

        TIM_Cmd(_SIGNAL_GEN_TIM,ENABLE);

        /* preemption = 1, sub-priority = 1 */
        NVIC_SetPriority(_MEASURE_TIM_INTR, ((0x01 << 3) | 0x01));

        /* Enable interrupt for timer 0 */
        NVIC_EnableIRQ(_MEASURE_TIM_INTR);

        first_capture = TRUE;
        done = FALSE;
        capture = 0;

        // To start timer 0
        TIM_Cmd(_MEASURE_TIM,ENABLE);

        _DBG("\n\rMeasuring......");

        while(done == FALSE);

        temcap = 1000000 / capture;

        _DBD16(temcap);_DBG("Hz");

        NVIC_DisableIRQ(_MEASURE_TIM_INTR);

        TIM_DeInit(_MEASURE_TIM);

        TIM_DeInit(_SIGNAL_GEN_TIM);

        _DBG("\n\rPress 'c' or 'C' to continue measuring other signals...");

        while((_DG != 'c') && (_DG != 'C'));
    }
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
