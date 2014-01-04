/**********************************************************************
* $Id$      sdram_is42s32800d.h         2011-08-22
*//**
* @file     sdram_is42s32800d.h
* @brief    Contains all macro definitions and function prototypes
*           support for external SDRAM ISSI IS42S32800D
* @version  1.0
* @date     22. August. 2011
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
/** @defgroup  Sdram_IS42S32800D    Sdram IS42S32800D
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */

#ifndef __SDRAM_IS42S32800D_H
#define __SDRAM_IS42S32800D_H

#include "bsp.h"

#define SDRAM_BASE_ADDR     0xA0000000
#define SDRAM_SIZE          0x10000000

#if (_CURR_USING_BRD == _EA_PA_BOARD)
extern void SDRAMInit( void );
#endif

#endif //__SDRAM_IS42S32800D_H

/**
 * @}
 */
/*****************************************************************************
**                            End Of File
******************************************************************************/
