/**********************************************************************
* $Id$      Dac_SineWave.c  2011-06-02
*//**
* @file     Dac_SineWave.c
* @brief    This example describes how to use DAC to generate a sine wave
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
#include "LPC407x_8x_177x_8x.h"
#include "lpc_dac.h"
#include "lpc_gpdma.h"
#include "lpc_clkpwr.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup DAC_SineWave  DAC SineWave
 * @ingroup DAC_Examples
 * @{
 */

#define _ONE_POSITIVE_HALF      (0)
#define _DMA_USING              (1)


/************************** PRIVATE MACROS *************************/
#define SINEWAVE_AMPLITUDE          512

//Thi minimum value is as amplitude to make all the signed not to be less than 0 (zero)
#define SINEWAVE_OFFSET             SINEWAVE_AMPLITUDE

#if _ONE_POSITIVE_HALF
#define NUM_SINE_SAMPLE 30
#define SINE_FREQ_IN_HZ 100
#else
#define NUM_SINE_SAMPLE 60
#define SINE_FREQ_IN_HZ 60
#endif

/** DMA size of transfer */
#define DMA_SIZE        NUM_SINE_SAMPLE

/************************** PRIVATE VARIABLES *************************/
GPDMA_Channel_CFG_Type GPDMACfg;

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main DAC program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    DAC_CONVERTER_CFG_Type DAC_ConverterConfigStruct;
    GPDMA_LLI_Type DMA_LLI_Struct;

    uint32_t dac_sine_lut[NUM_SINE_SAMPLE];
    uint32_t cnt;

    uint32_t sin_0_to_90_16_samples[16]={\
                                        0,      1045,   2079,   3090,
                                        4067,   5000,   5877,   6691,
                                        7431,   8090,   8660,   9135,
                                        9510,   9781,   9945,   10000\
                                        };

    // Clear all value
    for(cnt = 0; cnt < NUM_SINE_SAMPLE; cnt++)
    {
        dac_sine_lut[cnt] = 0;
    }

    //Prepare DAC sine look up table
    for(cnt = 0; cnt < NUM_SINE_SAMPLE; cnt++)
    {
        if(cnt <= 15)
        {
            dac_sine_lut[cnt] = SINEWAVE_OFFSET + (SINEWAVE_AMPLITUDE * sin_0_to_90_16_samples[cnt]) / 10000;
        }
        else if(cnt <= 30)
        {
            dac_sine_lut[cnt] =  SINEWAVE_OFFSET + (SINEWAVE_AMPLITUDE * sin_0_to_90_16_samples[30-cnt]) / 10000;
        }
        else if(cnt <= 45)
        {
            dac_sine_lut[cnt] = SINEWAVE_OFFSET - (SINEWAVE_AMPLITUDE * sin_0_to_90_16_samples[cnt-30]) / 10000;
        }
        else
        {
            dac_sine_lut[cnt] = SINEWAVE_OFFSET - (SINEWAVE_AMPLITUDE * sin_0_to_90_16_samples[60-cnt]) / 10000;
        }

        //To make sure the output value is not over 10-bit width of DAC
        if(dac_sine_lut[cnt] > 0x3FF)
            dac_sine_lut[cnt] = 0x3FF;

#if _DMA_USING
        //Shift the value before DMA to DAC component in case of using DMA
        dac_sine_lut[cnt] = (dac_sine_lut[cnt] << 6);
#endif
    }

#if _DMA_USING
    //Prepare DMA link list item structure
    DMA_LLI_Struct.SrcAddr= (uint32_t)dac_sine_lut;

    DMA_LLI_Struct.DstAddr= (uint32_t)&(LPC_DAC->CR);

    DMA_LLI_Struct.NextLLI= (uint32_t)&DMA_LLI_Struct;

    DMA_LLI_Struct.Control= DMA_SIZE
                                | (2<<18) //source width 32 bit
                                | (2<<21) //dest. width 32 bit
                                | (1<<26); //source increment

    /* GPDMA block section -------------------------------------------- */
    /* Initialize GPDMA controller */
    GPDMA_Init();

    // Setup GPDMA channel --------------------------------
    // channel 0
    GPDMACfg.ChannelNum = 0;

    // Source memory
    GPDMACfg.SrcMemAddr = (uint32_t)(dac_sine_lut);

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
    GPDMACfg.DMALLI = (uint32_t)&DMA_LLI_Struct;

    // Setup channel with given parameter
    GPDMA_Setup(&GPDMACfg);
#endif

    DAC_ConverterConfigStruct.CNT_ENA = SET;
    DAC_ConverterConfigStruct.DMA_ENA = RESET;

    DAC_Init(0);

    cnt = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);

    // set time out for DAC
    // clk = sine_freq * number_of_samples_per_sine_cycle * sample_freq     (pulses of clock)
    // => sample_freq = clk / (sine_freq * number_of_samples_per_sine_cycle)

    cnt = cnt/(SINE_FREQ_IN_HZ * NUM_SINE_SAMPLE * 5);

    DAC_SetDMATimeOut(0, cnt);

#if _DMA_USING
    DAC_ConverterConfigStruct.CNT_ENA = SET;
    DAC_ConverterConfigStruct.DMA_ENA = SET;
#endif
    DAC_ConfigDAConverterControl(0, &DAC_ConverterConfigStruct);

#if _DMA_USING
    // Enable GPDMA channel 0
    GPDMA_ChannelCmd(0, ENABLE);
#else
    cnt = 0;

    while(1)
    {
        DAC_UpdateValue(0, dac_sine_lut[cnt]);

        while(!DAC_IsIntRequested(0));

        cnt ++;

        if(cnt == NUM_SINE_SAMPLE)
            cnt = 0;
    }
#endif

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
