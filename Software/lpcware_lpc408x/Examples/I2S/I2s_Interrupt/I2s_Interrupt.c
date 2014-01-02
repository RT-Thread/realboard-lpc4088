/**********************************************************************
* $Id$      I2s_Interrupt.c 2011-06-02
*//**
* @file     I2s_Interrupt.c
* @brief    This example describes how to use I2S transfer in interrupt
*           mode
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
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"

/** @defgroup I2S_Interrupt I2S Interrupt
 * @ingroup I2S_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
/** Max buffer length */
#define BUFFER_SIZE             0x400
/** I2S Buffer Source Address is AHBRAM1_BASE that used for USB RAM purpose, but
 * it is not used in this example, so this memory section can be used for general purpose
 * memory
 */
#define I2S_BUFFER_SRC          LPC_PERI_RAM_BASE //0x20000000
/** I2S Buffer Destination Address is (AHBRAM1_BASE + 0x100UL) that used for USB RAM purpose, but
 * it is not used in this example, so this memory section can be used for general purpose
 * memory
 */
#define I2S_BUFFER_DST          (I2S_BUFFER_SRC+0x1000UL) //0x20005000

#define RXFIFO_EMPTY        0
#define TXFIFO_FULL         8


/************************** PRIVATE VARIABLES ***********************/
uint8_t menu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" I2S INTERRUPT example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" Use two I2S channels in the same board to transfer data in interrupt mode\n\r"
"********************************************************************************\n\r";

volatile uint8_t  I2STXDone = 0;
volatile uint8_t  I2SRXDone = 0;

volatile uint32_t *I2STXBuffer = (uint32_t*)(I2S_BUFFER_SRC);
volatile uint32_t *I2SRXBuffer = (uint32_t *)(I2S_BUFFER_DST);

volatile uint32_t I2SReadLength = 0;
volatile uint32_t I2SWriteLength = 0;

uint8_t tx_depth_irq = 0;
uint8_t rx_depth_irq = 0;
uint8_t dummy=0;


/************************** PRIVATE FUNCTIONS *************************/
void I2S_IRQHandler(void);

void Buffer_Init(void);
Bool Buffer_Verify(void);
void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       I2S IRQ Handler
 * @param[in]   None
 * @return      None
 **********************************************************************/
void I2S_IRQHandler()
{
    uint32_t RXLevel = 0;

    //Check RX interrupt
    if(I2S_GetIRQStatus(LPC_I2S, I2S_RX_MODE))
    {
        RXLevel = I2S_GetLevel(LPC_I2S, I2S_RX_MODE);
        if ( (RXLevel != RXFIFO_EMPTY) && !I2SRXDone )
        {
            while ( RXLevel > 0 )
            {
                if ( I2SReadLength == BUFFER_SIZE )
                {
                    //Stop RX
                    I2S_Stop(LPC_I2S, I2S_RX_MODE);
                    // Disable RX
                    I2S_IRQCmd(LPC_I2S, I2S_RX_MODE, DISABLE);
                    I2SRXDone = 1;
                    break;
                }
                else
                {
                    I2SRXBuffer[I2SReadLength++] = LPC_I2S->RXFIFO;
                }
                RXLevel--;
            }
        }
    }
    return;
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Initialize buffer
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Buffer_Init(void) {
    uint32_t i;
    for ( i = 0; i < BUFFER_SIZE; i++ ) /* clear buffer */
    {
    I2STXBuffer[i] = i;
    I2SRXBuffer[i] = 0;
    }

}

/*********************************************************************//**
 * @brief       Verify buffer
 * @param[in]   none
 * @return      TRUE - if two buffers are similar
 *              FALSE - if two buffers are different
 **********************************************************************/
Bool Buffer_Verify(void) {
    uint32_t i;
    uint32_t *pTX = (uint32_t *) &I2STXBuffer[0];
    uint32_t *pRX = (uint32_t *) &I2SRXBuffer[1];
    /* Because first element of RX is dummy 0 value, so we just verify
     * from 2nd element
     */
    for (i = 1; i < BUFFER_SIZE; i++) {
        if (*pTX++ != *pRX++)  {
            return FALSE;
        }
    }
    return TRUE;
}
/*********************************************************************//**
 * @brief       print menu
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
void c_entry (void) {                       /* Main Program */
    I2S_MODEConf_Type I2S_ClkConfig;
    I2S_CFG_Type I2S_ConfigStruct;

    uint32_t i;
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
/* Initialize I2S peripheral ------------------------------------*/
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

//  PINSEL_ConfigPin(1,16,2);
    Buffer_Init();

    I2S_Init(LPC_I2S);

    /* setup:
     *      - wordwidth: 16 bits
     *      - stereo mode
     *      - master mode for I2S_TX and slave for I2S_RX
     *      - ws_halfperiod is 31
     *      - not use mute mode
     *      - use reset and stop mode
     *      - select the fractional rate divider clock output as the source,
     *      - disable 4-pin mode
     *      - MCLK ouput is disable
     *      - Frequency = 44.1 kHz
     * Because we use mode I2STXMODE[3:0]= 0000, I2SDAO[5]=0 and
     * I2SRX[3:0]=0000, I2SDAI[5] = 1. So we have I2SRX_CLK = I2STX_CLK
     * --> I2SRXBITRATE = 1 (not divide TXCLK to produce RXCLK)
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
//  I2S_ClkConfig.mcena = I2S_MCLK_DISABLE;
    I2S_ClkConfig.mcena = I2S_MCLK_ENABLE;
    I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_TX_MODE);
    I2S_ClkConfig.mcena = I2S_MCLK_DISABLE;
    I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_RX_MODE);

    I2S_FreqConfig(LPC_I2S, 44100, I2S_TX_MODE);
    I2S_SetBitRate(LPC_I2S, 0, I2S_RX_MODE);

    I2S_Stop(LPC_I2S, I2S_TX_MODE);
    I2S_Stop(LPC_I2S, I2S_RX_MODE);

    NVIC_EnableIRQ(I2S_IRQn);

    /* RX FIFO depth is 1, TX FIFO depth is 8. */
    I2S_IRQConfig(LPC_I2S,I2S_TX_MODE,8);
    I2S_IRQConfig(LPC_I2S,I2S_RX_MODE,1);
    I2S_IRQCmd(LPC_I2S,I2S_RX_MODE,ENABLE);
    I2S_Start(LPC_I2S);

/* I2S transmit ---------------------------------------------------*/
    while ( I2SWriteLength < BUFFER_SIZE )
    {
        while(I2S_GetLevel(LPC_I2S, I2S_TX_MODE)==TXFIFO_FULL);
        I2S_Send(LPC_I2S, I2STXBuffer[I2SWriteLength++]);
    }

    I2STXDone = 1;

    /* Wait for transmit/receive complete */
    while ( !I2SRXDone || !I2STXDone );
    for(i=0;i<BUFFER_SIZE;i++)
    {
        _DBH32(I2SRXBuffer[i]);_DBG_("");
    }
    /* Verify RX and TX Buffer */
    if(Buffer_Verify())
    {
        _DBG_("Verify Buffer: OK...");
    }
    else
    {
        _DBG_("Verify Buffer: ERROR...");
    }


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
