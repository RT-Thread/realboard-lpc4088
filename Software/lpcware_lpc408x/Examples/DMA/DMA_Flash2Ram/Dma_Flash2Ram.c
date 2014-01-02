/**********************************************************************
* $Id$      Dma_Flash2Ram.c 2011-06-02
*//**
* @file     Dma_Flash2Ram.c
* @brief    This example used to test GPDMA function by transferring
*           data from flash to ram memory
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
#include "lpc_gpdma.h"
#include "debug_frmwrk.h"

/** @defgroup DMA_Flash2Ram DMA Flash To RAM
 * @ingroup DMA_Examples
 * @{
 */
/************************** PRIVATE DEFINTIONS*************************/
/** DMA transfer size */
#define DMA_SIZE        16
/** DMA channel */
#define DMA_CHANNEL_NO   0

/************************** PRIVATE VARIABLES *************************/
uint8_t menu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" GPDMA FLASH to RAM example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" This example used to test GPDMA function by transfer data from Flash \n\r"
" to RAM memory\n\r"
"********************************************************************************\n\r";
uint8_t menu2[] = "Demo terminated! \n\r";
uint8_t err_menu[] = "Buffer Check fail!";
uint8_t compl_menu[] = "Buffer Check success!";

//DMAScr_Buffer will be burn into flash when compile
const uint32_t DMASrc_Buffer[DMA_SIZE]=
{
    0x01020304,0x05060708,0x090A0B0C,0x0D0E0F10,
    0x11121314,0x15161718,0x191A1B1C,0x1D1E1F20,
    0x21222324,0x25262728,0x292A2B2C,0x2D2E2F30,
    0x31323334,0x35363738,0x393A3B3C,0x3D3E3F40
};

uint32_t DMADest_Buffer[DMA_SIZE];

// Terminal Counter flag for Channel 0
__IO uint32_t Channel0_TC;

// Error Counter flag for Channel 0
__IO uint32_t Channel0_Err;

/*-------------------------MAIN FUNCTION------------------------------*/
void DMA_IRQHandler (void);

void print_menu(void);
void Error_Loop(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       GPDMA interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void DMA_IRQHandler (void)
{
    // check GPDMA interrupt on channel DMA_CHANNEL_NO
    if (GPDMA_IntGetStatus(GPDMA_STAT_INT, DMA_CHANNEL_NO)){ //check interrupt status on channel DMA_CHANNEL_NO
        // Check counter terminal status
        if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, DMA_CHANNEL_NO)){
            // Clear terminate counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, DMA_CHANNEL_NO);
                Channel0_TC++;
        }
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, DMA_CHANNEL_NO)){
            // Clear error counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, DMA_CHANNEL_NO);
            Channel0_Err++;
        }
    }
}

/*-------------------------PRIVATE FUNCTIONS-----------------------------*/
/*********************************************************************//**
 * @brief       Print Welcome menu
 * @param[in]   none
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG(menu);
}


/*********************************************************************//**
 * @brief       Verify buffer
 * @param[in]   none
 * @return      None
 **********************************************************************/
void Buffer_Verify(void)
{
    uint8_t i;
    uint32_t *src_addr = (uint32_t *)DMASrc_Buffer;
    uint32_t *dest_addr = (uint32_t *)DMADest_Buffer;

    for ( i = 0; i < DMA_SIZE; i++ )
    {
        if ( *src_addr++ != *dest_addr++ )
        {
            /* Call Error Loop */
            Error_Loop();
        }
    }
}


/*********************************************************************//**
 * @brief       Error Loop (called by Buffer_Verify() if any error)
 * @param[in]   none
 * @return      None
 **********************************************************************/
void Error_Loop(void)
{
    _DBG(err_menu);

    /* Loop forever */
    while (1);
}


/*-------------------------MAIN FUNCTION--------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    GPDMA_Channel_CFG_Type GPDMACfg;

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

    /* GPDMA block section -------------------------------------------- */
    /* Disable GPDMA interrupt */
    NVIC_DisableIRQ(DMA_IRQn);
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(DMA_IRQn, ((0x01<<3)|0x01));

    /* Initialize GPDMA controller */
    GPDMA_Init();

    // Setup GPDMA channel --------------------------------
    // channel DMA_CHANNEL_NO
    GPDMACfg.ChannelNum = DMA_CHANNEL_NO;
    // Source memory
    GPDMACfg.SrcMemAddr = (uint32_t)DMASrc_Buffer;
    // Destination memory
    GPDMACfg.DstMemAddr = (uint32_t)DMADest_Buffer;
    // Transfer size
    GPDMACfg.TransferSize = DMA_SIZE;
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

    /* Reset terminal counter */
    Channel0_TC = 0;
    /* Reset Error counter */
    Channel0_Err = 0;

    _DBG("Start transfer on channel "); _DBD(DMA_CHANNEL_NO);_DBG_("");

    // Enable GPDMA channel DMA_CHANNEL_NO
    GPDMA_ChannelCmd(DMA_CHANNEL_NO, ENABLE);

    /* Enable GPDMA interrupt */
    NVIC_EnableIRQ(DMA_IRQn);

    /* Wait for GPDMA processing complete */
    while ((Channel0_TC == 0) && (Channel0_Err == 0));

    /* Verify buffer */
    Buffer_Verify();

    _DBG_(compl_menu);

    _DBG("Demo terminated!");

    /* Loop forever */
    while(1);
}

/* With ARM and GHS toolsets, the entry point is main() - this will
 allow the linker to generate wrapper code to setup stacks, allocate
 heap area, and initialize and copy code and data segments. For GNU
 toolsets, the entry point is through __start() in the crt0_gnu.asm
 file, and that startup code will setup stacks and data */
int main(void) {
    c_entry();
    return 0;
}

/**
 * @}
*/
