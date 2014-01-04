/**********************************************************************
* $Id$      Wdt_Interrupt.c         2011-06-02
*//**
* @file     Wdt_Interrupt.c
* @brief    This example describes how to use Watch-dog timer application
*           in interrupt mode
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
#include "bsp.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup WDT_Interrupt Watchdog Interrupt Mode
 * @ingroup WDT_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD) 
#define LED_WDT_INDICATOR_PORT      (1)
#define LED_WDT_INDICATOR_PIN       (13)
#define LED_WDT_INDICATOR_VAL       (1 << LED_WDT_INDICATOR_PIN)
#else
#define LED_WDT_INDICATOR_PORT      (0)
#define LED_WDT_INDICATOR_PIN       (13)
#define LED_WDT_INDICATOR_VAL       (1 << LED_WDT_INDICATOR_PIN)
#endif


//Watchodog time out in 10 seconds
#define WDT_INTERRUPT_TIMEOUT   10000000


/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" Watch dog timer interrupt (test or debug mode) demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" An interrupt will be generated once WWDT is timeout (depend on configuration)\r\n" 
" or the counter is reached the Warning Value.\r\n"
" After interrupt WDT interrupt is disabled immediately! \n\r"
"********************************************************************************\n\r";

uint8_t info1[] = "BEFORE WDT interrupt!\n\r";

uint8_t info2[] = "AFTER WDT interrupt\n\r";

__IO Bool wdt_flag = FALSE;
__IO Bool LED_toggle = FALSE;


/************************** PRIVATE FUNCTION *************************/
void WDT_IRQHandler(void);

void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       WDT interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void WDT_IRQHandler(void)
{
    // Disable WDT interrupt
    NVIC_DisableIRQ(WDT_IRQn);
    
    // Set WDT flag according
    if (wdt_flag == TRUE)
    {
        wdt_flag = FALSE;
    }
    else
    {
        wdt_flag = TRUE;
    }

    _DBG("\r\nThe Timer Value causes the Interrupt: ");_DBH32(WWDT_GetCurrentCount());  _DBG_("");
    
    // Clear TimeOut flag
    WWDT_ClrTimeOutFlag();
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
 * @brief       c_entry: Main WDT program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    volatile uint32_t delay;
    uint8_t ch;
    
    GPIO_Init();
    
    // Init LED port
    GPIO_SetDir(LED_WDT_INDICATOR_PORT, LED_WDT_INDICATOR_VAL, GPIO_DIRECTION_OUTPUT);
    GPIO_SetValue(LED_WDT_INDICATOR_PORT, LED_WDT_INDICATOR_VAL);

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

    /* Install interrupt for WDT interrupt */
    NVIC_SetPriority(WDT_IRQn, 0x10);

    // Init WDT, IRC OSC, interrupt mode
    WWDT_Init(1000000);

    _DBG_(info1);

    _DBG_("Press '1' to enable Watchdog timer Interrupt by Timeout only...\n\r");
    _DBG_("Press '2' to enable Watchdog timer Interrupt by Warning ...\n\r");

    do
    {
        ch = _DG;
    }
    while((ch !='1') && (ch != '2'));

    if(ch == '2')
    {
        _DBG_("Pressed '2' - Working with Warning Interrupt\n\r");

        WWDT_SetWarning(500000);
    }
    else
    {
        _DBG_("Pressed '1' - Working with Normal Timeout Interrupt\n\r");
    }
    // Start watchdog
    WWDT_Start(WDT_INTERRUPT_TIMEOUT);

    /* Enable the Watch dog interrupt*/
    NVIC_EnableIRQ(WDT_IRQn);

    while (1)
    {
        if (wdt_flag == FALSE)
        {

            while(wdt_flag == FALSE)
            {
                _DBH32(WWDT_GetCurrentCount()); _DBG_("");

                for(delay = 0; delay < 1000; delay ++);
            }
        } 
        else 
        {
            // after WDT interrupt
            _DBG_(info2);
            
            _DBG_("LED is blinking...");

            while(wdt_flag == TRUE)
            {
                if (LED_toggle == FALSE)
                {
                    //Turn on LED
                    GPIO_SetValue(LED_WDT_INDICATOR_PORT, LED_WDT_INDICATOR_VAL);

                    LED_toggle = TRUE;
                }
                else
                {
                    //Turn off LED
                    GPIO_ClearValue(LED_WDT_INDICATOR_PORT, LED_WDT_INDICATOR_VAL);

                    LED_toggle = FALSE;
                }
                
                //delay
                for(delay = 0; delay<1000000; delay ++);
            }
        }
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
