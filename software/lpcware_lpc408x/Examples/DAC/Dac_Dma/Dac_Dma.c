/**********************************************************************
* $Id$      Dac_Dma.c   2011-06-02
*//**
* @file     Dac_Dma.c
* @brief    This example describes how to use DAC conversion and
*           using DMA to transfer data
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
#include "lpc_dac.h"
#include "lpc_pinsel.h"
#include "lpc_gpdma.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup DAC_Dma   DAC DMA
 * @ingroup DAC_Examples
 * @{
 */

/************************** PRIVATE MACROS *************************/
/** DMA size of transfer */
#define DMA_SIZE        1

/************************** PRIVATE VARIABLES *************************/
// Terminal Counter flag for Channel 0
__IO uint32_t Channel0_TC;

// Error Counter flag for Channel 0
__IO uint32_t Channel0_Err;

GPDMA_Channel_CFG_Type GPDMACfg;

/************************** PRIVATE FUNCTION *************************/
void DMA_IRQHandler (void);


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
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
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 0);
            Channel0_TC++;
        }

        // Check error terminal status
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0))
        {
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, 0);
            Channel0_Err++;
        }
    }
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main DAC program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    DAC_CONVERTER_CFG_Type DAC_ConverterConfigStruct;
    uint32_t dac_value = 0;
    volatile uint32_t i;

    /* GPDMA block section -------------------------------------------- */

    /* Disable GPDMA interrupt */
    NVIC_DisableIRQ(DMA_IRQn);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(DMA_IRQn, ((0x01 << 3) | 0x01));

    DAC_ConverterConfigStruct.CNT_ENA =SET;
    DAC_ConverterConfigStruct.DMA_ENA = SET;

    DAC_Init(0);

    /* set time out for DAC*/
    DAC_SetDMATimeOut(0, 0xFFFF);

    DAC_ConfigDAConverterControl(0, &DAC_ConverterConfigStruct);

    /* Initialize GPDMA controller */
    GPDMA_Init();

    // Setup GPDMA channel --------------------------------
    // channel 0
    GPDMACfg.ChannelNum = 0;

    // Source memory
    GPDMACfg.SrcMemAddr = (uint32_t)(&dac_value);

    // Destination memory - unused
    GPDMACfg.DstMemAddr = 0;

    // Transfer size
    GPDMACfg.TransferSize = DMA_SIZE;

    // Transfer width - unused
    GPDMACfg.TransferWidth = 0;

    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;

    // Source connection - unused
    GPDMACfg.SrcConn = 0;

    // Destination connection
    GPDMACfg.DstConn = GPDMA_CONN_DAC;

    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;

    // Setup channel with given parameter
    GPDMA_Setup(&GPDMACfg);

    /* Reset terminal counter */
    Channel0_TC = 0;

    /* Reset Error counter */
    Channel0_Err = 0;

    /* Enable GPDMA interrupt */
    NVIC_EnableIRQ(DMA_IRQn);

    /* Wait for GPDMA processing complete */
    while (1)
    {
        // Enable GPDMA channel 0
        GPDMA_ChannelCmd(0, ENABLE);

        while ((Channel0_TC == 0) );

        // Disable GPDMA channel 0
        GPDMA_ChannelCmd(0, DISABLE);

        dac_value ++;

        if (dac_value == 0x3FF)
            dac_value = 0;

        //delay
        for(i=0;i<100000;i++);

        /* Reset terminal counter */
        Channel0_TC = 0;

        // Re-setup channel
        GPDMA_Setup(&GPDMACfg);
    }

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
