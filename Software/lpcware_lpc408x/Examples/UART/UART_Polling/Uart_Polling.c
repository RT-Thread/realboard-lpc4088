/**********************************************************************
* $Id$      Uart_Polling.c          2011-06-02
*//**
* @file     Uart_Polling.c
* @brief    This example describes how to using UART in polling mode
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
#include "bsp.h"
#include "lpc_uart.h"
#include "lpc_pinsel.h"

/** @defgroup UART_Polling  UART Polling
 * @ingroup UART_Examples
 * @{
 */


/************************** PRIVATE DEFINTIONS *************************/
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

uint8_t menu1[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" UART Polling example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" This example used to test UART component.\n\r"
" Receiving by keep polling the RBR register for character stored\n\r"
"  + Please press any key to be echoed\n\r"
"  + Press 'r' to re-show the welcome string\n\r"
"  + Press ESC to terminate\n\r"
"********************************************************************************\n\r";

uint8_t menu2[] = "\n\rUART Polling demo terminated!";

/************************** PRIVATE FUNCTIONS *************************/
void print_menu(void);

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Print Welcome menu
 * @param[in]   none
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    UART_Send((UART_ID_Type)_LPC_UART, menu1, sizeof(menu1), BLOCKING);
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

    uint32_t idx, len;
    FlagStatus exitflag;
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
     * Initialize UART3 pin connect
     * P0.2: U3_TXD
     * P0.3: U3_RXD
     */
    PINSEL_ConfigPin(0,2,2);
    PINSEL_ConfigPin(0,3,2);
#elif (UART_TEST_NUM == 4)
    /*
     * Initialize UART4 pin connect
     * P0.22: U4_TXD
     * P2.9: U4_RXD
     */
    PINSEL_ConfigPin(0,22,3);
    PINSEL_ConfigPin(2,9,3);
#endif

    /* Initialize UART Configuration parameter structure to default state:
     * Baudrate = 115200 bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */

    UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize UART0 peripheral with given to corresponding parameter
    UART_Init((UART_ID_Type)_LPC_UART, &UARTConfigStruct);

    /* Initialize FIFOConfigStruct to default state:
     *              - FIFO_DMAMode = DISABLE
     *              - FIFO_Level = UART_FIFO_TRGLEV0
     *              - FIFO_ResetRxBuf = ENABLE
     *              - FIFO_ResetTxBuf = ENABLE
     *              - FIFO_State = ENABLE
     */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig((UART_ID_Type)_LPC_UART, &UARTFIFOConfigStruct);


    // Enable UART Transmit
    UART_TxCmd((UART_ID_Type)_LPC_UART, ENABLE);

    // print welcome screen
    print_menu();

    // Reset exit flag
    exitflag = RESET;

    /* Read some data from the buffer */
    while (exitflag == RESET)
    {
       len = 0;
        while (len == 0)
        {
            len = UART_Receive((UART_ID_Type)_LPC_UART, buffer, sizeof(buffer), NONE_BLOCKING);
        }

        /* Got some data */
        idx = 0;
        while (idx < len)
        {
            if (buffer[idx] == 27)
            {
                /* ESC key, set exit flag */
                UART_Send((UART_ID_Type)_LPC_UART, menu2, sizeof(menu2), BLOCKING);
                exitflag = SET;
            }
            else if (buffer[idx] == 'r')
            {
                /* Echo it back */
                UART_Send((UART_ID_Type)_LPC_UART, &buffer[idx], 1, BLOCKING);

                print_menu();
            }
            else
            {
                /* Echo it back */
                UART_Send((UART_ID_Type)_LPC_UART, &buffer[idx], 1, BLOCKING);
            }
            idx++;
        }
    }

    // wait for current transmission complete - THR must be empty
    while (UART_CheckBusy((UART_ID_Type)_LPC_UART) == SET);

    // DeInitialize UART0 peripheral
    UART_DeInit((UART_ID_Type)_LPC_UART);

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
