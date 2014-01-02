/**********************************************************************
* $Id$      Pwr_DeepSleep.c     2011-06-02
*//**
* @file     Pwr_DeepSleep.c
* @brief    This example describes how to enter the system in deep sleep
*           mode and wake-up by using external interrupt
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
#include "lpc_exti.h"
#include "lpc_pinsel.h"
#include "lpc_clkpwr.h"
#include "debug_frmwrk.h"

/** @defgroup PWR_DeepSleep Pwr Mgr Deep Sleep
 * @ingroup PWR_Examples
 * @{
 */

/************************** PRIVATE FUNCTIONS *************************/
uint8_t menu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" Power - Deep Sleep example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication115200 bps \n\r"
" This example used to enter system in deep sleep mode and wake up it by using \n\r"
" external interrupt\n\r"
"********************************************************************************\n\r";
/* Interrupt service routines */
void EINT0_IRQHandler(void);

void print_menu(void);
void delay (void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       External interrupt 0 handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void EINT0_IRQHandler(void)
{
      //clear the EINT0 flag
      EXTI_ClearEXTIFlag(EXTI_EINT0);

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

/*********************************************************************//**
 * @brief       Delay function
 * @param[in]   None
 * @return      None
 **********************************************************************/
void delay (void) {
  unsigned int i;

  for (i = 0; i < 0x100000; i++) {
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
    EXTI_InitTypeDef EXTICfg;

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

    /* Initialize EXT pin and registers
     * P2.10 as /EINT0
     */
    PINSEL_ConfigPin(2,10,1);

    EXTI_Init();

    EXTICfg.EXTI_Line = EXTI_EINT0;
    /* edge sensitive */
    EXTICfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    EXTICfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
    EXTI_ClearEXTIFlag(EXTI_EINT0);
    EXTI_Config(&EXTICfg);

    NVIC_SetPriorityGrouping(4);
    NVIC_SetPriority(EINT0_IRQn, 0);
    NVIC_EnableIRQ(EINT0_IRQn);

    _DBG_("Press '1' to enter system in deep sleep mode.\n\r"\
          "If you want to wake-up the system, press INT/WAKE-UP button.");
    while(_DG !='1')
    {
        _DBG_("I'm waiting...\n\r");
    }

    _DBG_("I'm sleeping...");
    // Enter target deep sleep mode
    CLKPWR_DeepSleep();
    SystemInit();
    debug_frmwrk_init();

    // MCU will be here after waking up
    _DBG_("\n\r-------- I'm wake up! -------- ");
    while (1);
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
