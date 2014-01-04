/**********************************************************************
* $Id$      Wdt_Reset.c         2011-06-02
*//**
* @file     Wdt_Reset.c
* @brief    This example describes how to use WDT in reset mode
* @version  2.0
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
 
#include "lpc_wwdt.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"
#include "lpc_gpio.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup WDT_Reset Watchdog Reset Mode
 * @ingroup WDT_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/


//Watchodog time out in 5 seconds
#define WDT_TIMEOUT     5000000


/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
"This Welcome Screen below will executive after reset event \n\r"
"Hello NXP Semiconductors \n\r"
"Watch dog timer reset when timeout demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
"Use WDT with Internal RC OSC, reset mode, timeout = 5 seconds \n\r"
"To reset MCU when time out. After reset, program will determine what cause of "
"last reset time (external reset or WDT time-out)\n\r"
#ifdef __RAM_MODE__
"The program is currently working in RAM mode\n\r"
#else
"The program is currently working in FLASH mode\n\r"
#endif
"********************************************************************************\n\r";


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main WDT program body
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
    _DBG(menu1);
    
    // Read back TimeOut flag to determine previous timeout reset
    if (WWDT_GetStatus(WWDT_TIMEOUT_FLAG))
    {
        _DBG_("Last MCU reset caused by WDT TimeOut!\n\r");
        
        // Clear WDT TimeOut
        WWDT_ClrTimeOutFlag();
    } 
    else
    {
        _DBG_("Last MCU reset caused by External!\n\r");
    }

    // Initialize WDT, IRC OSC, interrupt mode, timeout = 5000000us = 5s
    WWDT_Init(WDT_TIMEOUT);

    WWDT_Enable(ENABLE);

    WWDT_SetMode(WWDT_RESET_MODE, ENABLE);
    
    // Start watchdog with timeout given
    WWDT_Start(WDT_TIMEOUT);

    WWDT_Feed();
    
    //infinite loop to wait chip reset from WDT
    while(1);

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
