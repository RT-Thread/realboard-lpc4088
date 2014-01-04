 /**********************************************************************
* $Id$      Wdt_WindowMode.c            2011-06-02
*//**
* @file     Wdt_WindowMode.c
* @brief    This example describes how to use WDT in window setting
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
#include "lpc_types.h"
#include "lpc_wwdt.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"
#include "lpc_gpio.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup WDT_WindowMode    Watchdog Window Mode
 * @ingroup WDT_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
//Watchodog time out in 5 seconds
#define WDT_TIMEOUT     5000000

#define WDT_WINDOW_VAL      (0x07FF & 0xFFFFFF)

#define LED_WDT_INDICATOR_PORT      (0)
#define LED_WDT_INDICATOR_PIN       (13)
#define LED_WDT_INDICATOR_VAL       (1 << LED_WDT_INDICATOR_PIN)


//Watchodog time out in 5 seconds
#define WDT_INTERRUPT_TIMEOUT   5000000

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"\n\r********************************************************************************\n\r"
"This Application is for the Windows feature of the WatchDog Timer\n\r"
"Hello NXP Semiconductors \n\r"
"Watch dog Windows Mode example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - Communicate via: UART0 - 115200 bps \n\r"
"Use WDT with pre-configuration for Windows value \n\r"
"The feed to WDT is only valid if the Timer Counter value if it's below this\r\n"
"setting value. When it's above, an error occurs and interrupt requests\n\r"
#ifdef __RAM_MODE__
"The program is currently working in RAM mode\n\r"
#else
"The program is currently working in FLASH mode\n\r"
#endif
"********************************************************************************\n\r";

void DoWdtValidFeed(void);


/*********************************************************************//**
 * @brief       WDT interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void DoWdtValidFeed(void)
{
    WWDT_SetTimerConstant(0x0007FFFF);

    WWDT_Enable(ENABLE);

    WWDT_SetWindowRaw(0xFFFFFF);

    //At this time, the feed is ok,no problem occur
    WWDT_FeedStdSeq();
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main WDT program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    volatile uint32_t cnt;
    uint32_t wdtReset;


    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    // print welcome screen
    _DBG_(menu1);

    for(cnt = 0; cnt < 100000; cnt++);

    if (WWDT_GetStatus(WWDT_TIMEOUT_FLAG))
    {
        // Clear WDT TimeOut
        WWDT_ClrTimeOutFlag();

        wdtReset = TRUE;

        _DBG_("This is coming from WDT Reset");
    }
    else
    {
        wdtReset = FALSE;

        _DBG_("This is coming from External Reset");
    }

    if(wdtReset == FALSE)
    {
        WWDT_SetMode(WWDT_RESET_MODE, ENABLE);

        DoWdtValidFeed();
    }
    else
    {
        WWDT_SetMode(WWDT_RESET_MODE, DISABLE);
    }

    _DBG("Changing the Window Value"); _DBG_("");

    //The value of windows setting.
    WWDT_SetWindowRaw(WDT_WINDOW_VAL);

    for(cnt = 0; cnt < 256; cnt++);


    WWDT_FeedStdSeq();

    for(cnt = 0; cnt < 10000; cnt++);

    _DBG("The WDT is fed OK"); _DBG_("");

    //_DBG("Current Timer Value: ");_DBH32(wdt_CntVal); _DBG_("");

    //_DBG("Windows Setting Value: ");_DBH32(WDT_WINDOW_VAL); _DBG_("");

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
