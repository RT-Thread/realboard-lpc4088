/**********************************************************************
* $Id$      I2s_Dma.c   2011-06-02
*//**
* @file     I2s_Dma.c
* @brief    This example describes how to use DMA mode to test the I2S
*           driver
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
#include "lpc_i2s.h"
#include "lpc_gpdma.h"
#include "debug_frmwrk.h"
#include "lpc_pinsel.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup I2S_DMA   I2S DMA
 * @ingroup I2S_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
/** Max buffer length */
#define BUFFER_SIZE         0x0A
/** DMA transfer size */
#define DMA_SIZE        0x100UL
/** DMA Source Address is AHBRAM1_BASE that used for USB RAM purpose, but
 * it is not used in this example, so this memory section can be used for general purpose
 * memory
 */
#define DMA_SRC         LPC_PERI_RAM_BASE
/** DMA Source Address is (AHBRAM1_BASE + DMA_SIZE) that used for USB RAM purpose, but
 * it is not used in this example, so this memory section can be used for general purpose
 * memory
 */
#define DMA_DST         (DMA_SRC+DMA_SIZE)

/************************** PRIVATE VARIABLES ***********************/
uint8_t menu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" I2S DMA example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" Use two I2S channels in the same board to transfer data use DMA mode\n\r"
"********************************************************************************\n\r";

// Terminal Counter flag for Channel 0
__IO uint32_t Channel0_TC;

// Error Counter flag for Channel 0
__IO uint32_t Channel0_Err;

// Terminal Counter flag for Channel 1
__IO uint32_t Channel1_TC;

// Error Counter flag for Channel 1
__IO uint32_t Channel1_Err;

volatile uint32_t *I2STXBuffer = (uint32_t*)(DMA_SRC);
volatile uint32_t *I2SRXBuffer = (uint32_t*)(DMA_DST);

/************************** PRIVATE FUNCTIONS *************************/
void DMA_IRQHandler (void);

void Buffer_Init(void);
Bool Buffer_Verify(void);
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
        // Check error terminal status
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0)){
            // Clear error counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, 0);
            Channel0_Err++;
        }
    }
    if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 1)){ //check interrupt status on channel 0
        // Check counter terminal status
        if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1)){
            // Clear terminate counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 1);
                Channel1_TC++;
        }
        // Check error terminal status
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR,1)){
            // Clear error counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, 1);
            Channel1_Err++;
        }
    }
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Initialize buffer
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Buffer_Init(void) {
    uint8_t i;

    for (i = 0; i < BUFFER_SIZE; i++) {
        I2STXBuffer[i] = ((i+1)<<16) + i + 1;
        I2SRXBuffer[i] = 0;
    }
}

/*********************************************************************//**
 * @brief       Verify buffer
 * @param[in]   none
 * @return      None
 **********************************************************************/
Bool Buffer_Verify(void) {
    uint8_t i;
    uint32_t *pTX = (uint32_t *) &I2STXBuffer[0];
    uint32_t *pRX = (uint32_t *) &I2SRXBuffer[1];

    for (i = 0; i < BUFFER_SIZE; i++) {
        if (*pTX++ != *pRX++)  {
            /* Call Error Loop */
            return FALSE;
        }
    }
    return TRUE;
}
/*********************************************************************//**
 * @brief       Print menu screen
 * @param[in]   none
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG_(menu);
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    uint32_t i;
    GPDMA_Channel_CFG_Type GPDMACfg;
    I2S_MODEConf_Type I2S_ClkConfig;
    I2S_CFG_Type I2S_ConfigStruct;
    I2S_DMAConf_Type I2S_DMAStruct;

    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    //print menu screen
    print_menu();

    //Initialize buffer
    Buffer_Init();

    _DBG_("Press '1' to initialize buffer...");
    while(_DG !='1');
    _DBG_("Transmit Buffer init: ...");
    for(i=0;i<BUFFER_SIZE;i++)
    {
        _DBH32(I2STXBuffer[i]);_DBG_("");
    }
    _DBG_("Receive Buffer init: ...");
    for(i=0;i<BUFFER_SIZE;i++)
    {
        _DBH32(I2SRXBuffer[i]);_DBG_("");
    }

    /* Pin configuration:
     * Assign:  - P0.4 as I2SRX_CLK
     *          - P0.5 as I2SRX_WS
     *          - P0.6 as I2SRX_SDA
     *          - P0.7 as I2STX_CLK
     *          - P0.8 as I2STX_WS
     *          - P0.9 as I2STX_SDA
     */
    PINSEL_ConfigPin(0,4,1);
    PINSEL_ConfigPin(0,5,1);
    PINSEL_ConfigPin(0,6,1);
    PINSEL_ConfigPin(0,7,1);
    PINSEL_ConfigPin(0,8,1);
    PINSEL_ConfigPin(0,9,1);

    /* Initialize I2S */
    I2S_Init(LPC_I2S);

    /* setup:
     *      - wordwidth: 16 bits
     *      - stereo mode
     *      - master mode for I2S_TX and slave for I2S_RX
     *      - Frequency = 44.1 kHz
     */

    /* Audio Config*/
    I2S_ConfigStruct.wordwidth = I2S_WORDWIDTH_16;
    I2S_ConfigStruct.mono = I2S_STEREO;
    I2S_ConfigStruct.stop = I2S_STOP_ENABLE;
    I2S_ConfigStruct.reset = I2S_RESET_ENABLE;
    I2S_ConfigStruct.ws_sel = I2S_MASTER_MODE;
    I2S_ConfigStruct.mute = I2S_MUTE_DISABLE;
    I2S_Config(LPC_I2S,I2S_TX_MODE,&I2S_ConfigStruct);

    I2S_ConfigStruct.ws_sel = I2S_SLAVE_MODE;
    I2S_Config(LPC_I2S,I2S_RX_MODE,&I2S_ConfigStruct);

    /* Clock Mode Config*/
    I2S_ClkConfig.clksel = I2S_CLKSEL_FRDCLK;
    I2S_ClkConfig.fpin = I2S_4PIN_DISABLE;
    I2S_ClkConfig.mcena = I2S_MCLK_DISABLE;
    I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_TX_MODE);
    I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_RX_MODE);

    /* Set up frequency and bit rate*/
    I2S_FreqConfig(LPC_I2S, 44100, I2S_TX_MODE);
    I2S_SetBitRate(LPC_I2S, 0, I2S_RX_MODE);
    _DBG_("Press '2' to initialize DMA...");
    while(_DG !='2');
      /* GPDMA Interrupt configuration section ------------------------------------------------- */

     /* Initialize GPDMA controller */
     GPDMA_Init();
     LPC_GPDMA->Config = 0x01;

     /* Setting GPDMA interrupt */
     // Disable interrupt for DMA
     NVIC_DisableIRQ (DMA_IRQn);
     /* preemption = 1, sub-priority = 1 */
     NVIC_SetPriority(DMA_IRQn, ((0x01<<3)|0x01));

    /*
     * Configure GPDMA channel 0 -------------------------------------------------------------
     * Used for I2S Transmit
     */
    // Setup GPDMA channel --------------------------------
    // channel 0
    GPDMACfg.ChannelNum = 0;
    // Source memory
    GPDMACfg.SrcMemAddr = DMA_SRC;
    // Destination memory
    GPDMACfg.DstMemAddr = 0;
    // Transfer size
    GPDMACfg.TransferSize = BUFFER_SIZE;
    // Transfer width - unused
    GPDMACfg.TransferWidth = 0;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
    // Source connection
    GPDMACfg.SrcConn = 0;
    // Destination connection - unused
    GPDMACfg.DstConn = GPDMA_CONN_I2S_Channel_0;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    GPDMA_Setup(&GPDMACfg);
    _DBG_("DMA Channel 0 setting finised...");
    /* Reset terminal counter */
    Channel0_TC = 0;
    /* Reset Error counter */
    Channel0_Err = 0;

    /*
    * Configure GPDMA channel 1 -------------------------------------------------------------
    * Used for UART0 Receive
    */
    // Setup GPDMA channel --------------------------------
    // channel 1
    GPDMACfg.ChannelNum = 1;
    // Source memory - unused
    GPDMACfg.SrcMemAddr = 0;
    // Destination memory
    GPDMACfg.DstMemAddr = DMA_DST;
    // Transfer size
    GPDMACfg.TransferSize = BUFFER_SIZE+1;
    // Transfer width - unused
    GPDMACfg.TransferWidth = 0;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_P2M;
    // Source connection - unused
    GPDMACfg.SrcConn = GPDMA_CONN_I2S_Channel_1;
    // Destination connection
    GPDMACfg.DstConn = 0;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    GPDMA_Setup(&GPDMACfg);
    _DBG_("DMA Channel 1 setting finised...");
    /* Reset terminal counter */
    Channel1_TC = 0;
    /* Reset Error counter */
    Channel1_Err = 0;

    // Enable GPDMA channel 0 & 1
    GPDMA_ChannelCmd(0, ENABLE);
    GPDMA_ChannelCmd(1, ENABLE);

    // Enable interrupt for DMA
    NVIC_EnableIRQ (DMA_IRQn);
    _DBG_("Press '3' to start I2S transfer process...");
    while(_DG !='3');
    _DBG_("I2S Start...");

    I2S_DMAStruct.DMAIndex = I2S_DMA_2;
    I2S_DMAStruct.depth = 8;
    I2S_DMAConfig(LPC_I2S, &I2S_DMAStruct, I2S_RX_MODE);
    I2S_DMAStruct.DMAIndex = I2S_DMA_1;
    I2S_DMAStruct.depth = 1;
    I2S_DMAConfig(LPC_I2S, &I2S_DMAStruct, I2S_TX_MODE);

    I2S_Start(LPC_I2S);

    I2S_DMACmd(LPC_I2S, I2S_DMA_2, I2S_RX_MODE, ENABLE);
    I2S_DMACmd(LPC_I2S, I2S_DMA_1, I2S_TX_MODE, ENABLE);

    while ((Channel0_TC == 0)||(Channel1_TC == 0) );

    _DBG_("I2S Finish...");
    _DBG_("Receive Buffer data: ...");
    for(i=0;i<BUFFER_SIZE+1;i++)
    {
     _DBH32(I2SRXBuffer[i]);
     if(I2SRXBuffer[i]==0)
     {
         _DBG_(" ->Dummy data");
     }
     else _DBG_("");
    }
    _DBG_("Demo is terminated!!!");
    I2S_DeInit(LPC_I2S);
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
