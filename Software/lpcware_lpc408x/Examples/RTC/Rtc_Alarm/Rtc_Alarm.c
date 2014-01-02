/**********************************************************************
* $Id$      Rtc_Alarm.c         2011-06-02
*//**
* @file     Rtc_Alarm.c
* @brief    This example describes how to use RTC to generate interrupt
*           in Second Counter Increment Interrupt (1s) and generate
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
#include "lpc_rtc.h"
#include "debug_frmwrk.h"


/** @defgroup RTC_Alarm RTC Alarm
 * @ingroup RTC_Examples
 * @{
 */


/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" RTC Alarm Example: \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" A simple RTC example. \n\r"
" To generate interrupt in Second Counter Increment Interrupt (1s) \n\r"
" and generate Alarm interrupt at 10s \n\r"
"********************************************************************************\n\r";

/************************** PRIVATE FUNCTION *************************/
void RTC_IRQHandler(void);

void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       RTC interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void RTC_IRQHandler(void)
{
    uint32_t secval;

    /* This is increment counter interrupt*/
    if (RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE))
    {
        secval = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);

        /* Send debug information */
        _DBG ("Second: "); _DBD(secval);
        _DBG_("");

        // Clear pending interrupt
        RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
    }

    /* Continue to check the Alarm match*/
    if (RTC_GetIntPending(LPC_RTC, RTC_INT_ALARM))
    {
        /* Send debug information */
        _DBG_ ("ALARM 10s matched!");

        // Clear pending interrupt
        RTC_ClearIntPending(LPC_RTC, RTC_INT_ALARM);
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
 * @brief       c_entry: Main RTC program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    RTC_TIME_Type RTCFullTime;

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

    /* RTC Block section ------------------------------------------------------ */
    // Init RTC module
    RTC_Init(LPC_RTC);

    /* Disable RTC interrupt */
    NVIC_DisableIRQ(RTC_IRQn);
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(RTC_IRQn, ((0x01<<3)|0x01));

    /* Enable rtc (starts increase the tick counter and second counter register) */
    RTC_ResetClockTickCounter(LPC_RTC);
    RTC_Cmd(LPC_RTC, ENABLE);
    RTC_CalibCounterCmd(LPC_RTC, DISABLE);

    /* Set current time for RTC */
    // Current time is 06:45:00PM, 2011-03-25
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, 0);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE, 45);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, 18);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, 3);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, 2011);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, 25);

    /* Set ALARM time for second */
    RTC_SetAlarmTime (LPC_RTC, RTC_TIMETYPE_SECOND, 10);

    // Get and print current time
    RTC_GetFullTime (LPC_RTC, &RTCFullTime);
    _DBG( "Current time set to: ");
    _DBD((RTCFullTime.HOUR)); _DBG (":");
    _DBD ((RTCFullTime.MIN)); _DBG (":");
    _DBD ((RTCFullTime.SEC)); _DBG("  ");
    _DBD ((RTCFullTime.DOM)); _DBG("/");
    _DBD ((RTCFullTime.MONTH)); _DBG("/");
    _DBD16 ((RTCFullTime.YEAR)); _DBG_("");

    _DBG("Second ALARM set to ");
    _DBD (RTC_GetAlarmTime (LPC_RTC, RTC_TIMETYPE_SECOND));
    _DBG_("s");

    /* Set the CIIR for second counter interrupt*/
    RTC_CntIncrIntConfig (LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);
    /* Set the AMR for 10s match alarm interrupt */
    RTC_AlarmIntConfig (LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);

    /* Enable RTC interrupt */
    NVIC_EnableIRQ(RTC_IRQn);

    /* Loop forever */
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
