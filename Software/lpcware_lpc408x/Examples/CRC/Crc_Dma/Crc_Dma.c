/**********************************************************************
* $Id$      Adc_Burst.c 2011-06-02
*//**
* @file     Adc_Burst.c
* @brief    This example describes how to use CRC engine with DMA
*               supporting on LPC177x_8x/LPC407x_8x.
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
#include "lpc_crc.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"
#include "lpc_gpdma.h"

/** @defgroup CRC_Dma   CRC DMA
 * @ingroup CRC_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS ***********************/
#define BLOCK_SIZE      0x40

/************************** PRIVATE VARIABLES *************************/
uint8_t menu[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" CRC DMA demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" Use CRC engine to calculate CRC for a block 32-bit data \n\r"
" This example use CRC-32 polynomial and use DMA for transfering data \n\r"
"********************************************************************************\n\r";

uint32_t BlockData[BLOCK_SIZE];
Bool CRC_init_flag = FALSE;
// Terminal Counter flag for Channel 0
__IO uint32_t Channel0_TC;

// Error Counter flag for Channel 0
__IO uint32_t Channel0_Err;

/************************** PRIVATE FUNCTION *************************/
void DMA_IRQHandler (void);
void Init_BlockData (void);
void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       GPDMA interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void DMA_IRQHandler (void)
{
    // check GPDMA interrupt on channel 0
    if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 0)){ //check interrupt status on channel 0
        // Check counter terminal status
        if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0)){
            // Clear terminate counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 0);
                Channel0_TC++;
        }
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0)){
            // Clear error counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, 0);
            Channel0_Err++;
        }
    }
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Initialize block data
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Init_BlockData(void)
{
    uint32_t i;
    for(i=0;i<BLOCK_SIZE;i++)
    {
        BlockData[i] = i;
    }
}

/*********************************************************************//**
 * @brief       Print menu
 * @param[in]   None
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG(menu);
}
/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main ADC program body
 * @param[in]   None
 * @return  None
 **********************************************************************/
void c_entry(void)
{
    uint32_t result, i;
    GPDMA_Channel_CFG_Type GPDMACfg;

    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();
    print_menu();

    Init_BlockData();

    _DBG_("Block data: ");
    for(i=0;i<BLOCK_SIZE;i++)
    {
        _DBH32(BlockData[i]);_DBG("  ");
    }
    //Initialize DMA
    /* GPDMA block section -------------------------------------------- */
    /* Disable GPDMA interrupt */
    _DBG_("\n\rInitialize DMA controller...");
    NVIC_DisableIRQ(DMA_IRQn);
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(DMA_IRQn, ((0x01<<3)|0x01));

    /* Initialize GPDMA controller */
    GPDMA_Init();

    // Setup GPDMA channel --------------------------------
    // channel 0
    GPDMACfg.ChannelNum = 0;
    // Source memory
    GPDMACfg.SrcMemAddr = (uint32_t)&BlockData;
    // Destination memory
    GPDMACfg.DstMemAddr = (uint32_t)&(LPC_CRC->WR_DATA_DWORD);
    // Transfer size
    GPDMACfg.TransferSize = BLOCK_SIZE;
    // Transfer width
    GPDMACfg.TransferWidth = GPDMA_WIDTH_WORD;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2M;
    // Source connection - unused
    GPDMACfg.SrcConn = 0;
    // Destination connection - unused
    GPDMACfg.DstConn = 0;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    // Setup channel with given parameter
    GPDMA_Setup(&GPDMACfg);

    /* Use CRC_32 for demo */
    CRC_Init(CRC_POLY_CRC32);
    _DBG("\n\rCRC-32 Result: ");

    /* Reset terminal counter */
    Channel0_TC = 0;
    /* Reset Error counter */
    Channel0_Err = 0;

    // Enable GPDMA channel 0
    GPDMA_ChannelCmd(0, ENABLE);

    /* Enable GPDMA interrupt */
    NVIC_EnableIRQ(DMA_IRQn);

    while ((Channel0_TC == 0) && (Channel0_Err == 0));
    result =  LPC_CRC->SUM;
    _DBH32(result);_DBG_("");
    _DBG_("Demo terminated!!!");
    while(1);
}

/* Support required entry point for other toolchain */
int main (void)
{
    c_entry();
    return 0;
}


/**
 * @}
 */
