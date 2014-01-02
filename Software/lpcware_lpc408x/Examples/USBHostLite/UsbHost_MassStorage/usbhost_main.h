/**********************************************************************
* $Id$		usbhost_main.h			2011-09-05
*//**
* @file		usbhost_main.h
* @brief	Demo for USB Host Controller.
* @version	1.0
* @date		05. September. 2011
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

#ifndef  USBHOST_MAIN_H
#define  USBHOST_MAIN_H

/*
**************************************************************************************************************
*                                       DEFINITIONS
**************************************************************************************************************
*/

#include "usbhost_inc.h"

/** @addtogroup USBHost_MassStorage
 * @{
 */

/*
**************************************************************************************************************
*                                       DEFINITIONS
**************************************************************************************************************
*/

#define  MAX_BUFFER_SIZE             (4000)
#define  WRITE_SIZE          (10 * 1000000)

/* Public Functions ----------------------------------------------------------- */
/** @defgroup Demo Functions
 * @{
 */

void  Main_Copy (void);
void  Main_ReadDir (uint8_t* pName);

/**
 * @}
 */

/**
 * @}
 */
 


#endif
