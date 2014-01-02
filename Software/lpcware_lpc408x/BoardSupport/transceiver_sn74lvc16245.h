/**********************************************************************
* $Id$      transceiver_sn74lvc16245.c          2011-06-02
*//**
* @file     transceiver_sn74lvc16245.c
* @brief    This SN74LVC16245 is mounted on the LPC7188 OEM Board (target
*           board, not for the base board)
*           Contains all macro, definitions and function prototypes that
*           support this SN74LVC16245 transceiver
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
/** @defgroup  Transceiver_SN74LVC16245 Transceiver SN74LVC16245
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */
#ifndef __TRANSCEIVER_SN74LVC16245_H
#define __TRANSCEIVER_SN74LVC16245_H

#include "lpc_pinsel.h"
#include "lpc_gpio.h"


#define SN74X_DIR_CONTROLLED_PORT       (4)
#define SN74X_DIR_CONTROLLED_PIN        (24)
#define SN74X_DIR_CONTROLLED_PINMASK    (1 << SN74X_DIR_CONTROLLED_PIN)


#define SN74X_OE0_CONTROLLED_PORT       (4)
#define SN74X_OE0_CONTROLLED_PIN        (26)
#define SN74X_OE0_CONTROLLED_PINMASK    (1 << SN74X_OE0_CONTROLLED_PIN)

#define SN74X_OE1_CONTROLLED_PORT       (4)
#define SN74X_OE1_CONTROLLED_PIN        (27)
#define SN74X_OE1_CONTROLLED_PINMASK    (1 << SN74X_OE1_CONTROLLED_PIN)

#define SN74X_OE2_CONTROLLED_PORT       (4)
#define SN74X_OE2_CONTROLLED_PIN        (28)
#define SN74X_OE2_CONTROLLED_PINMASK    (1 << SN74X_OE2_CONTROLLED_PIN)

#define SN74X_OE3_CONTROLLED_PORT       (4)
#define SN74X_OE3_CONTROLLED_PIN        (29)
#define SN74X_OE3_CONTROLLED_PINMASK    (1 << SN74X_OE3_CONTROLLED_PIN)


typedef enum sn74x_Group_Cntrl
{
    ///From pin P3.0 .. P3.7
    SN74X_PORT3_GROUP_0 = 0,

    ///From pin P3.8 .. P3.15
    SN74X_PORT3_GROUP_1,

    ///From pin P3.16 .. P3.23
    SN74X_PORT3_GROUP_2,

    ///From pin P3.24 .. P3.31
    SN74X_PORT3_GROUP_3,

    SN74X_PORT3_GROUP_NUMGRP
}en_sn74x_Group_Cntrl;


typedef enum sn74x_Group_Direction
{
    ///From pin P3.0 .. P3.7
    SN74X_GRP_DATA_OUTPUT = 0,

    ///From pin P3.8 .. P3.15
    SN74X_GRP_DATA_INPUT
}en_sn74x_Group_DataDir;


void sn74x_Init(uint8_t port3Grp);
void sn74x_DeInit(uint8_t port3Grp);
void sn74x_SetDataDir(uint8_t dataDir);


#endif//__TRANSCEIVER_SN74LVC16245_H

/**
 * @}
 */
