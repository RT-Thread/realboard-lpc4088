/****************************************************************************
 *   $Id:: cmptest.c 5992 2010-12-22 21:10:51Z nxp28548                     $
 *   Project: NXP LPC17xx CMP example
 *
 *   Description:
 *     This file contains Comparator test code example to test
 *     Comparator initialization, COMP interrupt handler, and APIs.
 *
 ****************************************************************************
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
****************************************************************************/
#include "lpc407x_8x_177x_8x.h"
//#include "type.h"
#include "cmp.h"

/******************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{
  uint32_t i;

  //SystemInit();

  /* Set port 1_0 to output */
  LPC_GPIO1->DIR |= 1 << 0;     
  LPC_GPIO1->SET |= 1 << 0;
  /* Set port 1_1 to output */
  LPC_GPIO1->DIR |= 1 << 1;
  LPC_GPIO1->SET |= 1 << 1;
  /* Set port 1_2 to output */
  LPC_GPIO1->DIR |= 1 << 2;
  LPC_GPIO1->SET |= 1 << 2;

  CMP_Init();           

  CMP_CurrentSrcControl( POWERUP );            // Current Source powered


  CMP_BangapControl( POWERUP );                // Band Gap powered
    


  CMP_TempSenControl( ENABLE, POWERUP );       // Temp Sensor enabled, powered

  for (i = 0; i < 0x80; i++);                    // wait for comparators stablized
 
  CMP_SelectInput( 1, POWERUP, CMP_VP, 1 );    // CMP1, powered, VP -> CMP1_IN[2]
    
  CMP_SelectInput( 1, POWERUP, CMP_VM, 2 );    // CMP1, powered, VP -> CMP1_IN[1]
  CMP_SetHysteresis( 1, 0 );                   // CMP1, hysteresis : none
    
  CMP_SetOutput( 1, ENABLE, ASYNC );           // CMP1, enabled, async
    for (i = 0; i < 0x80; i++);                  // wait for comparators stablized

    
    if (CMP_GetOutputStatus(1))
    {
      LPC_GPIO1->SET |= 1 << 1;
    }
    else
    {
      LPC_GPIO1->CLR |= 1 << 1;
    }
        

    
    for (i = 0; i < 0x80; i++);                  // wait for comparators stablized
        
  CMP_SelectInput( 1, POWERUP, CMP_VP, 2 );    // CMP1, powered, VP -> CMP1_IN[2]
            
        for (i = 0; i < 0x80; i++);                  // wait for comparators stablized
        
  CMP_SelectInput( 1, POWERUP, CMP_VM, 1 );    // CMP1, powered, VP -> CMP1_IN[1]
  CMP_SetHysteresis( 1, 0 );                   // CMP1, hysteresis : none
  CMP_SetOutput( 1, ENABLE, ASYNC );           // CMP1, enabled, async
 for (i = 0; i < 0x80; i++);                     // wait for comparators stablized

        
    if (CMP_GetOutputStatus(1))
    {
      LPC_GPIO1->SET |= 1 << 1;
    }
    else
    {
      LPC_GPIO1->CLR |= 1 << 1;
    }

    while(1);   
        
        

  }


/******************************************************************************
**                            End Of File
******************************************************************************/

