/**********************************************************************
* $Id$      I2c_Pca9532Drv.c        2011-06-02
*//**
* @file     Gpio_LedBlinky.c
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
#include "pca9532.h"


/** @defgroup I2C_Pca9532Drv    I2C PCA9532 Driver
 * @ingroup I2C_Examples
 * @{
 */

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]           None
 * @return      None
 **********************************************************************/
void c_entry (void)
{
    uint32_t cnt = 0;

    pca9532_Configure_st_t pca9532Cfg;

    Pca9532_Init(200000);

    // Control the LEDs via this channel of PCA9532_1 IC
    pca9532Cfg.led_blinking_freq_0 = 2;
    pca9532Cfg.led_freq0_unit = PCA9532_CALCULATING_TIME_IN_HERTZ;
    pca9532Cfg.duty_cycle_0 = 50;//percent

    pca9532Cfg.led_blinking_freq_1 = 0;
    pca9532Cfg.led_freq1_unit = PCA9532_CALCULATING_TIME_IN_SECOND;
    pca9532Cfg.duty_cycle_1 = 30;

    for (cnt = 0; cnt < 8; cnt++)
    {
        pca9532Cfg.led_settings[cnt] = PCA9532_LED_LEVEL_DEFAULT;
    }

    pca9532Cfg.led_settings[8]= PCA9532_LED_LEVEL_PWM1;
    pca9532Cfg.led_settings[9]= PCA9532_LED_LEVEL_DEFAULT;
    pca9532Cfg.led_settings[10]= PCA9532_LED_LEVEL_ON;
    pca9532Cfg.led_settings[11]= PCA9532_LED_LEVEL_PWM0;
    pca9532Cfg.led_settings[12]= PCA9532_LED_LEVEL_PWM0;
    pca9532Cfg.led_settings[13]= PCA9532_LED_LEVEL_DEFAULT;
    pca9532Cfg.led_settings[14]= PCA9532_LED_LEVEL_PWM1;
    pca9532Cfg.led_settings[15]= PCA9532_LED_LEVEL_ON;

    Pca9532_LedOutputControl(&pca9532Cfg);

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
