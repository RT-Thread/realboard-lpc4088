/**********************************************************************
* $Id$      RTC_EV.c            2011-06-02
*//**
* @file     RTC_EV.c
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
#include "lpc_types.h"
#include "LPC407x_8x_177x_8x.h"
#include "lpc_rtc.h"
#include "debug_frmwrk.h"
#include "lpc_clkpwr.h"
#include "lpc_pinsel.h"


/** @defgroup RTC_EV    RTC Event Monitor/Recorder
 * @ingroup RTC_Examples
 * @{
 */



#define ER_INPUT_CHANNEL_TEST_NUM       (2)

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" Event Monitor/Recorder Example: \n\r"
"\t - MCU: LPC407x_8x_177x_8x \n\r"
"\t - Core: ARM CORTEX-M4/M3 \n\r"
"\t - UART Communication: 115200 bps \n\r"
" This is a simple example for Event Monitor/Recoder.\n\r"
" Press S to enter to sleep mode\n\r"
" Press P to enter to power down mode\n\r"
" Press D to enter to deep power down mode\n\r"
"********************************************************************************\n\r";
volatile uint8_t evt_cnt = 0;
volatile RTC_ER_TIMESTAMP_Type FirstTimeStamp;
volatile RTC_ER_TIMESTAMP_Type LastTimeStamp;

/************************** PRIVATE FUNCTION *************************/
void RTC_IRQHandler(void);

void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       RTC(Real-time clock) interrupt handler
 * @param[in]   none
 * @return      None
 **********************************************************************/
void RTC_IRQHandler(void)
{
  uint32_t er_status;

  if(RTC_GetIntPending(LPC_RTC, RTC_INT_ALARM|RTC_INT_COUNTER_INCREASE))
  {
    RTC_ClearIntPending(LPC_RTC, RTC_INT_ALARM|RTC_INT_COUNTER_INCREASE);
  }

  // Get status
  er_status = RTC_ER_GetStatus(); 

  // Get events
  evt_cnt =  RTC_ER_GetEventCount(ER_INPUT_CHANNEL_TEST_NUM); 
  RTC_ER_GetFirstTimeStamp(ER_INPUT_CHANNEL_TEST_NUM,(RTC_ER_TIMESTAMP_Type*) &FirstTimeStamp);
  RTC_ER_GetLastTimeStamp(ER_INPUT_CHANNEL_TEST_NUM, (RTC_ER_TIMESTAMP_Type*) &LastTimeStamp);

  // Clear status
  RTC_ER_ClearStatus(er_status);
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
/*********************************************************************//**
 * @brief       Set System Time
 * @param[in]   None
 * @return      None
 **********************************************************************/
void SetSystemTime(void)
{
    RTC_TIME_Type RTCFullTime;

    /* RTC Block section ------------------------------------------------------ */
    // Init RTC module
    RTC_Init(LPC_RTC);

    /* Enable rtc (starts increase the tick counter and second counter register) */
    RTC_ResetClockTickCounter(LPC_RTC);
    RTC_Cmd(LPC_RTC, ENABLE);

    /* Set current time for RTC */
    // Current time is 06:45:00PM, 2012-03-01
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_SECOND, 0);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MINUTE, 45);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_HOUR, 18);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_MONTH, 1);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_YEAR, 2012);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFMONTH, 1);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFYEAR, 1);
    RTC_SetTime (LPC_RTC, RTC_TIMETYPE_DAYOFWEEK, 6);

    // Get and print current time
    RTC_GetFullTime (LPC_RTC, &RTCFullTime);
    _DBG( "Current time set to: ");
    _DBD((RTCFullTime.HOUR)); _DBG (":");
    _DBD ((RTCFullTime.MIN)); _DBG (":");
    _DBD ((RTCFullTime.SEC)); _DBG("  ");
    _DBD ((RTCFullTime.DOM)); _DBG("/");
    _DBD ((RTCFullTime.MONTH)); _DBG("/");
    _DBD16 ((RTCFullTime.YEAR)); _DBG(" (DOY = ");
    _DBD16 ((RTCFullTime.DOY)); _DBG(", DOW = ");
    _DBD16 ((RTCFullTime.DOW)); _DBG_(")");
}
/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main RTC program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    RTC_ER_CONFIG_Type  ErInit;
    uint8_t uart_in;
    uint32_t tmp = 0;
    
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

start:
    SetSystemTime();

    /* Pin Configuration for Event Monitor/Recoder */
    PINSEL_ConfigPin(0,7,4);
    PINSEL_ConfigPin(0,8,4);
    PINSEL_ConfigPin(0,9,4);

    // Init RTC_ER
    RTC_ER_InitConfigStruct(&ErInit);
    ErInit.InputChannel[ER_INPUT_CHANNEL_TEST_NUM].EventOnPosEdge = TRUE;
    ErInit.InputChannel[ER_INPUT_CHANNEL_TEST_NUM].IntWake= TRUE;
    if(RTC_ER_Init(&ErInit) == SUCCESS)
    {
        // Enable RTC_ER
        RTC_ER_Cmd(ER_INPUT_CHANNEL_TEST_NUM,ENABLE);

        // Enable Interrupt
        NVIC_EnableIRQ(RTC_IRQn);

        while(1)
        {
             uart_in = 0;
            _DG_NONBLOCK(&uart_in);
            switch(uart_in)
            {
                case 'S':
                case 's':
                    _DBG_("Enter to Sleep Mode");
                    CLKPWR_Sleep();
                    _DBG_("\n\rSystem wake-up!\n\r");
                    break;
                case 'P':
                case 'p':
                    _DBG_("Enter to Power Down Mode");
                    CLKPWR_PowerDown();
                    SystemInit();
                    debug_frmwrk_init();
                    _DBG_("\n\rSystem wake-up!\n\r");
                    goto start;
                case 'D':
                case 'd':
                    _DBG_("Enter to Deep Power Down Mode");
                    CLKPWR_DeepPowerDown();
                    SystemInit();
                    debug_frmwrk_init();
                    _DBG_("\n\rSystem wake-up!\n\r");
                    goto start;
                default:
                    break;
            }
           
            if(evt_cnt)
            {
                _DBG("Event Count: ");_DBD(evt_cnt);_DBG_("");
                _DBG( "First TimeStamp: ");
                _DBD((FirstTimeStamp.HOUR)); _DBG (":");
                _DBD ((FirstTimeStamp.MIN)); _DBG (":");
                _DBD ((FirstTimeStamp.SEC)); _DBG(", DOY = ");
                _DBD ((FirstTimeStamp.DOY)); _DBG_("");
                _DBG( "Last TimeStamp: ");
                _DBD((LastTimeStamp.HOUR)); _DBG (":");
                _DBD ((LastTimeStamp.MIN)); _DBG (":");
                _DBD ((LastTimeStamp.SEC));  _DBG(", DOY = ");
                _DBD ((LastTimeStamp.DOY)); _DBG_("");
                evt_cnt = 0;
            }
            for(tmp = 0; tmp < 100000; tmp++);
        }
        
    }
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
