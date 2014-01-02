/**********************************************************************
* $Id$      lcd_qvga_tft_revPB1.c           2012-04-25
*//**
* @file     lcd_qvga_tft_revPB1.c
* @brief    Contains all functions to control LCD controller using SPI
* @version  1.0
* @date     25. April. 2012
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
#include "bsp.h"
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */

#include "lpc_pinsel.h"
#include "lpc_timer.h"
#include "lpc_gpio.h"
#include "lpc_i2c.h"
#include "lpc_lcd.h"

#if (_CUR_USING_LCD == _RUNNING_LCD_EA_REV_PB1)
#if defined(_I2C)&&defined (_GPIO) && defined(_TIM)
/** @defgroup LCD_QVGA_rev_PB1  QVGA TFT LCD rev.PB1
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */

#define I2CDEV  (I2C_0)
#define I2C_EEPROM_ADDR     (0x56)
#define I2C_PCA9532_ADDR    (0x64)

//
// PCA9532 Register addresses
//
#define INPUT0  0
#define INPUT1  1
#define PSC0    2
#define PWM0    3
#define PSC1    4
#define PWM1    5
#define LS0     6
#define LS1     7
#define LS2     8
#define LS3     9

#define LSn(LedNum)     (LS0+(LedNum/4))
#define BITn(LedNum)    ((LedNum%4)*2)

#define LCD_3V3_PIN_NUM         0
#define LCD_5V_PIN_NUM          1
#define LCD_DSP_EN_PIN_NUM      4
#define LCD_BL_CONSTRAST2_PIN_NUM       7
#define LCD_BL_CONSTRAST_PIN_NUM        8

#define LCD_OUT_HI_IMPEDANCE    (0)
#define LCD_OUT_LED             (1)
#define LCD_OUT_PWM0            (2)
#define LCD_OUT_PWM1            (3)

#define PCA9532_PRESCALER_FACTOR        (152)
#define PCA9532_PWM_FACTOR              (256)
#define PERCENT_FACTOR                  (100)

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/*********************************************************************//**
 * @brief       Initialize I2C port
 * @param[in]   i2cClockFreq    I2C clock frequency that Pca9532 operate
 * @return      None
 **********************************************************************/
static void init_i2c(uint32_t i2cClockFreq)
{
    // Config Pin for I2C_SDA and I2C_SCL of I2C0
    // It's because the PCA9532 IC is linked to LPC177x_8x by I2C0 clearly
    PINSEL_ConfigPin (0, 27, 1);
    PINSEL_ConfigPin (0, 28, 1);

    I2C_Init(I2CDEV, i2cClockFreq);

    /* Enable I2C1 operation */
    I2C_Cmd(I2CDEV, I2C_MASTER_MODE, ENABLE);

    return;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/
void SetPWM(uint8_t brightness)
{
    /* Transmit setup */
    I2C_M_SETUP_Type txsetup;
    uint8_t i2c_buf[2];
    
    txsetup.sl_addr7bit = I2C_PCA9532_ADDR;
    txsetup.tx_data = i2c_buf;
    txsetup.tx_length = 2;
    txsetup.rx_data = NULL;
    txsetup.rx_length = 0;
    txsetup.retransmissions_max = 3;

    i2c_buf[0] = PSC0;  // frequency setting
    i2c_buf[1] = 0;       // max
    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &txsetup, I2C_TRANSFER_POLLING) != SUCCESS){
        return;
    }
    
    i2c_buf[0] = PWM0;  // duty-cycle
    i2c_buf[1] = (100-brightness)*PCA9532_PWM_FACTOR / PERCENT_FACTOR;      // brightness setting
    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &txsetup, I2C_TRANSFER_POLLING) != SUCCESS){
        return;
    }
    
    i2c_buf[0] = LSn(LCD_BL_CONSTRAST_PIN_NUM); //source of the BL pin 
    i2c_buf[1] = LCD_OUT_PWM0<<(BITn(LCD_BL_CONSTRAST_PIN_NUM));   // use PWM0 output
    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &txsetup, I2C_TRANSFER_POLLING) != SUCCESS){
        return;
    }
}
/*********************************************************************//**
 * @brief       Initialize LCD Controller.
 * @param[in]   None
 * @return      None
 **********************************************************************/

void InitLcdController (void)
{
    TIM_TIMERCFG_Type TIM_ConfigStruct;
    uint8_t i2c_buf[2];
    /* Transmit setup */
    I2C_M_SETUP_Type txsetup;
  
    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue  = 1;
      
    // Set configuration for Tim_config and Tim_MatchConfig
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
  
    init_i2c(100000);

    txsetup.sl_addr7bit = I2C_PCA9532_ADDR;
    txsetup.tx_data = i2c_buf;
    txsetup.tx_length = 2;
    txsetup.rx_data = NULL;
    txsetup.rx_length = 0;
    txsetup.retransmissions_max = 3;
    
    // "v1,cc0,c31,d50,o,d200,c51,cc100";
    //   1st letter     2nd letter        Meaning
    //      c           c                 Send command to update PWM
    //                  d                 Send command to update the status for Display Enable Pin
    //                  3                 Send command to update the status for 3V3 pin
    //                  5                 Send command to update the status for 5V pin
    //      d           Number            Delay in a number of miliseconds
    //      o                             open the LCD
    //      v                             Sequence version info

    // PWM setting
    SetPWM(0);

    // 3V3 pin 
    i2c_buf[0] = LSn(LCD_3V3_PIN_NUM);
    txsetup.tx_length = 1;
    txsetup.rx_data = &i2c_buf[1];
    txsetup.rx_length = 1;
    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &txsetup, I2C_TRANSFER_POLLING) != SUCCESS){
        return;
    } 
    i2c_buf[1]&= ~0x03<<(BITn(LCD_3V3_PIN_NUM));
    i2c_buf[1]|= LCD_OUT_LED<<(BITn(LCD_3V3_PIN_NUM));
    txsetup.tx_length = 2;
    txsetup.rx_data = NULL;
    txsetup.rx_length = 0;
    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &txsetup, I2C_TRANSFER_POLLING) != SUCCESS){
        return;
    } 
    TIM_Waitms(250);

    // 5V pin
    i2c_buf[0] =  LSn(LCD_5V_PIN_NUM);  
    txsetup.tx_length = 1;
    txsetup.rx_data = &i2c_buf[1];
    txsetup.rx_length = 1;
    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &txsetup, I2C_TRANSFER_POLLING) != SUCCESS){
        return;
    } 
    i2c_buf[1]&= ~(0x03 << (BITn(LCD_5V_PIN_NUM)));
    i2c_buf[1]|= LCD_OUT_LED <<(BITn(LCD_5V_PIN_NUM));
    txsetup.tx_length = 2;
    txsetup.rx_data = NULL;
    txsetup.rx_length = 0;
    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &txsetup, I2C_TRANSFER_POLLING) != SUCCESS){
        return;
    }

    // Set PWM
    SetPWM(100);
    
    TIM_DeInit(LPC_TIM0);

}

/**
 * @}
 */
#endif
#endif

