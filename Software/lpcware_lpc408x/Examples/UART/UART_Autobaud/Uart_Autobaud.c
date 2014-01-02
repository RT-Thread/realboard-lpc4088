/**********************************************************************
* $Id$      Uart_Autobaud.c         2011-06-02
*//**
* @file     Uart_Autobaud.c
* @brief    This example describes how to configure UART using auto-baud
*           rate in interrupt mode
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
#include "bsp.h"
#include "lpc_uart.h"
#include "lpc_pinsel.h"


/** @defgroup UART_Autobaurate      UART Auto-baurate
 * @ingroup UART_Examples
 * @{
 */


/************************** PRIVATE DEFINITIONS *************************/
/* Macro UART test number, chose what UART will be tested, should be 0..2*/
/* But recommend just use 0/1/2 because these UART were supported on EA-LPC1788 board */
#define UART_TEST_NUM       1
#if (UART_TEST_NUM == 0)
#define _LPC_UART           UART_0
#define _UART_IRQ           UART0_IRQn
#define _UART_IRQHander     UART0_IRQHandler
#elif (UART_TEST_NUM == 1)
#define _LPC_UART           UART_1
#define _UART_IRQ           UART1_IRQn
#define _UART_IRQHander     UART1_IRQHandler
#elif (UART_TEST_NUM == 2)
#define _LPC_UART           UART_2
#define _UART_IRQ           UART2_IRQn
#define _UART_IRQHander     UART2_IRQHandler
#elif (UART_TEST_NUM == 3)
#define _LPC_UART           UART_3
#define _UART_IRQ           UART3_IRQn
#define _UART_IRQHander     UART3_IRQHandler
#elif (UART_TEST_NUM == 4)
#define _LPC_UART           UART_4
#define _UART_IRQ           UART4_IRQn
#define _UART_IRQHander     UART4_IRQHandler
#endif

/************************** PRIVATE VARIABLES *************************/
uint8_t syncmenu[] = "\n\r\n\rAutoBaudrate Status: Synchronous!\n\r";

uint8_t menu1[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" UART Auto-Baudrate example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" This example used to test UART component with autobaudrate function.\n\r"
" It will adjust its rate to synchronize with the sending data\n\r"
"  + Please press any key to be echoed\n\r"
"  + Press 'r' to re-show the welcome string\n\r"
"  + Press ESC to terminate\n\r"
"********************************************************************************\n\r";

uint8_t menu2[] = "\n\rUART Auto-Baudrate demo terminated!";


/* Synchronous Flag */
__IO FlagStatus Synchronous;

/************************** PRIVATE FUNCTIONS *************************/
void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief   UART0 interrupt handler sub-routine
 * @param   None
 * @return  None
 **********************************************************************/
void _UART_IRQHander(void)
{
    // Call Standard UART 0 interrupt handler
    uint32_t intsrc, tmp, tmp1;

    /* Determine the interrupt source */
    intsrc = UART_GetIntId(_LPC_UART);
    tmp = intsrc & UART_IIR_INTID_MASK;

    // Receive Line Status
    if (tmp == UART_IIR_INTID_RLS){
        // Check line status
        tmp1 = UART_GetLineStatus(_LPC_UART);
        // Mask out the Receive Ready and Transmit Holding empty status
        tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE \
                | UART_LSR_BI | UART_LSR_RXFE);
        // If any error exist
        if (tmp1) {
            while(tmp1){
                ; //implement error handling here
            }
        }
    }
    intsrc &= (UART_IIR_ABEO_INT | UART_IIR_ABTO_INT);
    // Check if End of auto-baudrate interrupt or Auto baudrate time out
    if (intsrc){
        // Clear interrupt pending
        if(intsrc & UART_IIR_ABEO_INT)
            UART_ABClearIntPending(_LPC_UART, UART_AUTOBAUD_INTSTAT_ABEO);
        if (intsrc & UART_IIR_ABTO_INT)
            UART_ABClearIntPending(_LPC_UART, UART_AUTOBAUD_INTSTAT_ABTO);
            if (Synchronous == RESET)
            {
                /* Interrupt caused by End of auto-baud */
                if (intsrc & UART_AUTOBAUD_INTSTAT_ABEO){
                    // Disable AB interrupt
                    UART_IntConfig(_LPC_UART, UART_INTCFG_ABEO, DISABLE);
                    // Set Sync flag
                    Synchronous = SET;
                }

                /* Auto-Baudrate Time-Out interrupt (not implemented) */
                if (intsrc & UART_AUTOBAUD_INTSTAT_ABTO) {
                    /* Just clear this bit - Add your code here */
                    UART_ABClearIntPending(_LPC_UART, UART_AUTOBAUD_INTSTAT_ABTO);
                }
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
    UART_Send(_LPC_UART, menu1, sizeof(menu1), BLOCKING);
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main UART program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    // UART Configuration structure variable
    UART_CFG_Type UARTConfigStruct;
    // UART FIFO configuration Struct variable
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    // Auto baudrate configuration structure
    UART_AB_CFG_Type ABConfig;

    uint32_t idx, len;
    __IO FlagStatus exitflag;
    uint8_t buffer[10];

#if (UART_TEST_NUM == 0)
    /*
     * Initialize UART0 pin connect
     * P0.2: U0_TXD
     * P0.3: U0_RXD
     */
    PINSEL_ConfigPin(0,2,1);
    PINSEL_ConfigPin(0,3,1);
#elif (UART_TEST_NUM == 1)
    /*
     * Initialize UART1 pin connect
     * P2.0: U1_TXD
     * P2.1: U1_RXD
     */
    PINSEL_ConfigPin(2,0,2);
    PINSEL_ConfigPin(2,1,2);
#elif (UART_TEST_NUM == 2)
#if (_CURR_USING_OEM_BRD == LPC4088_OEM_BOARD)
    /*
     * Initialize UART2 pin connect
     * P4.22: U2_TXD
     * P4.23: U2_RXD
     */
    PINSEL_ConfigPin(4,22,2);
    PINSEL_ConfigPin(4,23,2);
#else
    /*
     * Initialize UART2 pin connect
     * P0.10: U2_TXD
     * P0.11: U2_RXD
     */
    PINSEL_ConfigPin(0,10,1);
    PINSEL_ConfigPin(0,11,1);
#endif
#elif (UART_TEST_NUM == 3)
    /*
     * Initialize UART2 pin connect
     * P0.2: U3_TXD
     * P0.3: U3_RXD
     */
    PINSEL_ConfigPin(0,2,2);
    PINSEL_ConfigPin(0,3,2);
#elif (UART_TEST_NUM == 4)
    /*
     * Initialize UART2 pin connect
     * P0.22: U4_TXD
     * P2.9: U4_RXD
     */
    PINSEL_ConfigPin(0,22,3);
    PINSEL_ConfigPin(2,9,3);
#endif
    /* Initialize UART Configuration parameter structure to default state:
     * Baudrate = 115200bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */
    UART_ConfigStructInit(&UARTConfigStruct);

    /* Initialize UART0 peripheral with given to corresponding parameter
     * in this case, don't care the baudrate value UART initialized
     * since this will be determine when running auto baudrate
     */
    UART_Init(_LPC_UART, &UARTConfigStruct);

    /* Initialize FIFOConfigStruct to default state:
     *              - FIFO_DMAMode = DISABLE
     *              - FIFO_Level = UART_FIFO_TRGLEV0
     *              - FIFO_ResetRxBuf = ENABLE
     *              - FIFO_ResetTxBuf = ENABLE
     *              - FIFO_State = ENABLE
     */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig(_LPC_UART, &UARTFIFOConfigStruct);


    // Enable UART Transmit
    UART_TxCmd(_LPC_UART, ENABLE);


    /* Enable UART End of Auto baudrate interrupt */
    UART_IntConfig(_LPC_UART, UART_INTCFG_ABEO, ENABLE);
    /* Enable UART Auto baudrate timeout interrupt */
    UART_IntConfig(_LPC_UART, UART_INTCFG_ABTO, ENABLE);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(_UART_IRQ, ((0x01<<3)|0x01));
    /* Enable Interrupt for UART0 channel */
    NVIC_EnableIRQ(_UART_IRQ);


/* ---------------------- Auto baud rate section ----------------------- */
    // Reset Synchronous flag for auto-baudrate mode
    Synchronous = RESET;

    // Configure Auto baud rate mode
    ABConfig.ABMode = UART_AUTOBAUD_MODE0;
    ABConfig.AutoRestart = ENABLE;

    // Start auto baudrate mode
    UART_ABCmd(_LPC_UART, &ABConfig, ENABLE);
    print_menu();

    /* Loop until auto baudrate mode complete */
    while (Synchronous == RESET);


    // Print status of auto baudrate
    UART_Send(_LPC_UART, syncmenu, sizeof(syncmenu), BLOCKING);
/* ---------------------- End of Auto baud rate section ----------------------- */

    // print welcome screen
    print_menu();

    // reset exit flag
    exitflag = RESET;

    /* Read some data from the buffer */
    while (exitflag == RESET)
    {
       len = 0;
        while (len == 0)
        {
            len = UART_Receive(_LPC_UART, buffer, sizeof(buffer), NONE_BLOCKING);
        }

        /* Got some data */
        idx = 0;
        while (idx < len)
        {
            if (buffer[idx] == 27)
            {
                /* ESC key, set exit flag */
                UART_Send(_LPC_UART, menu2, sizeof(menu2), BLOCKING);
                exitflag = SET;
            }
            else if (buffer[idx] == 'r')
            {
                /* Echo it back */
                UART_Send(_LPC_UART, &buffer[idx], 1, BLOCKING);

                print_menu();
            }
            else
            {
                /* Echo it back */
                UART_Send(_LPC_UART, &buffer[idx], 1, BLOCKING);
            }
            idx++;
        }
    }

    // wait for current transmission complete - THR must be empty
    while (UART_CheckBusy(_LPC_UART) == SET);

    // DeInitialize UART0 peripheral
    UART_DeInit(_LPC_UART);

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
