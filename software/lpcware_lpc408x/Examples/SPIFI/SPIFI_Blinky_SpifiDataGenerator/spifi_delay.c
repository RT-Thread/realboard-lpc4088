/*****************************************************************************
 *   spifi_delay.c:  main C entry file for NXP LPC177x_8x Family Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.26  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/

#include "LPC407x_8x_177x_8x.h"
//#include "type.h"
#include "spifi_delay.h"

/******************************************************************************
** Function name:       spifi_blinky
**
** Descriptions:        spifi blinky
**
** parameters:          None
** Returned value:      None
** 
******************************************************************************/
void spifi_delay(void)
{
  uint32_t i;

  for(i = 1000000; i > 0; i--);
}

/*****************************************************************************
**                            End Of File
******************************************************************************/
