/*****************************************************************************
 *   spifi_blinky.c:  main C entry file for NXP LPC177x_8x Family Microprocessors
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
#include "spifi_blinky.h"
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
void spifi_blinky(void)
{
  uint32_t i;

  while (1)
  {
 //   for(i = 0; i < 4; i++)
   // {
      LPC_GPIO1->SET = 1 << 18;
        for(i = 10000000; i > 0; i--);
//      spifi_delay();
    //}
    LPC_GPIO1->CLR = 1 << 18;
        for(i = 10000000; i > 0; i--);
//    spifi_delay();
  }
}

/*****************************************************************************
**                            End Of File
******************************************************************************/
