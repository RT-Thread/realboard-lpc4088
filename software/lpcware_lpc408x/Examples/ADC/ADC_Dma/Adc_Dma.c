/**********************************************************************
* $Id$      Adc_Dma.c       2011-06-02
*//**
* @file     Adc_Dma.c
* @brief    This example describes how to use ADC conversions and
*           transfer converted data by using DMA.
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

#include "lpc_adc.h"
#include "lpc_pinsel.h"
#include "lpc_gpdma.h"
#include "debug_frmwrk.h"
#include "bsp.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup ADC_DMA       ADC DMA
 * @ingroup ADC_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
/** DMA size of transfer */
#define DMA_SIZE        1

/************************** PRIVATE VARIABLES *************************/
uint8_t  menu1[] =
"***********************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" ADC DMA example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - Communicate via: UART0\1\2 - 115200bps \n\r"
" DMA testing : ADC peripheral to memory\n\r"
" Use ADC with 12-bit resolution rate of 400KHz\n\r"
" The ADC value is handled by DMA function\n\r"
" The content here is displayed by UART interface\n\r"
" Turn the potentiometer to see how ADC value changes\n\r"
" Press q to stop the demo\n\r"
"***********************************************************************\n\r";

/* Terminal Counter flag for Channel 0 */
__IO uint32_t Channel0_TC;

/* Error Counter flag for Channel 0 */
__IO uint32_t Channel0_Err;

/************************** PRIVATE FUNCTION *************************/
void DMA_IRQHandler (void);

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
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Print menu
 * @param[in]   None
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG(menu1);
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main ADC program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    GPDMA_Channel_CFG_Type GPDMACfg;
    volatile uint32_t adc_value, tmp;
    uint8_t  quit;

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

    /* Initialize ADC ----------------------------------------------------*/

    /* Settings for AD input pin
     */
    PINSEL_ConfigPin (BRD_ADC_PREPARED_CH_PORT, BRD_ADC_PREPARED_CH_PIN, BRD_ADC_PREPARED_CH_FUNC_NO);
    PINSEL_SetAnalogPinMode(BRD_ADC_PREPARED_CH_PORT,BRD_ADC_PREPARED_CH_PIN,ENABLE);

    /*  Configuration for ADC :
     *  ADC conversion rate = 400KHz
     */
    ADC_Init(LPC_ADC, 400000);

    ADC_IntConfig(LPC_ADC, BRD_ADC_PREPARED_INTR, ENABLE);
    ADC_ChannelCmd(LPC_ADC, BRD_ADC_PREPARED_CHANNEL, ENABLE);

    /* GPDMA block section -------------------------------------------- */
    /* Disable GPDMA interrupt */
    NVIC_DisableIRQ(DMA_IRQn);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(DMA_IRQn, ((0x01<<3)|0x01));

    /* Initialize GPDMA controller */
    GPDMA_Init();

    // Setup GPDMA channel --------------------------------
    // channel 0
    GPDMACfg.ChannelNum = 0;
    // Source memory - unused
    GPDMACfg.SrcMemAddr = 0;
    // Destination memory
    GPDMACfg.DstMemAddr = (uint32_t) &adc_value;
    // Transfer size
    GPDMACfg.TransferSize = DMA_SIZE;
    // Transfer width - unused
    GPDMACfg.TransferWidth = 0;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_P2M;
    // Source connection
    GPDMACfg.SrcConn = GPDMA_CONN_ADC;
    // Destination connection - unused
    GPDMACfg.DstConn = 0;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    GPDMA_Setup(&GPDMACfg);

    /* Reset terminal counter */
    Channel0_TC = 0;
    /* Reset Error counter */
    Channel0_Err = 0;

    /* Enable GPDMA interrupt */
    NVIC_EnableIRQ(DMA_IRQn);

    while (1)
    {
        // Enable GPDMA channel 0
        GPDMA_ChannelCmd(0, ENABLE);

        ADC_StartCmd(LPC_ADC, ADC_START_NOW);

        /* Wait for GPDMA processing complete */
        while ((Channel0_TC == 0));

        // Disable GPDMA channel 0
        GPDMA_ChannelCmd(0, DISABLE);

        //Display the result of conversion on the UART
        _DBG("ADC value on channel "); _DBD(BRD_ADC_PREPARED_CHANNEL);
        _DBG(" is: "); _DBD32(ADC_DR_RESULT(adc_value)); _DBG_("");

        // Wait for a while
        for(tmp = 0; tmp < 1000000; tmp++);

        /* GPDMA Re-setup */
        GPDMA_Setup(&GPDMACfg);

        /* Reset terminal counter */
        Channel0_TC = 0;

        /* Reset Error counter */
        Channel0_Err = 0;

        if(_DG_NONBLOCK(&quit) &&
            (quit == 'Q' || quit == 'q'))
            break;
    }
    _DBG_("Demo termination!!!");

    ADC_DeInit(LPC_ADC);
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
