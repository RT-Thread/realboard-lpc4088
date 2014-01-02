/**********************************************************************
* $Id$      Systick_10msBase.c          2011-06-02
*//**
* @file     Systick_10msBase.c
* @brief    This example describes how to configure System Tick to generate
*           interrupt each 10ms
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

#include "lpc_gpio.h"
#include "lpc_systick.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup SysTick_10msBase  Systick 10 milisecond base
 * @ingroup SysTick_Examples
 * @{
 */

#define PIO_PORT_USED       (0)
#define PIO_PIN_USED        (26)

#define PIO_PIN_VALUE       (1 << PIO_PIN_USED)

/************************** PRIVATE VARIABLES *************************/
FunctionalState Cur_State = ENABLE;

/************************** PRIVATE FUNCTIONS *************************/
void SysTick_Handler(void);

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

    //toggle GPIO pin
    GPIO_OutputValue(PIO_PORT_USED, PIO_PIN_VALUE, Cur_State);
    Cur_State = (Cur_State == ENABLE) ? DISABLE:ENABLE;
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry (void)
{
    GPIO_Init();
    
    //Use GPIO to test System Tick interrupt
    GPIO_SetDir(PIO_PORT_USED, PIO_PIN_VALUE, 1);

    //Initialize System Tick with 10ms time interval
    SYSTICK_InternalInit(10);

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
