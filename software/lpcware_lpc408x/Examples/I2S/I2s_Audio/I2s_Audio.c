/**********************************************************************
* $Id$      I2s_Audio.c     2011-06-02
*//**
* @file     I2s_Audio.c
* @brief    This example describes how to use I2S to play a short demo
*           audio data on LPC177x_8x/LPC407x_8x.
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
#include <stdio.h>
#include <string.h>
#include "lpc_i2s.h"
#include "lpc_pinsel.h"
#include "uda1380.h"

/** @defgroup I2S_Audio I2S Audio
 * @ingroup I2S_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
/** Max buffer length */
#define DATA_SIZE           434904
#define BUFFER_SIZE         1048 * 4


/************************** PRIVATE VARIABLES ***********************/
extern unsigned char audio[];
uint8_t tx_buffer[BUFFER_SIZE];
uint32_t data_offset, buffer_offset,remain_data;
uint32_t tx_offset = 0;
Bool Tx_EndofBuf_F = FALSE;

/************************** PRIVATE FUNCTIONS *************************/
void I2S_Callback(void);
void I2S_IRQHandler(void);

/*********************************************************************//**
 * @brief       I2S callback function, will be call when I2S send a half of
                buffer complete
 * @param[in]   None
 * @return      None
 **********************************************************************/
void I2S_Callback(void)
{
    if(remain_data >=BUFFER_SIZE/2)
    {
        if(buffer_offset == BUFFER_SIZE)
        {
            // copy audio data into remain half of tx_buffer
            memcpy(tx_buffer + BUFFER_SIZE/2, audio+data_offset, BUFFER_SIZE/2);
            buffer_offset = 0;
        }
        else
            // copy audio data into remain half of tx_buffer
            memcpy(tx_buffer, audio+data_offset, BUFFER_SIZE/2);
            data_offset += BUFFER_SIZE/2;
            remain_data -= BUFFER_SIZE/2;
    }
    else // reach the last copy
    {
        if(buffer_offset == BUFFER_SIZE)
        {
            // copy audio data into remain half of tx_buffer
            memcpy(tx_buffer + BUFFER_SIZE/2, audio+data_offset, remain_data);
            buffer_offset = 0;
        }
        else
            // copy audio data into remain half of tx_buffer
            memcpy(tx_buffer, audio+data_offset, remain_data);

    }
}
/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       I2S IRQ Handler, call to send data to transmit buffer
 * @param[in]   None
 * @return      None
 **********************************************************************/
void I2S_IRQHandler()
{
    uint32_t txlevel,i;
    txlevel = I2S_GetLevel(LPC_I2S,I2S_TX_MODE);
    if(txlevel <= 4)
    {
        for(i=0;i<8-txlevel;i++)
        {
            LPC_I2S->TXFIFO = *(uint32_t *)(tx_buffer + buffer_offset);
            tx_offset +=4;
            buffer_offset +=4;
            //call I2S_Callback() function when reach each half of tx_buffer
            if((buffer_offset == BUFFER_SIZE/2)||(buffer_offset == BUFFER_SIZE))
                I2S_Callback();
            if(tx_offset >= DATA_SIZE)//reach the end of buffer
            {
                NVIC_DisableIRQ(I2S_IRQn);
                I2S_Stop(LPC_I2S, I2S_TX_MODE);
            }
        }
    }
}


/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Initialize transmit buffer
 * @param[in]   none
 * @return      None
 **********************************************************************/
void Buffer_Init(void)
{
    memcpy(tx_buffer, audio, BUFFER_SIZE);
    buffer_offset = 0;
    data_offset = BUFFER_SIZE;
    remain_data = DATA_SIZE - BUFFER_SIZE;
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry (void) {                       /* Main Program */

/*      This code will set I2S in MASTER mode, 44100Hz, Mono,16bit
 *      This example is used to test I2S transmit mode. There is an audio
 *      data in audiodata.c in mono 44100Hz 16 bit, it will be send out to
 *      I2S port. User must plug Ext I2S DAC IC to hear sound.
 */
    I2S_MODEConf_Type I2S_ClkConfig;
    I2S_CFG_Type I2S_ConfigStruct;
    volatile uint32_t i;

    Buffer_Init();

    /* Initialize I2S peripheral ------------------------------------*/
    /* Pin configuration:
     * Assign:  - P0.7 as I2STX_CLK
     *          - P0.8 as I2STX_WS
     *          - P0.9 as I2STX_SDA
     *          - P1.16 as I2SMCLK
     */
    PINSEL_ConfigPin(0,7,1);
    PINSEL_ConfigPin(0,8,1);
    PINSEL_ConfigPin(0,9,1);
    PINSEL_ConfigPin(1,16,2);

    I2S_Init(LPC_I2S);

    /* setup:
     *      - wordwidth: 16 bits
     *      - mono mode
     *      - master mode for I2S_TX
     *      - Frequency = 44.1 kHz
     */

    /* Audio Config*/
    I2S_ConfigStruct.wordwidth = I2S_WORDWIDTH_16;
    I2S_ConfigStruct.mono = I2S_MONO;
    I2S_ConfigStruct.stop = I2S_STOP_ENABLE;
    I2S_ConfigStruct.reset = I2S_RESET_ENABLE;
    I2S_ConfigStruct.ws_sel = I2S_MASTER_MODE;
    I2S_ConfigStruct.mute = I2S_MUTE_DISABLE;
    I2S_Config(LPC_I2S,I2S_TX_MODE,&I2S_ConfigStruct);

    /* Clock Mode Config*/
    I2S_ClkConfig.clksel = I2S_CLKSEL_FRDCLK;
    I2S_ClkConfig.fpin = I2S_4PIN_DISABLE;
    I2S_ClkConfig.mcena = I2S_MCLK_ENABLE;
    I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_TX_MODE);

    I2S_FreqConfig(LPC_I2S, 44100, I2S_TX_MODE);

    I2S_Stop(LPC_I2S, I2S_TX_MODE);

    /* TX FIFO depth is 4 */
    I2S_IRQConfig(LPC_I2S,I2S_TX_MODE,4);
    I2S_IRQCmd(LPC_I2S,I2S_TX_MODE,ENABLE);

    for(i = 0; i <0x1000000; i++);

    Uda1380_Init(200000, 44100);
    
    I2S_Start(LPC_I2S);

    NVIC_EnableIRQ(I2S_IRQn);


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
