/**********************************************************************
* $Id$      tsc2046.h           2012-03-13
*//**
* @file     tsc2046.h
* @brief    Contains all functions to control TSC2046 using SPI
* @version  1.0
* @date     13. March. 2012
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
#ifndef _TSC2046_H_

#define _TSC2046_H_

#include "lpc_types.h"

/** @defgroup  TSC2046  TSC2046
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */


#define TSC2046_CONVERSION_BITS     12

#define TSC2046_SSP_CLOCK       (2000000ul)     

/* TSC2046 control byte definitions */

#define START_BIT               (0x01<<7)

#define CHANNEL_SELECT(cmd)     ((cmd&0x07)<<4)

#define X_MEASURE               (0x05)      // X Channel

#define Y_MEASURE               (0x01)      // Y Channel

#define Z1_MEASURE              (0x03)      // Z1 Channel

#define Z2_MEASURE              (0x04)      // Z2 Channel

#define SER_MODE                (0x01<<2) // Single-Ended Reference Mode
#define DFR_MODE                (0x00<<2) // Differential Reference Mode

#define CONVERT_MODE_8_BITS     (0x01<<3)   
#define CONVERT_MODE_12_BITS    (0x00<<3)

#define PD_ENABLED              (0x00)      // Power-Down Between Conversions.
#define REF_OFF_ADC_ON          (0x01)      // Reference is off and ADC is on.
#define REF_ON_ADC_OFF          (0x02)      // Reference is on and ADC is off.
#define PD_DISABLED             (0x03)      // Device is always powered

typedef struct
{
    int16_t ad_left;                        // left margin
    int16_t ad_right;                       // right margin
    int16_t ad_top;                         // top margin
    int16_t ad_bottom;                      // bottom margin
    int16_t lcd_h_size;                     // lcd horizontal size
    int16_t lcd_v_size;                     // lcd vertical size
    uint8_t swap_xy;                        // 1: swap x-y coords
} TSC2046_Init_Type;

/* Initialize TSC2046 */
void InitTSC2046(TSC2046_Init_Type* pConfig);   
/* Get current Touch Coordinates */
void GetTouchCoord(int16_t *pX, int16_t* pY);
/**
 * @}
 */

#endif /*_TSC2046_H_*/
