/**********************************************************************
* $Id$      uda1380.c           2011-10-26
*//**
* @file     uda1380.c
* @brief    This module contains functions to control UDA1380.
* @version  1.0
* @date     26. October. 2011
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
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _I2C

#include "uda1380.h"
#include "lpc_i2c.h"
#include "lpc_pinsel.h"

//Uda1380 link to I2C0 only
#define UDA1380_I2C     I2C_0


/*********************************************************************//**
 * @brief       Initialize Uda1380
 * @param[in]   i2cClockFreq    I2C clock frequency that Uda1380 operate
 * @param[in]   i2sClockFreq    I2S bit clock frequency
 * @return      UDA1380_FUNC_OK/UDA1380_FUNC_ERR
 **********************************************************************/
int32_t Uda1380_Init(uint32_t i2cClockFreq, uint32_t i2sClockFreq)
{
    int32_t ret;
    uint8_t clk;
    
    // Config Pin for I2C_SDA and I2C_SCL of I2C0
    // It's because the Uda1380 IC is linked to LPC407x_8x_177x_8x by I2C0 clearly
    PINSEL_ConfigPin (0, 27, 1);
    PINSEL_ConfigPin (0, 28, 1);

    I2C_Init(UDA1380_I2C, i2cClockFreq);

    /* Enable I2C1 operation */
    I2C_Cmd(UDA1380_I2C, I2C_MASTER_MODE, ENABLE);

    /* Reset */
    ret = Uda1380_WriteData(UDA1380_REG_L3, 0 );
    if(ret != UDA1380_FUNC_OK)
        return ret;

    /* Write clock settings */
    ret = Uda1380_WriteData(UDA1380_REG_I2S,0 );
    if(ret != UDA1380_FUNC_OK)
        return ret;

#if UDA1380_SYSCLK_USED //Use SYSCLK
    ret = Uda1380_WriteData(UDA1380_REG_EVALCLK, 
           EVALCLK_DEC_EN | EVALCLK_DAC_EN | EVALCLK_INT_EN | EVALCLK_DAC_SEL_SYSCLK );
    if(ret != UDA1380_FUNC_OK)
        return ret;
    
    ret = Uda1380_WriteData(UDA1380_REG_PWRCTRL,
                      PWR_PON_HP_EN | PWR_PON_DAC_EN | PWR_PON_BIAS_EN);
    if(ret != UDA1380_FUNC_OK)
        return ret;
    
#else //Use WSPLL   
    if(i2sClockFreq >= 6250 && i2sClockFreq < 12500)
        clk = EVALCLK_WSPLL_SEL6_12K;
    else if(i2sClockFreq >= 12501 && i2sClockFreq < 25000)  
        clk = EVALCLK_WSPLL_SEL12_25K;
    else if(i2sClockFreq >= 25001 && i2sClockFreq < 50000)
        clk = EVALCLK_WSPLL_SEL25_50K;
    else if(i2sClockFreq >= 50001 && i2sClockFreq < 100000)
        clk = EVALCLK_WSPLL_SEL50_100K;
    else
        clk= 0;
        
    ret = Uda1380_WriteData(UDA1380_REG_EVALCLK, 
                     EVALCLK_DEC_EN | EVALCLK_DAC_EN | EVALCLK_INT_EN | EVALCLK_DAC_SEL_WSPLL | clk);
    if(ret != UDA1380_FUNC_OK)
        return ret;
    
    ret = Uda1380_WriteData(UDA1380_REG_PWRCTRL,
                      PWR_PON_PLL_EN | PWR_PON_HP_EN | PWR_PON_DAC_EN | PWR_PON_BIAS_EN);
    if(ret != UDA1380_FUNC_OK)
        return ret;
#endif

    ret = Uda1380_Mute(FALSE);
    if(ret != UDA1380_FUNC_OK)
        return ret;

    return UDA1380_FUNC_OK;
}

/*********************************************************************//**
 * @brief       Write data to a register of Uda1380
 * @param[in]   reg   Register address
 * @param[out]  data  data value.
 * @return      UDA1380_FUNC_OK/UDA1380_FUNC_ERR
 **********************************************************************/
int32_t Uda1380_WriteData(uint8_t reg, uint16_t data)
{
    I2C_M_SETUP_Type i2cData;
    uint8_t i2cBuf[UDA1380_CMD_BUFF_SIZE];

    i2cBuf[0] = reg;
    i2cBuf[1] = (data >> 8) & 0xFF;
    i2cBuf[2] = data & 0xFF;
    
    i2cData.sl_addr7bit = UDA1380_SLAVE_ADDR;
    i2cData.tx_length = UDA1380_CMD_BUFF_SIZE;
    i2cData.tx_data = i2cBuf;
    i2cData.rx_data = NULL;
    i2cData.rx_length = 0;
    i2cData.retransmissions_max = 3;    
    
    if (I2C_MasterTransferData(UDA1380_I2C, &i2cData, I2C_TRANSFER_POLLING) == SUCCESS)
    {
        uint16_t dataTmp; 
        if(Uda1380_ReadData(reg, &dataTmp) != UDA1380_FUNC_OK) {
            return UDA1380_FUNC_ERR;
        }
        if(dataTmp != data)
            return UDA1380_FUNC_ERR;
        
        return UDA1380_FUNC_OK;
    }

    return UDA1380_FUNC_ERR;
}


/*********************************************************************//**
 * @brief       Read data stored in register of Uda1380
 * @param[in]   reg   Register address
 * @param[out]  data  point to the buffer which is used for storing data.
 * @return      UDA1380_FUNC_OK/UDA1380_FUNC_ERR
 **********************************************************************/
int32_t Uda1380_ReadData(uint8_t reg, uint16_t *data)
{
    I2C_M_SETUP_Type i2cData;
    uint8_t i2cBuf[UDA1380_CMD_BUFF_SIZE];

    if(data == NULL)
        return UDA1380_FUNC_ERR;

    i2cBuf[0] = reg;
    
    i2cData.sl_addr7bit = UDA1380_SLAVE_ADDR;
    i2cData.tx_length = 1;
    i2cData.tx_data = i2cBuf;
    i2cData.rx_data = &i2cBuf[1];
    i2cData.rx_length = UDA1380_CMD_BUFF_SIZE - 1;
    i2cData.retransmissions_max = 3;    
    
    if (I2C_MasterTransferData(UDA1380_I2C, &i2cData, I2C_TRANSFER_POLLING) == SUCCESS)
    {
        *data = i2cBuf[1] << 8 | i2cBuf[2];
        return UDA1380_FUNC_OK;
    }

    return UDA1380_FUNC_ERR;
}
/*********************************************************************//**
 * @brief       Mute On/Off
 * @param[in]   MuteOn  TRUE: mute signal
 * @return      UDA1380_FUNC_OK/UDA1380_FUNC_ERR
 **********************************************************************/
int32_t Uda1380_Mute(Bool MuteOn)
{
    uint16_t tmp;
    int32_t ret;
    /* backup old value setting of clock */
    ret = Uda1380_ReadData(UDA1380_REG_EVALCLK, &tmp);
    if(ret != UDA1380_FUNC_OK)
        return ret;
    /* Use sysclk */
    ret = Uda1380_WriteData(UDA1380_REG_EVALCLK, tmp & (~EVALCLK_DAC_SEL_WSPLL));
    if(ret != UDA1380_FUNC_OK)
        return ret;

    if(!MuteOn)
    {
       ret = Uda1380_WriteData(UDA1380_REG_MSTRMUTE,0x0202);
    }
    else
    {
       ret = Uda1380_WriteData(UDA1380_REG_MSTRMUTE,0x4808);
    }
    if(ret != UDA1380_FUNC_OK)
        return ret;
    
     /* Use sysclk */
    ret = Uda1380_WriteData(UDA1380_REG_EVALCLK, tmp);
    if(ret != UDA1380_FUNC_OK)
        return ret;

    return UDA1380_FUNC_OK;
}
#endif /*_I2C*/

