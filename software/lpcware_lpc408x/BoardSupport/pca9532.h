/**********************************************************************
* $Id$      pca9532.h           2011-06-02
*//**
* @file     pca9532.h
* @brief    Contains all macro definitions and function prototypes
*           support for external PCA9532 IC to drive 16 LEDs
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



/* Peripheral group ----------------------------------------------------------- */
/** @defgroup  I2C_Led_Dimmer_PCA9532   I2C Led Dimmer PCA9532
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */

#ifndef __PCA9532_H
#define __PCA9532_H


#include "lpc_types.h"
#include "lpc_i2c.h"


#define PCA9532_I2CADDR         (0xC0)

#define BYTE_BITMASK            (0xFF)

#define PCA9532_RETFUNC_OK                  (0)
#define PCA9532_RETFUNC_NOT_INITIALIZED     (-1)
#define PCA9532_RETFUNC_FAILED_OP           (-2)


#define PERCENT_FACTOR          (100)

#define PCA9532_LED_LEVEL_DEFAULT       (PCA9532_LED_LEVEL_OFF)
#define PCA9532_LED_LEVEL_MASK          (0x03)
#define PCA9532_LED_LEVEL_NUM_BITS      (2)
#define PCA9532_PRESCALER_FACTOR        (152)
#define PCA9532_PWM_FACTOR              (256)


#define NUMBER_OF_ONCHIP_LEDS           (16)

typedef enum pca9532_Calculating_Time_Unit_en
{
    PCA9532_CALCULATING_TIME_IN_SECOND = 0,
    PCA9532_CALCULATING_TIME_IN_HERTZ,
    PCA9532_CALCULATING_TIME_MAX_UNITS
} pca9532_Calculating_Time_Unit_en_t;

typedef enum pca9532_Led_Settings_en
{
    PCA9532_LED_LEVEL_OFF = 0,
    PCA9532_LED_LEVEL_ON,
    PCA9532_LED_LEVEL_PWM0,
    PCA9532_LED_LEVEL_PWM1,
    PCA9532_LED_MAX_LEVEL,
    PCA9532_LED_MIN_LEVEL = PCA9532_LED_LEVEL_OFF,
} pca9532_Led_Settings_en_t;


/** \brief  Structure type to configure the operation for the LEDs connecting to PCA9532 chip.
 */

typedef struct pca9532_Configure_st
{
    /**The frequency (in seconds) that the LED will be blinking with, is to calculated for PSC0 */
    uint32_t led_blinking_freq_0;

    /**The unit of frequency of the led_blinking_freq_0 value. It may be second(s) or hert(s)*/
    pca9532_Calculating_Time_Unit_en_t led_freq0_unit;

    /**The frequency (in seconds) that the LED will be blinking with, is to calculated for PSC1 */
    uint32_t led_blinking_freq_1;

    /**The unit of frequency of the led_blinking_freq_1 value. It may be second(s) or hert(s) */
    pca9532_Calculating_Time_Unit_en_t led_freq1_unit;

    /**Set the luminosity of the light for the LEDs if it's ON. This value will be calculated for PWM0 register */
    uint32_t duty_cycle_0;

    /**Set the luminosity of the light for the LEDs if it's ON. This value will be calculated for PWM1 register */
    uint32_t duty_cycle_1;

    /** Control LED0 by 2 bits */
    pca9532_Led_Settings_en_t  led_settings[NUMBER_OF_ONCHIP_LEDS];
} pca9532_Configure_st_t;


void Pca9532_Init(uint32_t i2cClockFreq);
void Pca9532_DeInit(void);
int Pca9532_LedOutputControl(pca9532_Configure_st_t* settings);

#endif//__PCA9532_H

/**
 * @}
 */
