/**********************************************************************
* $Id$      Nvic_VectorTableRelocation.c    2011-06-02
*//**
* @file     Nvic_VectorTableRelocation.c
* @brief    This example used to test NVIC Vector Table relocation
*           function
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
#include <stdio.h>
#include <string.h>
#include "lpc_types.h"
#include "debug_frmwrk.h"
#include "lpc_gpio.h"
#include "lpc_nvic.h"
#include "lpc_systick.h"
#include "lpc_clkpwr.h"
#include "bsp.h"



/** @defgroup NVIC_VectorTableRelocation    NVIC Vector Table Relocation
 * @ingroup NVIC_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS ***********************/
/* Vector Table Offset */
#define VTOR_OFFSET     0x20001000

#define NO_TIME_LED_ON              (150)
#define NO_TIME_LED_OFF             (150)


/************************** PRIVATE VARIABLES *************************/
uint8_t menu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" Privileged demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" This example used to test NVIC Vector Table Relocation function\n\r"
"********************************************************************************\n\r";
FunctionalState Cur_State = DISABLE;

/************************** PRIVATE FUNCTIONS *************************/

void SysTick_Handler(void);
void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       SysTick interrupt handler
 * @param       None
 * @return      None
 ***********************************************************************/
void SysTick_Handler(void)
{
    //Clear System Tick counter flag
    SYSTICK_ClearCounterFlag();

    GPIO_OutputValue(BRD_LED_1_CONNECTED_PORT, BRD_LED_1_CONNECTED_MASK, Cur_State);
    Cur_State = (Cur_State == ENABLE)? DISABLE:ENABLE;
}
/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Print Welcome menu
 * @param[in]   none
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG_(menu);
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry (void)
{
    uint8_t* pDest =    (uint8_t*)VTOR_OFFSET;
    uint8_t* pSource = NULL;
    GPIO_Init();
    
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

    GPIO_SetDir(BRD_LED_1_CONNECTED_PORT, BRD_LED_1_CONNECTED_MASK, 1);

    _DBG(" Remapping Vector Table at address: ");

    _DBH32(VTOR_OFFSET); _DBG_("");

    NVIC_SetVTOR(VTOR_OFFSET);

    /* Copy Vector Table from 0x00000000 to new address
     * In ROM mode: Vector Interrupt Table is initialized at 0x00000000
     * In RAM mode: Vector Interrupt Table is initialized at 0x10000000
     * Aligned: 256 words
     */


#ifdef __RAM_MODE__ //Run in RAM mode
  pSource =  (void*)0x10000000;
  memcpy(pDest,pSource , 256*4);
#else
  pSource = (void*)0x00000000;
  memcpy(pDest,pSource , 256*4);
#endif

    _DBG_(" If Vector Table remapping is successful, LED P2.10 will blink by using\n\r SysTick interrupt");
    //Initialize System Tick with 100ms time interval
    /* Input parameter for SysTick in range 0..174 ms */
    SYSTICK_InternalInit(100);

    //Enable System Tick interrupt
    SYSTICK_IntCmd(ENABLE);

    //Enable System Tick Counter
    SYSTICK_Cmd(ENABLE);

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
