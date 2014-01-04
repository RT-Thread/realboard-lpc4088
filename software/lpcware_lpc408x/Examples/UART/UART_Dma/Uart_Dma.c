/**********************************************************************
* $Id$      Uart_Dma.c          2011-06-02
*//**
* @file     Uart_Dma.c
* @brief    This example describes how to using UART in DMA mode
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
#include "LPC407x_8x_177x_8x.h"
#include "lpc_uart.h"
#include "lpc_gpdma.h"
#include "lpc_pinsel.h"


/** @defgroup UART_Dma      UART DMA
 * @ingroup UART_Examples
 * @{
 */


/************************** PRIVATE DEFINITIONS *************************/
#define UART_TEST_NUM           1
#if (UART_TEST_NUM == 0)
#define _LPC_UART               UART_0
#define _GPDMA_CONN_UART_Tx     GPDMA_CONN_UART0_Tx
#define _GPDMA_CONN_UART_Rx     GPDMA_CONN_UART0_Rx
#elif (UART_TEST_NUM == 1)
#define _LPC_UART               UART_1
#define _GPDMA_CONN_UART_Tx     GPDMA_CONN_UART1_Tx
#define _GPDMA_CONN_UART_Rx     GPDMA_CONN_UART1_Rx
#elif (UART_TEST_NUM == 2)
#define _LPC_UART               UART_2
#define _GPDMA_CONN_UART_Tx     GPDMA_CONN_UART2_Tx
#define _GPDMA_CONN_UART_Rx     GPDMA_CONN_UART2_Rx
#elif (UART_TEST_NUM == 3)
#define _LPC_UART               UART_3
#define _GPDMA_CONN_UART_Tx     GPDMA_CONN_UART3_Tx
#define _GPDMA_CONN_UART_Rx     GPDMA_CONN_UART3_Rx
#elif (UART_TEST_NUM == 4)
#define _LPC_UART               UART_4
#define _GPDMA_CONN_UART_Tx     GPDMA_CONN_UART4_Tx
#define _GPDMA_CONN_UART_Rx     GPDMA_CONN_UART4_Rx
#endif
/* Receive buffer size */
#define RX_BUF_SIZE 0x10

/************************** PRIVATE VARIABLES *************************/

uint8_t menu1[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" UART DMA example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" The data will transfer into DMA memory, then transmit through Tx line of UART.\n\r"
" To use UART with DMA mode, FIFO function must be enabled\n\r"
" To terminate, press ESC\n\r"
"********************************************************************************\n\r";
uint8_t terminate[] = "\n\r Demo termination!!! \n\r" ;

// Receive buffer
__IO uint8_t rx_buf[RX_BUF_SIZE];

// Terminal Counter flag for Channel 0
__IO uint32_t Channel0_TC;

// Error Counter flag for Channel 0
__IO uint32_t Channel0_Err;

// Terminal Counter flag for Channel 1
__IO uint32_t Channel1_TC;

// Error Counter flag for Channel 1
__IO uint32_t Channel1_Err;


/************************** PRIVATE FUNCTIONS *************************/
void DMA_IRQHandler (void);


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       GPDMA interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void DMA_IRQHandler (void)
{
    uint32_t tmp;
        // Scan interrupt pending
    for (tmp = 0; tmp <= 7; tmp++) {
        if (GPDMA_IntGetStatus(GPDMA_STAT_INT, tmp)){
            // Check counter terminal status
            if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, tmp)){
                // Clear terminate counter Interrupt pending
                GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, tmp);

                switch (tmp){
                    case 0:
                        Channel0_TC++;
                        GPDMA_ChannelCmd(0, DISABLE);
                        break;
                    case 1:
                        Channel1_TC++;
                        GPDMA_ChannelCmd(1, DISABLE);
                        break;
                    default:
                        break;
                }

            }
                // Check error terminal status
            if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, tmp)){
                // Clear error counter Interrupt pending
                GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, tmp);
                switch (tmp){
                    case 0:
                        Channel0_Err++;
                        GPDMA_ChannelCmd(0, DISABLE);
                        break;
                    case 1:
                        Channel1_Err++;
                        GPDMA_ChannelCmd(1, DISABLE);
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main UART program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    uint8_t *rx_char;
    volatile uint32_t idx;
    uint8_t  stop = 0;
    // UART Configuration structure variable
    UART_CFG_Type UARTConfigStruct;
    // UART FIFO configuration Struct variable
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;
    GPDMA_Channel_CFG_Type GPDMACfg;

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
     * Baudrate = 115200 bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */
    UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize UART0 peripheral with given to corresponding parameter
    UART_Init(_LPC_UART, &UARTConfigStruct);


    /* Initialize FIFOConfigStruct to default state:
     *              - FIFO_DMAMode = DISABLE
     *              - FIFO_Level = UART_FIFO_TRGLEV0
     *              - FIFO_ResetRxBuf = ENABLE
     *              - FIFO_ResetTxBuf = ENABLE
     *              - FIFO_State = ENABLE
     */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Enable DMA mode in UART
    UARTFIFOConfigStruct.FIFO_DMAMode = ENABLE;

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig(_LPC_UART, &UARTFIFOConfigStruct);

    // Enable UART Transmit
    UART_TxCmd(_LPC_UART, ENABLE);


    /* GPDMA Interrupt configuration section ------------------------------------------------- */

    /* Initialize GPDMA controller */
    GPDMA_Init();

    /* Setting GPDMA interrupt */
    // Disable interrupt for DMA
    NVIC_DisableIRQ (DMA_IRQn);
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(DMA_IRQn, ((0x01<<3)|0x01));

    // Setup GPDMA channel --------------------------------
    // channel 0
    GPDMACfg.ChannelNum = 0;
    // Source memory
    GPDMACfg.SrcMemAddr = (uint32_t) &menu1;
    // Destination memory - don't care
    GPDMACfg.DstMemAddr = 0;
    // Transfer size
    GPDMACfg.TransferSize = sizeof(menu1);
    // Transfer width - don't care
    GPDMACfg.TransferWidth = 0;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
    // Source connection - don't care
    GPDMACfg.SrcConn = 0;
    // Destination connection
    GPDMACfg.DstConn = _GPDMA_CONN_UART_Tx;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    // Setup channel with given parameter
    GPDMA_Setup(&GPDMACfg);

    // Setup GPDMA channel --------------------------------
    // channel 1
    GPDMACfg.ChannelNum = 1;
    // Source memory - don't care
    GPDMACfg.SrcMemAddr = 0;
    // Destination memory
    GPDMACfg.DstMemAddr = (uint32_t) &rx_buf;
    // Transfer size
    GPDMACfg.TransferSize = sizeof(rx_buf);
    // Transfer width - don't care
    GPDMACfg.TransferWidth = 0;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_P2M;
    // Source connection
    GPDMACfg.SrcConn = _GPDMA_CONN_UART_Rx;
    // Destination connection - don't care
    GPDMACfg.DstConn = 0;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    GPDMA_Setup(&GPDMACfg);

    /* Reset terminal counter */
    Channel0_TC = 0;
    /* Reset Error counter */
    Channel0_Err = 0;

    // Enable interrupt for DMA
    NVIC_EnableIRQ (DMA_IRQn);

    // Enable GPDMA channel 0
    GPDMA_ChannelCmd(0, ENABLE);
    // Make sure GPDMA channel 1 is disabled
    GPDMA_ChannelCmd(1, DISABLE);

    /* Wait for GPDMA on UART0 Tx processing complete */
    while ((Channel0_TC == 0) && (Channel0_Err == 0));

    // Main loop - echos back to the terminal
    while (1)
    {
        /* Reset terminal counter */
        Channel1_TC = 0;
        /* Reset Error counter */
        Channel1_Err = 0;

        // Setup channel with given parameter
        GPDMA_Setup(&GPDMACfg);

        // Enable GPDMA channel 1
        GPDMA_ChannelCmd(1, ENABLE);

        // Clear Rx buffer using DMA
        for (idx = 0; idx < RX_BUF_SIZE; idx++){
            rx_buf[idx] = 0;
        }

        // now, start receive character using GPDMA
        rx_char = (uint8_t *) &rx_buf;
        while ((Channel1_TC == 0) && (Channel1_Err == 0)){
            // Check whether to terminate
            if(*rx_char == 27)
            {
               stop = 1;
               break;
            }
            // Check whether if there's any character received, then print it back
            if (*rx_char != 0)
            {
                UART_Send(_LPC_UART, rx_char, 1, BLOCKING);
                rx_char++;
            }
        }
        if(stop)
            break;
    }

    // Setup channel with given parameter
    // channel 0
    GPDMACfg.ChannelNum = 0;
    // Source memory
    GPDMACfg.SrcMemAddr = (uint32_t) &terminate;
    // Destination memory - don't care
    GPDMACfg.DstMemAddr = 0;
    // Transfer size
    GPDMACfg.TransferSize = sizeof(terminate);
    // Transfer width - don't care
    GPDMACfg.TransferWidth = 0;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
    // Source connection - don't care
    GPDMACfg.SrcConn = 0;
    // Destination connection
    GPDMACfg.DstConn = _GPDMA_CONN_UART_Tx;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    // Setup channel with given parameter
    GPDMA_Setup(&GPDMACfg);

    
    // Enable GPDMA channel 0
    GPDMA_ChannelCmd(0, ENABLE);
    
    for(idx = 0; idx < 100000; idx++);

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
