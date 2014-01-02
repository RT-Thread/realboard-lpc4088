/**********************************************************************
* $Id$      Uart_IrdaTransmit.c         2011-06-02
*//**
* @file     Uart_IrdaTransmit.c
* @brief    This example describes how to using UART in IrDA Transmit mode
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

#include "lpc_uart.h"
#include "lpc_pinsel.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup UART_IrDA_Transmit    UART IrDA Transmitting
 * @ingroup UART_IrDA_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
#define UART_PORT 0

#if (UART_PORT == 0)
#define TEST_UART UART_0
#elif (UART_PORT == 1)
#define TEST_UART UART_1
#endif

#define TEST_IRDA UART_4

/************************** PRIVATE VARIABLES *************************/

uint8_t menu1[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" UART IrDA example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" By this example, UART4 is configured to work in IrDA mode.\n\r"
" This application roles as Transmitter will connect to another one as Receiver\n\r"
" through IrDA interface for data transmission\n\r"
"  + Press 'r' to re-show the welcome string\n\r"
"  + Press ESC to terminate\n\r"
"********************************************************************************\n\r";

uint8_t menu2[] = "\n\rUART demo terminated!";

uint8_t menu3[] = "\n\rPlease enter a hex byte value to transmit: 0x";
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
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    // UART Configuration structure variable
    UART_CFG_Type UARTConfigStruct;
    
    // UART FIFO configuration Struct variable
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    
    uint32_t idx,len;
    __IO FlagStatus exitflag;
    uint8_t buffer,temp;

#if (UART_PORT == 0)
    // Initialize UART0 pin connect
    PINSEL_ConfigPin(0, 2, 1);
    PINSEL_ConfigPin(0, 3, 1);
#endif

#if (UART_PORT == 1)
    /*
     * Initialize UART1 pin connect
     * P2.0: U1_TXD
     * P2.1: U1_RXD
     */
    PINSEL_ConfigPin(2,0,2);
    PINSEL_ConfigPin(2,1,2);
#endif

    // Initialize UART4 pin connect
    //P0.22 as UART4_TXD with func 03 for recieving the signal from Infra module
    PINSEL_ConfigPin(0, 22, 3);

    /* Initialize UART Configuration parameter structure to default state:
     * Baudrate = 115200 bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */
    UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize UART0 & UART3 peripheral with given to corresponding parameter
    UART_Init(TEST_UART, &UARTConfigStruct);
    
    UART_Init(TEST_IRDA, &UARTConfigStruct);
    
    /* Initialize FIFOConfigStruct to default state:
     *              - FIFO_DMAMode = DISABLE
     *              - FIFO_Level = UART_FIFO_TRGLEV0
     *              - FIFO_ResetRxBuf = ENABLE
     *              - FIFO_ResetTxBuf = ENABLE
     *              - FIFO_State = ENABLE
     */
    //UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);
    UARTFIFOConfigStruct.FIFO_DMAMode = DISABLE;
    UARTFIFOConfigStruct.FIFO_Level = UART_FIFO_TRGLEV0;
    UARTFIFOConfigStruct.FIFO_ResetRxBuf = ENABLE;
    UARTFIFOConfigStruct.FIFO_ResetTxBuf = ENABLE;

    // Initialize FIFO for UART0 & UART3 peripheral
    UART_FIFOConfig(TEST_UART, &UARTFIFOConfigStruct);
    
    UART_FIFOConfig(TEST_IRDA, &UARTFIFOConfigStruct);

    //Configure and enable IrDA mode on UART
    UART_IrDACmd(TEST_IRDA,ENABLE);
    
    // Enable UART Transmit
    UART_TxCmd(TEST_UART, ENABLE);
    
    UART_TxCmd(TEST_IRDA, ENABLE);
    
    // print welcome screen
    print_menu();

    // Reset exit flag
    exitflag = RESET;
    idx = 0;
    buffer = 0;

    /* Read some data from the buffer */
    while (exitflag == RESET)
    {
        if(idx == 0)
        {
            UART_Send(TEST_UART, menu3, sizeof(menu3), BLOCKING);
        }
        
        len=0;

        while(len == 0)
        {
            len = UART_Receive(TEST_UART, &temp, 1, NONE_BLOCKING);
        }

        //In case of 'ESC' character from the serial terminal
        if(temp == 27)
        {
            UART_Send(TEST_UART, menu2, sizeof(menu2), BLOCKING);
            
            exitflag=SET;
        }
        else if(temp == 'r')
        {
            idx = 0;
            buffer = 0;
            
            print_menu();
            
            UART_Send(TEST_IRDA, &buffer, 1, BLOCKING);
        }
        else
        {
            idx++;
            
            switch(temp)
            {
                case '0': 
                    buffer = (buffer << 4) | 0x00;
                    break;

                case '1': 
                    buffer = (buffer << 4) | 0x01;
                    break;

                case '2': 
                    buffer = (buffer << 4) | 0x02;
                    break;

                case '3': 
                    buffer = (buffer << 4) | 0x03;
                    break;
                    
                case '4': 
                    buffer = (buffer << 4) | 0x04;
                    break;
                    
                case '5': 
                    buffer = (buffer << 4) | 0x05;
                    break;
                    
                case '6': 
                    buffer = (buffer << 4) | 0x06;
                    break;
                    
                case '7': 
                    buffer = (buffer << 4) | 0x07;
                    break;
                    
                case '8': 
                    buffer = (buffer << 4) | 0x08;
                    break;
                    
                case '9': 
                    buffer = (buffer << 4) | 0x09;
                    break;
                    
                case 'a': 
                    
                case 'A': 
                    buffer = (buffer << 4) | 0x0A;
                    break;
                    
                case 'b': 
                    
                case 'B': 
                    buffer = (buffer << 4) | 0x0B;
                    break;
                    
                case 'c': 
                    
                case 'C': 
                    buffer = (buffer << 4) | 0x0C;
                    break;
                    
                case 'd': 
                    
                case 'D': 
                    buffer = (buffer << 4) | 0x0D;
                    break;
                    
                case 'e': 

                case 'E': 
                    buffer = (buffer << 4) | 0x0E;
                    break;
                    
                case 'f':
                    
                case 'F': 
                    buffer = (buffer << 4) | 0x0F;
                    break;
                    
                default: 
                    idx = 0;
                    buffer = 0;
                    break;
            }
            
            if(idx == 2)
            {
                temp = buffer >> 4;
                
                if(temp <= 9)
                    temp = temp + 0x30;
                else 
                    temp = temp + 0x37;
                
                UART_Send(TEST_UART, &temp, 1, BLOCKING);
                
                temp = (buffer & 0x0F);
                
                if(temp <= 9)
                    temp = temp + 0x30;
                else 
                    temp = temp + 0x37;
                
                UART_Send(TEST_UART, &temp, 1, BLOCKING);

                UART_Send(TEST_IRDA, &buffer, 1, BLOCKING);
                
                idx = 0;
                buffer = 0;
            }
        }
    }
    
    // wait for current transmission complete - THR must be empty
    while (UART_CheckBusy(TEST_UART) == SET);
    
    while (UART_CheckBusy(TEST_IRDA) == SET);
    
    // DeInitialize UART0 & UART3 peripheral
    UART_DeInit(TEST_UART);
    
    UART_DeInit(TEST_IRDA);
    
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
