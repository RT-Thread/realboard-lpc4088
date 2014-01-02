/**********************************************************************
* $Id$      pca9532.c           2011-06-02
*//**
* @file     pca9532.c
* @brief    Contains all functions support for PCA9532 IC  to drive 16 LEDs
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
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _I2C
#include "pca9532.h"
#include "lpc_i2c.h"
#include "lpc_pinsel.h"

#define BUFSIZE         (20)

//PCA9532 link to I2C0 only
#define PCA9532_I2C     I2C_0


/*********************************************************************//**
 * @brief       Initialize Pca9532
 * @param[in]   i2cClockFreq    I2C clock frequency that Pca9532 operate
 * @return      None
 **********************************************************************/
void Pca9532_Init(uint32_t i2cClockFreq)
{
    // Config Pin for I2C_SDA and I2C_SCL of I2C0
    // It's because the PCA9532 IC is linked to LPC407x_8x_177x_8x by I2C0 clearly
    PINSEL_ConfigPin (0, 27, 1);
    PINSEL_ConfigPin (0, 28, 1);

    I2C_Init(PCA9532_I2C, i2cClockFreq);

    /* Enable I2C1 operation */
    I2C_Cmd(PCA9532_I2C, I2C_MASTER_MODE, ENABLE);

    return;
}

/*********************************************************************//**
 * @brief       Close Pca9532
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Pca9532_DeInit(void)
{
    /* Enable I2C1 operation */
    I2C_Cmd(PCA9532_I2C, I2C_MASTER_MODE, DISABLE);

    return;
}

/*********************************************************************//**
 * @brief       Control led output
 * @param[in]   settings    Pointer that point to PCA configuration struct
 * @return      Status, could be:
 *              - PCA9532_RETFUNC_OK: success
 *              - PCA9532_RETFUNC_FAILED_OP: fail
 **********************************************************************/
int Pca9532_LedOutputControl(pca9532_Configure_st_t* settings)
{
    uint32_t psc0 = 0, psc1 = 0, pwm0 = 0, pwm1 = 0, ls0 = 0, ls1 = 0, ls2 = 0, ls3 = 0;
    int cnt = 0;

    I2C_M_SETUP_Type i2cData;
    uint8_t i2cBuf[BUFSIZE];

    // Check if the LED level is in range. Unless set it to default
    for (cnt = 0; cnt < NUMBER_OF_ONCHIP_LEDS; cnt++)
    {
        settings->led_settings[cnt] = ((settings->led_settings[cnt] >= PCA9532_LED_MAX_LEVEL)) ?
                                            PCA9532_LED_LEVEL_DEFAULT : settings->led_settings[cnt];
    }

    if (settings->led_blinking_freq_0 == 0)
    {
        psc0 = 0;
    }
    else
    {
        switch(settings->led_freq0_unit)
        {
            default:

            case PCA9532_CALCULATING_TIME_IN_SECOND:
                psc0 = (settings->led_blinking_freq_0 * PCA9532_PRESCALER_FACTOR) - 1;
                break;

            case PCA9532_CALCULATING_TIME_IN_HERTZ:
                psc0 = (uint32_t) ((PCA9532_PRESCALER_FACTOR / settings->led_blinking_freq_0) - 1);
                break;
        }
    }

    pwm0 = settings->duty_cycle_0 * PCA9532_PWM_FACTOR / PERCENT_FACTOR;

    if (settings->led_blinking_freq_0 == 0)
    {
        psc1 = 0;
    }
    else
    {
        switch(settings->led_freq1_unit)
        {
            default:

            case PCA9532_CALCULATING_TIME_IN_SECOND:
                psc1 = (settings->led_blinking_freq_1 * PCA9532_PRESCALER_FACTOR) - 1;
                break;

            case PCA9532_CALCULATING_TIME_IN_HERTZ:
                psc1 = (uint32_t) ((PCA9532_PRESCALER_FACTOR / settings->led_blinking_freq_1) - 1);
                break;
        }
    }

    pwm1 = settings->duty_cycle_1 * PCA9532_PWM_FACTOR / PERCENT_FACTOR;

    ls0 = 0;
    ls1 = 0;
    ls2 = 0;
    ls3 = 0;

    for(cnt = 0; cnt < 4; cnt++)
    {
        ls0 |= (settings->led_settings[cnt] & PCA9532_LED_LEVEL_MASK) << (cnt * PCA9532_LED_LEVEL_NUM_BITS);

        ls1 |= (settings->led_settings[cnt + 4] & PCA9532_LED_LEVEL_MASK) << (cnt * PCA9532_LED_LEVEL_NUM_BITS);

        ls2 |= (settings->led_settings[cnt + 8] & PCA9532_LED_LEVEL_MASK) << (cnt * PCA9532_LED_LEVEL_NUM_BITS);

        ls3 |= (settings->led_settings[cnt + 12] & PCA9532_LED_LEVEL_MASK) << (cnt * PCA9532_LED_LEVEL_NUM_BITS);
    }

    i2cBuf[0] = 0x12;                       /* Control Register - start with Frequency Prescale 0 */
    i2cBuf[1] = psc0 & BYTE_BITMASK;        /* Value of Frequency Prescale 0 */
    i2cBuf[2] = pwm0 & BYTE_BITMASK;        /* Value of PWM Register 0 */
    i2cBuf[3] = psc1 & BYTE_BITMASK;        /* Value of Frequency Prescale 1 */
    i2cBuf[4] = pwm1 & BYTE_BITMASK;        /* Value of PWM Register 1 */
    i2cBuf[5] = ls0 & BYTE_BITMASK;         /* Value of LED Selector 0 */
    i2cBuf[6] = ls1 & BYTE_BITMASK;         /* Value of LED Selector 1 */
    i2cBuf[7] = ls2 & BYTE_BITMASK;         /* Value of LED Selector 2 */
    i2cBuf[8] = ls3 & BYTE_BITMASK;         /* Value of LED Selector 3 */


    i2cData.sl_addr7bit = PCA9532_I2CADDR >> 1;
    i2cData.tx_data = i2cBuf;
    i2cData.tx_length = 9;
    i2cData.rx_data = NULL;
    i2cData.rx_length = 0;
    i2cData.retransmissions_max = 3;

    if (I2C_MasterTransferData(PCA9532_I2C, &i2cData, I2C_TRANSFER_POLLING) == SUCCESS)
    {
        return PCA9532_RETFUNC_OK;
    }
    else
    {
        return PCA9532_RETFUNC_FAILED_OP;
    }

}
#endif /*_I2C*/
/*********************************************************************************
**                            End Of File
*********************************************************************************/
