/**********************************************************************
* $Id$      sdram_k4s561632j.h          2011-06-02
*//**
* @file     sdram_k4s561632j.h
* @brief    Contains all macro definitions for SAMSUNG K4S561632J
*           (supported on LPC1788 IAR Olimex Start Kit Rev.B)
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
/** @defgroup  Sdram_K4S561632J Sdram K4S561632J
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */

#ifndef __SDRAM_K4S561632J_H_
#define __SDRAM_K4S561632J_H_
#include "bsp.h"

#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)

#define SDRAM_BASE_ADDR     0xA0000000
#define SDRAM_SIZE          0x10000000

#define SDRAM_REFRESH         7813
#define SDRAM_TRP             20
#define SDRAM_TRAS            45
#define SDRAM_TAPR            1
#define SDRAM_TDAL            3
#define SDRAM_TWR             3
#define SDRAM_TRC             65
#define SDRAM_TRFC            66
#define SDRAM_TXSR            67
#define SDRAM_TRRD            15
#define SDRAM_TMRD            3

extern void SDRAMInit( void );

#endif /* __SDRAM_K4S561632J_H_ */
#endif

/**
 * @}
 */

/*****************************************************************************
**                            End Of File
******************************************************************************/
