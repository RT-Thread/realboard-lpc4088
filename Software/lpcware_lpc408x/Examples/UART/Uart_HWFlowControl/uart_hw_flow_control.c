/**********************************************************************
* $Id$      uart_hw_flow_control.c              2012-04-17
*//**
* @file     uart_hw_flow_control.c
* @brief    This example describes how to using UART Hardware flow
*           control mode
* @version  1.0
* @date     17. April. 2012
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
#include "lpc_uart.h"
#include "lpc_libcfg.h"
#include "lpc_pinsel.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup UART_HWFlowControl    HWFlowControl
 * @ingroup UART_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
#define TEST_UART   UART_1

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" UART hardware flow control mode example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" By this example, UART is configured to hardware flow control mode.\n\r"
"  + Please press any key to be echoed\n\r"
"  + Press 'r' to re-show the welcome string\n\r"
"  + Press ESC to terminate\n\r"
"********************************************************************************\n\r";
uint8_t menu2[] = "UART demo terminated!";

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
    UART_Send(TEST_UART, menu1, sizeof(menu1), BLOCKING);
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main UART program body
 * @param[in]   None
 * @return      int
 **********************************************************************/
int c_entry(void)
{
    // UART Configuration structure variable
    UART_CFG_Type UARTConfigStruct;
    // UART FIFO configuration Struct variable
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;

    uint32_t idx, len;
    __IO FlagStatus exitflag;
    uint8_t buffer[10];

    /*
     * Initialize UART1 pin connect
     */
    PINSEL_ConfigPin(2, 0, 2);//UART1 - TXD
    PINSEL_ConfigPin(2, 1, 2);//UART1 - RXD
    PINSEL_ConfigPin(2, 2, 2);//UART1 - CTS
    PINSEL_ConfigPin(0, 22, 1);//UART1 - RTS

    /* Initialize UART Configuration parameter structure to default state:
     * Baudrate = 115200bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */
    UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize UART0 peripheral with given to corresponding parameter
    UART_Init(TEST_UART, &UARTConfigStruct);

    /* Initialize FIFOConfigStruct to default state:
     *              - FIFO_DMAMode = DISABLE
     *              - FIFO_Level = UART_FIFO_TRGLEV0
     *              - FIFO_ResetRxBuf = ENABLE
     *              - FIFO_ResetTxBuf = ENABLE
     *              - FIFO_State = ENABLE
     */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig(TEST_UART, &UARTFIFOConfigStruct);

    // Configure UART1 hardware flow control RTS/CTS
    UART_FullModemForcePinState(TEST_UART,UART1_MODEM_PIN_RTS,ACTIVE);

    // Enable UART Transmit
    UART_TxCmd(TEST_UART, ENABLE);

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
            len = UART_Receive(TEST_UART, buffer, sizeof(buffer), NONE_BLOCKING);
        }

        /* Got some data */
        idx = 0;
        while (idx < len)
        {
            if (buffer[idx] == 27)
            {
                /* ESC key, set exit flag */
                UART_Send(TEST_UART, menu2, sizeof(menu2), BLOCKING);
                exitflag = SET;
            }
            else if (buffer[idx] == 'r')
            {
                print_menu();
            }
            else
            {
                /* Echo it back */
                UART_Send(TEST_UART, &buffer[idx], 1, BLOCKING);
            }
            idx++;
        }
    }

    // wait for current transmission complete - THR must be empty
    while (UART_CheckBusy(TEST_UART) == SET);

    // DeInitialize UART0 peripheral
    UART_DeInit(TEST_UART);

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
    return c_entry();
}


/*
 * @}
 */
