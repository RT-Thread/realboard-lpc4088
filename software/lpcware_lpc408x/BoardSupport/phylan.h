
/**********************************************************************
* $Id$      phylan.c            2011-11-01
*//**
* @file     phylan.c
* @brief    Contains all macro definitions and function prototypes
*           support for external PHY IC LAN8720 to work with LAN
* @version  1.0
* @date     01. November. 2011
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
#if (_CURR_USING_BRD == _QVGA_BOARD)
//Driver for PHY of LAN DP83848C IC
#include "phylan_dp83848c.h"
#elif (_CURR_USING_BRD == _EA_PA_BOARD)
//Driver for PHY of LAN LAN8720 IC
#include "phylan_lan8720.h"
#elif (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
//Driver for PHY of LAN KS8721B IC
#include "phylan_ks8721b.h"
#endif
#include "lpc_emac.h"


int32_t PHY_Init(EMAC_PHY_CFG_Type *pConfig);
int32_t PHY_Reset(void);
int32_t PHY_CheckStatus(uint32_t ulPHYState);
int32_t PHY_SetMode(uint32_t ulPHYMode);
int32_t PHY_UpdateStatus(void);





