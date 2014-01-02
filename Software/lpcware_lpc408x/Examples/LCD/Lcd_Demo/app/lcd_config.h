/**********************************************************************
* $Id$		lcd_config.h			2011-05-25
*//**
* @file		lcd_config.h
* @brief	Include LCD Configuration.
* @version	1.0
* @date		25. April. 2012
* @author	NXP MCU SW Application Team
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
#ifndef __LCD_CONFIG_H__
#define _LCD_CONFIG_H_
#include "bsp.h"
#include "tsc2046.h"
#include "sensor_smb380.h"
#include "sensor_mma7455.h"
#include "logo.h"
#include "Cursor.h"

#if (_CUR_USING_LCD == _RUNNING_LCD_GFT035A320240Y)
#define   LOGO_DISPLAYED                (1)
#define   TCS_USED						(0)
#define ACCEL_SENSOR_USED               (1)

/* LCD Config */
#define LCD_H_SIZE           320
#define LCD_H_PULSE          30
#define LCD_H_FRONT_PORCH    20
#define LCD_H_BACK_PORCH     38
#define LCD_V_SIZE           240
#define LCD_V_PULSE          3
#define LCD_V_FRONT_PORCH    5
#define LCD_V_BACK_PORCH     15
#define LCD_PIX_CLK          (6.5*1000000l)

/* PWM */
#define _PWM_NO_USED    1
#define _PWM_CHANNEL_NO 2
#define _PWM_PORT_NUM   2
#define _PWM_PIN_NUM    1
#define _PWM_PIN_FUNC_NUM 1

#elif (_CUR_USING_LCD ==_RUNNING_LCD_EA_REV_PB1)
#define   LOGO_DISPLAYED                (0)
#define   TCS_USED						(1)
#define   ACCEL_SENSOR_USED             (0)
#if TCS_USED
#define   PAINT_ON_SCREEN                (1)
#endif

/* LCD Config */
#define LCD_H_SIZE           800
#define LCD_H_PULSE          2
#define LCD_H_FRONT_PORCH    17
#define LCD_H_BACK_PORCH     45
#define LCD_V_SIZE           480
#define LCD_V_PULSE          2
#define LCD_V_FRONT_PORCH    22
#define LCD_V_BACK_PORCH     22
#define LCD_PIX_CLK          (36*1000000l)

/* PWM */
#define _PWM_NO_USED    1
#define _PWM_CHANNEL_NO 1
#define _PWM_PORT_NUM   1
#define _PWM_PIN_NUM    18
#define _PWM_PIN_FUNC_NUM 2

/* Touch Screen Config */
#if (TSC2046_CONVERSION_BITS == 8)
#define TOUCH_AD_LEFT    240
#define TOUCH_AD_RIGHT   10
#define TOUCH_AD_TOP     16
#define TOUCH_AD_BOTTOM  240
#else
#define TOUCH_AD_LEFT    3964
#define TOUCH_AD_RIGHT   102
#define TOUCH_AD_TOP     184
#define TOUCH_AD_BOTTOM  3842
#endif
#else  /*(_CUR_USING_LCD == _RUNNING_LCD_QVGA_TFT)*/

#define   LOGO_DISPLAYED                (0)
#define   TCS_USED						(1)
#define   ACCEL_SENSOR_USED             (0)
#if TCS_USED
#define   PAINT_ON_SCREEN                (0)
#endif

/* LCD Config */
#define LCD_H_SIZE           240
#define LCD_H_PULSE          2
#define LCD_H_FRONT_PORCH    10
#define LCD_H_BACK_PORCH     28
#define LCD_V_SIZE           320
#define LCD_V_PULSE          2
#define LCD_V_FRONT_PORCH    1
#define LCD_V_BACK_PORCH     2
#define LCD_PIX_CLK          (8200000l)

/* PWM */
#define _PWM_NO_USED    1
#define _PWM_CHANNEL_NO 1
#define _PWM_PORT_NUM   1
#define _PWM_PIN_NUM    18
#define _PWM_PIN_FUNC_NUM 2

/* Touch Screen Config */
#if (TSC2046_CONVERSION_BITS == 8)
#define TOUCH_AD_LEFT    240
#define TOUCH_AD_RIGHT   10
#define TOUCH_AD_TOP     240
#define TOUCH_AD_BOTTOM  16
#else
#define TOUCH_AD_LEFT    3686
#define TOUCH_AD_RIGHT   205
#define TOUCH_AD_TOP     3842
#define TOUCH_AD_BOTTOM  267
#endif

#endif  /*(_CUR_USING_LCD == _RUNNING_LCD_GFT035A320240Y)*/

#define LCD_VRAM_BASE_ADDR_UPPER 	((uint32_t)SDRAM_BASE_ADDR + 0x00100000)
#define LCD_VRAM_BASE_ADDR_LOWER 	(LCD_VRAM_BASE_ADDR_UPPER + 1024*768*4)
#define LCD_CURSOR_BASE_ADDR 	    ((uint32_t)0x20088800)

#if (LOGO_BPP == 24)
#define MAKE_COLOR(red,green,blue)  (blue<<16|green<<8|red)
#elif (LOGO_BPP == 16)
#define MAKE_COLOR(red,green,blue)  (blue << 10 | green << 5 | red)
#elif (LOGO_BPP == 8)
#define MAKE_COLOR(red,green,blue)  (blue << 5 | green << 3 | red)
#else
#define MAKE_COLOR(red,green,blue)  (red)
#endif

#define _BACK_LIGHT_BASE_CLK (1000/2)

#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
#define  AccSensor_Data_t  SMB380_Data_t
#define  AccSensor_Init    SMB380_Init
#define  AccSensor_GetData SMB380_GetData
#else
#define  AccSensor_Data_t  MMA7455_Data_t
#define  AccSensor_Init    MMA7455_Init
#define  AccSensor_GetData MMA7455_GetData
#endif
#if ((_CUR_USING_LCD == _RUNNING_LCD_QVGA_TFT)||(_CUR_USING_LCD == _RUNNING_LCD_EA_REV_PB1))
extern void InitLcdController (void);
#endif
#if ((_CUR_USING_LCD == _RUNNING_LCD_EA_REV_PB1))
void SetPWM(uint8_t brightness);
#endif
#endif
