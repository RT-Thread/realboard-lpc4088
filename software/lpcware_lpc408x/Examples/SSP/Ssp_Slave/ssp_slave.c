/**********************************************************************
* $Id$      ssp_slave.c             2010-05-21
*//**
* @file     ssp_slave.c
* @brief    This example describes how to use SPP in slave mode
* @version  2.0
* @date     21. May. 2010
* @author   NXP MCU SW Application Team
*
* Copyright(C) 2010, NXP Semiconductor
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
#include "lpc_ssp.h"
#include "lpc_libcfg.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"
#include "lpc_gpio.h"
#include "lpc_gpdma.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup SSP_Slave Slave
 * @ingroup SSP_Examples
 * @{
 */


/************************** PRIVATE DEFINTIONS *************************/
/** Max buffer length */
#define BUFFER_SIZE         0x40
#ifdef CORE_M4
#define _SSP_NO_USING       2
#else
#define _SSP_NO_USING       0
#endif
#define __DMA_USED__        1

#if (_SSP_NO_USING == 0)
#define LPC_SSP         LPC_SSP0
#define SSP_IRQn        SSP0_IRQn
#define SSP_IRQHandler  SSP0_IRQHandler
#if __DMA_USED__
#define SSP_TX_SRC_DMA_CONN     (GPDMA_CONN_SSP0_Tx)
#define SSP_RX_SRC_DMA_CONN     (GPDMA_CONN_SSP0_Rx)
#endif  /*__DMA_USED__*/
#elif  (_SSP_NO_USING == 1)
#define LPC_SSP         LPC_SSP1
#define SSP_IRQn        SSP1_IRQn
#define SSP_IRQHandler  SSP1_IRQHandler
#if __DMA_USED__
#define SSP_TX_SRC_DMA_CONN     (GPDMA_CONN_SSP1_Tx)
#define SSP_RX_SRC_DMA_CONN     (GPDMA_CONN_SSP1_Rx)
#endif  /*__DMA_USED__*/
#else
#define LPC_SSP         LPC_SSP2
#define SSP_IRQn        SSP2_IRQn
#define SSP_IRQHandler  SSP2_IRQHandler
#if __DMA_USED__
#define SSP_TX_SRC_DMA_CONN     (GPDMA_CONN_SSP2_Tx)
#define SSP_RX_SRC_DMA_CONN     (GPDMA_CONN_SSP2_Rx)
#endif  /*__DMA_USED__*/
#endif

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
"Hello NXP Semiconductors \n\r"
"SSP demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - Communicate via: UART0 - 115200bps \n\r"
"\t An example of SSP using interrupt mode to test the SSP driver \n\r"
" This example uses SSP in SPI mode as slave to communicate with an SSP master device \n\r"
" The master and slave transfer together a number of data byte \n\r"
"********************************************************************************\n\r";
uint8_t menu2[] = "Demo terminated! \n\r";

// SSP Configuration structure variable
SSP_CFG_Type SSP_ConfigStruct;
//SSP Data setup structure variable
SSP_DATA_SETUP_Type xferConfig;

// Tx buffer
uint8_t Tx_Buf[BUFFER_SIZE];
// Rx buffer
uint8_t Rx_Buf[BUFFER_SIZE];

/* Status Flag indicates current SSP transmission complete or not */
__IO FlagStatus complete;

#if __DMA_USED__
// Terminal Counter flag for Channel 0
__IO uint32_t Channel0_TC;

// Error Counter flag for Channel 0
__IO uint32_t Channel0_Err;

// Terminal Counter flag for Channel 1
__IO uint32_t Channel1_TC;

// Error Counter flag for Channel 1
__IO uint32_t Channel1_Err;
#endif  /*__DMA_USED__*/


/************************** PRIVATE FUNCTIONS *************************/

void print_menu(void);
void Buffer_Init(void);
void Error_Loop(void);
void Buffer_Verify(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
#if __DMA_USED__
/*********************************************************************//**
 * @brief       GPDMA interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void DMA_IRQHandler (void)
{
    // check GPDMA interrupt on channel 0
    if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 0))
    {
        // Check counter terminal status
        if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0))
        {
            // Clear terminate counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 0);
                Channel0_TC++;
        }

        // Check error terminal status
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0))
        {
            // Clear error counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, 0);
            Channel0_Err++;
        }
    }

    // check GPDMA interrupt on channel 1
    if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 1))
    {
        // Check counter terminal status
        if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1))
        {
            // Clear terminate counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 1);
                Channel1_TC++;
        }

        // Check error terminal status
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 1))
        {
            // Clear error counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, 1);
            Channel1_Err++;
        }
    }
}
#else
/*********************************************************************//**
 * @brief       SSP0 Interrupt used for reading and writing handler
 * @param       None
 * @return      None
 ***********************************************************************/
void SSP_IRQHandler(void)
{
    SSP_DATA_SETUP_Type *xf_setup;
    uint16_t tmp;
    uint8_t dataword;

    // Disable interrupt
    SSP_IntConfig(LPC_SSP, SSP_INTCFG_ROR|SSP_INTCFG_RT|SSP_INTCFG_RX|SSP_INTCFG_TX, DISABLE);

    if(SSP_GetDataSize(LPC_SSP)>SSP_DATABIT_8)
        dataword = 1;
    else
        dataword = 0;
    xf_setup = &xferConfig;
    // save status
    tmp = SSP_GetRawIntStatusReg(LPC_SSP);
    xf_setup->status = tmp;

    // Check overrun error
    if (tmp & SSP_RIS_ROR){
        // Clear interrupt
        SSP_ClearIntPending(LPC_SSP, SSP_INTCLR_ROR);
        // update status
        xf_setup->status |= SSP_STAT_ERROR;
        // Set Complete Flag
        complete = SET;
        return;
    }

    if ((xf_setup->tx_cnt != xf_setup->length) || (xf_setup->rx_cnt != xf_setup->length)){
        /* check if RX FIFO contains data */
        while ((SSP_GetStatus(LPC_SSP, SSP_STAT_RXFIFO_NOTEMPTY)) && (xf_setup->rx_cnt != xf_setup->length)){
            // Read data from SSP data
            tmp = SSP_ReceiveData(LPC_SSP);

            // Store data to destination
            if (xf_setup->rx_data != NULL)
            {
                if (dataword == 0){
                    *(uint8_t *)((uint32_t)xf_setup->rx_data + xf_setup->rx_cnt) = (uint8_t) tmp;
                } else {
                    *(uint16_t *)((uint32_t)xf_setup->rx_data + xf_setup->rx_cnt) = (uint16_t) tmp;
                }
            }
            // Increase counter
            if (dataword == 0){
                xf_setup->rx_cnt++;
            } else {
                xf_setup->rx_cnt += 2;
            }
        }

        while ((SSP_GetStatus(LPC_SSP, SSP_STAT_TXFIFO_NOTFULL)) && (xf_setup->tx_cnt != xf_setup->length)){
            // Write data to buffer
            if(xf_setup->tx_data == NULL){
                if (dataword == 0){
                    SSP_SendData(LPC_SSP, 0xFF);
                    xf_setup->tx_cnt++;
                } else {
                    SSP_SendData(LPC_SSP, 0xFFFF);
                    xf_setup->tx_cnt += 2;
                }
            } else {
                if (dataword == 0){
                    SSP_SendData(LPC_SSP, (*(uint8_t *)((uint32_t)xf_setup->tx_data + xf_setup->tx_cnt)));
                    xf_setup->tx_cnt++;
                } else {
                    SSP_SendData(LPC_SSP, (*(uint16_t *)((uint32_t)xf_setup->tx_data + xf_setup->tx_cnt)));
                    xf_setup->tx_cnt += 2;
                }
            }

            // Check overrun error
            if (SSP_GetRawIntStatus(LPC_SSP, SSP_INTSTAT_RAW_ROR)){
                // update status
                xf_setup->status |= SSP_STAT_ERROR;
                // Set Complete Flag
                complete = SET;
                return;
            }

            // Check for any data available in RX FIFO
            while ((SSP_GetStatus(LPC_SSP, SSP_STAT_RXFIFO_NOTEMPTY)) && (xf_setup->rx_cnt != xf_setup->length)){
                // Read data from SSP data
                tmp = SSP_ReceiveData(LPC_SSP);

                // Store data to destination
                if (xf_setup->rx_data != NULL)
                {
                    if (dataword == 0){
                        *(uint8_t *)((uint32_t)xf_setup->rx_data + xf_setup->rx_cnt) = (uint8_t) tmp;
                    } else {
                        *(uint16_t *)((uint32_t)xf_setup->rx_data + xf_setup->rx_cnt) = (uint16_t) tmp;
                    }
                }
                // Increase counter
                if (dataword == 0){
                    xf_setup->rx_cnt++;
                } else {
                    xf_setup->rx_cnt += 2;
                }
            }
        }
    }

    // If there more data to sent or receive
    if ((xf_setup->rx_cnt != xf_setup->length) || (xf_setup->tx_cnt != xf_setup->length)){
        // Enable all interrupt
        SSP_IntConfig(LPC_SSP, SSP_INTCFG_ROR|SSP_INTCFG_RT|SSP_INTCFG_RX|SSP_INTCFG_TX, ENABLE);
    } else {
        // Save status
        xf_setup->status = SSP_STAT_DONE;
        // Set Complete Flag
        complete = SET;
    }
}
#endif /**__DMA_USED__*/
/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Initialize buffer
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Buffer_Init(void)
{
    uint8_t i;

    for (i = 0; i < BUFFER_SIZE; i++) {
        Tx_Buf[i] = i;
        Rx_Buf[i] = 0;
    }
}

/*********************************************************************//**
 * @brief       Error Loop (called by Buffer_Verify() if any error)
 * @param[in]   none
 * @return      None
 **********************************************************************/
void Error_Loop(void)
{
    /* Loop forever */
    while (1);
}

/*********************************************************************//**
 * @brief       Verify buffer
 * @param[in]   none
 * @return      None
 **********************************************************************/
void Buffer_Verify(void)
{
    uint8_t i;
    uint8_t *src_addr = (uint8_t *) &Tx_Buf[0];
    uint8_t *dest_addr = (uint8_t *) &Rx_Buf[0];

    for ( i = 0; i < BUFFER_SIZE; i++ )
    {
        if ( *src_addr++ != *dest_addr++ )
        {
            _DBG_("Verify error");
            /* Call Error Loop */
            Error_Loop();
        }
    }
}

/*********************************************************************//**
 * @brief       Print Welcome menu
 * @param[in]   none
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG(menu1);
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main SSP program body
 * @param[in]   None
 * @return      int
 **********************************************************************/
int c_entry(void)
{
#if __DMA_USED__
    GPDMA_Channel_CFG_Type GPDMACfg;
#endif
    /*
     * Initialize SSP pin connect
     * P0.15 - SCK;
     * P0.16 - SSEL
     * P0.17 - MISO
     * P0.18 - MOSI
     */
#if (_SSP_NO_USING == 0)
    PINSEL_ConfigPin(0, 15, 2);
    PINSEL_ConfigPin(0, 16, 2);
    PINSEL_ConfigPin(0, 17, 2);
    PINSEL_ConfigPin(0, 18, 2);
#elif (_SSP_NO_USING == 1) 
    PINSEL_ConfigPin(0, 6, 2);

    PINSEL_ConfigPin(0, 7, 2);
    PINSEL_SetFilter(0, 7, 0);

    PINSEL_ConfigPin(0, 8, 2);
    PINSEL_SetFilter(0, 8, 0);

    PINSEL_ConfigPin(0, 9, 2);
    PINSEL_SetFilter(0, 9, 0);
#else
#ifdef CORE_M4
    PINSEL_ConfigPin(5, 2, 2);
    PINSEL_ConfigPin(5, 3, 2);
    PINSEL_ConfigPin(5, 1, 2);
    PINSEL_ConfigPin(5, 0, 2);
#else
    PINSEL_ConfigPin(1, 0, 4);
    PINSEL_ConfigPin(1, 8, 4);
    PINSEL_ConfigPin(1, 1, 4);
    PINSEL_ConfigPin(1, 4, 4);
#endif    
#endif

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

    // initialize SSP configuration structure to default
    SSP_ConfigStructInit(&SSP_ConfigStruct);
    SSP_ConfigStruct.Mode = SSP_SLAVE_MODE;
    // Initialize SSP peripheral with parameter given in structure above
    SSP_Init(LPC_SSP, &SSP_ConfigStruct);
    // Enable SSP peripheral
    SSP_Cmd(LPC_SSP, ENABLE);

    _DBG_("Init buffer");
    /* Initialize Buffer */
    Buffer_Init();

    _DBG_("Wait for master transfer...");

#if __DMA_USED__
    /* Initialize GPDMA controller */
    GPDMA_Init();

    /* Setting GPDMA interrupt */
    // Disable interrupt for DMA
    NVIC_DisableIRQ (DMA_IRQn);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(DMA_IRQn, ((0x01<<3)|0x01));


    /* Configure GPDMA channel 0 -------------------------------------------------------------*/
    /* DMA Channel 0 */
    GPDMACfg.ChannelNum = 0;
    // Source memory
    GPDMACfg.SrcMemAddr = (uint32_t) &Tx_Buf;
    // Destination memory - Not used
    GPDMACfg.DstMemAddr = 0;
    // Transfer size
    GPDMACfg.TransferSize = sizeof(Tx_Buf);
    // Transfer width - not used
    GPDMACfg.TransferWidth = 0;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
    // Source connection - unused
    GPDMACfg.SrcConn = 0;
    // Destination connection
    GPDMACfg.DstConn = SSP_TX_SRC_DMA_CONN;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    // Setup channel with given parameter
    GPDMA_Setup(&GPDMACfg);

    /* Reset terminal counter */
    Channel0_TC = 0;
    /* Reset Error counter */
    Channel0_Err = 0;


    /* Configure GPDMA channel 1 -------------------------------------------------------------*/
    /* DMA Channel 1 */
    GPDMACfg.ChannelNum = 1;
    // Source memory - not used
    GPDMACfg.SrcMemAddr = 0;
    // Destination memory - Not used
    GPDMACfg.DstMemAddr = (uint32_t) &Rx_Buf;
    // Transfer size
    GPDMACfg.TransferSize = sizeof(Rx_Buf);
    // Transfer width - not used
    GPDMACfg.TransferWidth = 0;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_P2M;
    // Source connection
    GPDMACfg.SrcConn = SSP_RX_SRC_DMA_CONN;
    // Destination connection - not used
    GPDMACfg.DstConn = 0;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    // Setup channel with given parameter
    GPDMA_Setup(&GPDMACfg);

    /* Reset terminal counter */
    Channel1_TC = 0;

    /* Reset Error counter */
    Channel1_Err = 0;

    // Enable Tx and Rx DMA on SSP0
    SSP_DMACmd (LPC_SSP, SSP_DMA_RX, ENABLE);
    SSP_DMACmd (LPC_SSP, SSP_DMA_TX, ENABLE);

    // Enable GPDMA channel 0
    GPDMA_ChannelCmd(0, ENABLE);
    // Enable GPDMA channel 0
    GPDMA_ChannelCmd(1, ENABLE);

    // Enable interrupt for DMA
    NVIC_EnableIRQ (DMA_IRQn);

    /* Wait for GPDMA processing complete */
    while (((Channel0_TC == 0) && (Channel0_Err == 0)) \
                || ((Channel1_TC == 0) && (Channel1_Err ==0)));

#else
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(SSP_IRQn, ((0x01<<3)|0x01));
    /* Enable SSP0 interrupt */
    NVIC_EnableIRQ(SSP_IRQn);

    complete = RESET;
    xferConfig.tx_data = Tx_Buf;
    xferConfig.rx_data = Rx_Buf;
    xferConfig.length = BUFFER_SIZE;
    SSP_ReadWrite(LPC_SSP, &xferConfig, SSP_TRANSFER_INTERRUPT);
    //SSP_ReadWrite(LPC_SSP, &xferConfig, SSP_TRANSFER_POLLING);
    while(complete == RESET);
#endif

    // Verify buffer after transferring
    Buffer_Verify();
    _DBG_("Verify complete!");

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


#ifdef  DEBUG
/*******************************************************************************
* @brief        Reports the name of the source file and the source line number
*               where the CHECK_PARAM error has occurred.
* @param[in]    file Pointer to the source file name
* @param[in]    line assert_param error line source number
* @return       None
*******************************************************************************/
void check_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while(1);
}
#endif

/*
 * @}
 */
