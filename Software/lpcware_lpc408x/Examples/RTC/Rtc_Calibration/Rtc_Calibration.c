/**********************************************************************
* $Id$      Rtc_Calibration.c           2011-06-02
*//**
* @file     Rtc_Calibration.c
* @brief    This example describes how to calibrate real time clock.
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
#include "lpc_clkpwr.h"
#include "debug_frmwrk.h"
#include "lpc_rtc.h"


/** @defgroup RTC_Calibration   RTC Calibration
 * @ingroup RTC_Examples
 * @{
 */


/************************** PRIVATE VARIABLES *************************/
uint8_t menu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" RTC Calibration demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" This example describes how to calibrate RTC \n\r"
"********************************************************************************\n\r";

/************************** PRIVATE FUNCTIONS *************************/
void print_menu(void);
__IO uint32_t secval;
/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       RTC interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void RTC_IRQHandler(void)
{
    /* This is increment counter interrupt*/
    if (RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE))
    {
        secval = RTC_GetTime (LPC_RTC, RTC_TIMETYPE_SECOND);

        // Clear pending interrupt
        RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
    }
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Print Welcome menu
 * @param[in]   none
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG(menu);
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry (void)
{
    uint32_t pre_secval = 0, inc = 0, calib_cnt = 0;
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

    /* In this example:
     * Suppose that the RTC need periodically adjust after each 5 second.
     * And the time counter need by incrementing the counter by 2 instead of 1
     * We will observe timer counter after calibration via serial display
     */
    // Init RTC module
    RTC_Init(LPC_RTC);

    /* Enable rtc (starts increase the tick counter and second counter register) */
    RTC_ResetClockTickCounter(LPC_RTC);
    RTC_Cmd(LPC_RTC, ENABLE);

    //Set current time = 0
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, 0);

    /* Setting Timer calibration
     * Calibration value =  6s;
     * Direction = Forward calibration
     * So after each 6s, calibration logic can periodically adjust the time counter by
     * incrementing the counter by 2 instead of 1
     */
    RTC_CalibConfig(LPC_RTC, 6, RTC_CALIB_DIR_FORWARD);
    RTC_CalibCounterCmd(LPC_RTC, ENABLE);

    /* Set the CIIR for second counter interrupt*/
    RTC_CntIncrIntConfig (LPC_RTC, RTC_TIMETYPE_SECOND, ENABLE);

    /* Enable RTC interrupt */
    NVIC_EnableIRQ(RTC_IRQn);

    /* Loop forever */
    while(1)
    {
        if(pre_secval != secval)
        {   
            if(pre_secval > secval)
                inc = 60 + secval - pre_secval;
            else
                inc = secval - pre_secval; 
            if(inc > 1)
            {
               _DBG ("Second: "); _DBD(secval); _DBG("--> Increase ");_DBD(inc); _DBG(" after ");_DBD(calib_cnt);_DBG(" seconds");
                _DBG_(""); 
                calib_cnt = 0;
            }   
            else
            {
                _DBG ("Second: "); _DBD(secval);  _DBG_(""); 
                calib_cnt++;
            }
            pre_secval = secval;
        }
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



/**
 * @}
 */
