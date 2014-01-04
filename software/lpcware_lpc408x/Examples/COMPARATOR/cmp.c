/****************************************************************************
 *   $Id:: cmp.c 5992 2010-12-22 21:10:51Z nxp28548                         $
 *   Project: NXP LPC17xx CMP example
 *
 *   Description:
 *     This file contains Comparator code example which include Comparator 
 *     initialization, CMP interrupt handler, and APIs.
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

/* statistics of all the interrupts */

/*****************************************************************************
** Function name:       CMP0_IRQHandler
**
** Descriptions:        Comparator 0 interrupt handler
**
** parameters:          None
** Returned value:      None
** 
*****************************************************************************/
void CMP0_IRQHandler(void) 
{
  LPC_COMPARATOR->CTRL0 |= (1 << 19);
#if 1
  if (LPC_GPIO1->PIN & (1 << 0))
  {
    LPC_GPIO1->CLR |= 1 << 0;
  }
  else
  {
    LPC_GPIO1->SET |= 1 << 0;
  }
#endif
  return;
}

/*****************************************************************************
** Function name:       CMP1_IRQHandler
**
** Descriptions:        Comparator 1 interrupt handler
**
** parameters:          None
** Returned value:      None
** 
*****************************************************************************/
void CMP1_IRQHandler(void) 
{
  LPC_COMPARATOR->CTRL1 |= (1 << 19);
#if 1
  if (LPC_GPIO1->PIN & (1 << 1))
  {
    LPC_GPIO1->CLR |= 1 << 1;
  }
  else
  {
    LPC_GPIO1->SET |= 1 << 1;
  }
#endif
  return;
}

/*****************************************************************************
** Function name:       CMP_IOConfig
**
** Descriptions:        CMP port initialization routine
**              
** parameters:          None
** Returned value:      None
** 
*****************************************************************************/
void CMP_IOConfig( void )
{
  /*  Comparator I/O config */
  LPC_IOCON->P1_14  &= ~0x9F;                //why bit [7] is reserved, ????????????????
  LPC_IOCON->P1_14  |= 5; /* CMP0_IN[1] @ P1.14 */
  LPC_IOCON->P1_16  &= ~0x9F;
  LPC_IOCON->P1_16  |= 5; /* CMP0_IN[2] @ P1.16 */
  LPC_IOCON->P1_17  &= ~0x9F;
  LPC_IOCON->P1_17  |= 5; /* CMP0_IN[3] @ P1.17 */
  LPC_IOCON->P1_6   &= ~0x9F;
  LPC_IOCON->P1_6   |= 5; /* CMP0_IN[4] @ P1.6  */
  LPC_IOCON->P4_30  |= 5; /* CMP0_OUT   @ P4.30 */  //why this pin does not need the ~0x9F?

  LPC_IOCON->P1_7   &= ~0x9F;
  LPC_IOCON->P1_7   |= 5; /* CMP1_IN[1] @ P1.7  */
  LPC_IOCON->P1_5   &= ~0x9F;
  LPC_IOCON->P1_5   |= 5; /* CMP1_IN[2] @ P1.5  */
  LPC_IOCON->P0_9   &= ~0x9F;
  LPC_IOCON->P0_9   |= 5; /* CMP1_IN[3] @ P0.9  */
  LPC_IOCON->P0_8   &= ~0x9F;
  LPC_IOCON->P0_8   |= 5; /* CMP1_IN[4] @ P0.8  */
  LPC_IOCON->P1_12  |= 5; /* CMP1_OUT   @ P1.12 */

  LPC_IOCON->P0_7   &= ~0x9F;
  LPC_IOCON->P0_7   |= 5; /* CMP_VREF   @ P0.7  */
#if 1
  LPC_IOCON->P0_6   |= 5; /* CMP_ROSC   @ P0.6  */
#endif
  LPC_IOCON->P0_5   |= 5; /* CMP_RESET  @ P0.5  */
#if 0
  LPC_IOCON->P0_4   |= 5; /* CMP_ROSC   @ P0.4  */
#endif
  return;       
}

/*****************************************************************************
** Function name:       CMP_Init
**
** Descriptions:        Comparator initialization routine
**              
** parameters:          None
** Returned value:      None
** 
*****************************************************************************/
void CMP_Init( void )
{
  LPC_SC->PCONP1 |= (1 << 3);       //power on the comparator

  LPC_SC->RSTCON1 |= (1 << 3);  // reset the comparator

  LPC_SC->RSTCON1 &= ~(1 << 3);

  CMP_IOConfig();

  return;
}

/*****************************************************************************
** Function name:       CMP_SelectInput
**
** Descriptions:        Select comparator input channel
**
** parameters:          num, power, channel, input
** Returned value:      None
** 
*****************************************************************************/
void CMP_SelectInput( uint32_t num, uint32_t power, uint32_t channel, uint32_t input )
{
  if ( num == 0 )
  {
    LPC_COMPARATOR->CTRL0 &= ~(0x3 << 0); //disable
    LPC_COMPARATOR->CTRL0 |= ((0x3 & power) << 0); //enable
    switch ( channel )
    {
      case CMP_VP: //positive Input
        LPC_COMPARATOR->CTRL0 &= ~(7 << 8); // this seems selects Vref divider 0, this makes [10:8]=0
        LPC_COMPARATOR->CTRL0 |= ((0x7 & input) << 8);  //this sets the selected input
      break;
      case CMP_VM: //negative Input
        LPC_COMPARATOR->CTRL0 &= ~(7 << 4); // this seems selects Vref divider 0, this makes [6:4]=0
        LPC_COMPARATOR->CTRL0 |= ((0x7 & input) << 4);      //this sets the selected input
      break;
      default:
      break;
    }
  }
  else if ( num == 1 )
  {
    LPC_COMPARATOR->CTRL1 &= ~(0x3 << 0);
    LPC_COMPARATOR->CTRL1 |= ((0x3 & power) << 0);
            
    switch ( channel )
    {
      case CMP_VP:
        LPC_COMPARATOR->CTRL1 &= ~(7 << 8);
        LPC_COMPARATOR->CTRL1 |= ((0x7 & input) << 8);
            
      break;
      case CMP_VM:
        LPC_COMPARATOR->CTRL1 &= ~(7 << 4);
        LPC_COMPARATOR->CTRL1 |= ((0x7 & input) << 4);
      break;
      default:
      break;
    }
  }

  return;
}

/*****************************************************************************
** Function name:       CMP_SelectReference
**
** Descriptions:        Select comparator voltage reference
**
** parameters:          num, vref, level
** Returned value:      None
** 
*****************************************************************************/
void CMP_SelectReference( uint32_t num, uint32_t power, uint32_t vref, uint32_t level )
{
  if ( num == 0 )
  {
    if ( vref == VDDA )
    {
      LPC_COMPARATOR->CTRL0 &= ~(1 << 22);
    }
    else if ( vref == VREF )
    {
      LPC_COMPARATOR->CTRL0 |= (1 << 22);
    }
    LPC_COMPARATOR->CTRL0 &= ~(0x3 << 20);
    LPC_COMPARATOR->CTRL0 |= ((0x3 & power) << 20);   //[21:20] is voltage ladder enable for comparator 0
    LPC_COMPARATOR->CTRL0 &= ~(0x1F << 24);
    LPC_COMPARATOR->CTRL0 |= ((0x1F & level) << 24);  //set voltage ladder level
  }
  else if ( num == 1 )
  {
    if ( vref == VDDA )
    {
      LPC_COMPARATOR->CTRL1 &= ~(1 << 22);
    }
    else if ( vref == VREF )
    {
      LPC_COMPARATOR->CTRL1 |= (1 << 22);
    }
    LPC_COMPARATOR->CTRL1 &= ~(0x3 << 20);
    LPC_COMPARATOR->CTRL1 |= ((0x3 & power) << 20);
    LPC_COMPARATOR->CTRL1 &= ~(0x1F << 24);
    LPC_COMPARATOR->CTRL1 |= ((0x1F & level) << 24);
  }
  return;
}

/*****************************************************************************
** Function name:       CMP_SetOutput
**
** Descriptions:        Set output enable, sync
**
** parameters:          num, enable, sync
** Returned value:      None
** 
*****************************************************************************/
void CMP_SetOutput( uint32_t num, uint32_t enable, uint32_t sync )
{
  if ( num == 0 )
  {
    if ( enable == DISABLE )
    {
      LPC_COMPARATOR->CTRL0 &= ~(1 << 2);
    }
    else if ( enable == ENABLE )
    {
      LPC_COMPARATOR->CTRL0 |= (1 << 2);
    }
    if ( sync == ASYNC )
    {
      LPC_COMPARATOR->CTRL0 &= ~(1 << 12);
    }
    else if ( sync == SYNC )
    {
      LPC_COMPARATOR->CTRL0 |= (1 << 12);
    }
  }
  else if ( num == 1 )
  {
    if ( enable == DISABLE )
    {
      LPC_COMPARATOR->CTRL1 &= ~(1 << 2);
    }
    else if ( enable == ENABLE )
    {
      LPC_COMPARATOR->CTRL1 |= (1 << 2);
    }
    if ( sync == ASYNC )
    {
      LPC_COMPARATOR->CTRL1 &= ~(1 << 12);
    }
    else if ( sync == SYNC )
    {
      LPC_COMPARATOR->CTRL1 |= (1 << 12);
    }
  }
  return;
}

/*****************************************************************************
** Function name:       CMP_SetInterrupt
**
** Descriptions:        Set interrupt polarity, type, edge.
**
** parameters:          num, inverted, edge, event
** Returned value:      None
** 
*****************************************************************************/
void CMP_SetInterrupt( uint32_t num, uint32_t inverted, uint32_t level, uint32_t edge )
{
  if ( num == 0 )
  {
    NVIC_DisableIRQ(CMP0_IRQn);
        
    if ( inverted == 0 )
    {
      LPC_COMPARATOR->CTRL0 &= ~(1 << 15);
    }
    else
    {
      LPC_COMPARATOR->CTRL0 |= (1 << 15);
    }
    if ( level == 0 )
    {
      LPC_COMPARATOR->CTRL0 &= ~(1 << 16);
    }
    else
    {
      LPC_COMPARATOR->CTRL0 |= (1 << 16);
    }
    LPC_COMPARATOR->CTRL0 &= ~(0x3 << 17);
    LPC_COMPARATOR->CTRL0 |= ((0x3 & edge) << 17);
    LPC_COMPARATOR->CTRL0 |= (1 << 19); //writing a 1 to this bit clears the flag.
    NVIC_ClearPendingIRQ(CMP0_IRQn);
    NVIC_EnableIRQ(CMP0_IRQn); 
  }
  else if ( num == 1 )
  {
    NVIC_DisableIRQ(CMP1_IRQn);
        
    if ( inverted == 0 )
    {
      LPC_COMPARATOR->CTRL1 &= ~(1 << 15);
    }
        
    else
    {
      LPC_COMPARATOR->CTRL1 |= (1 << 15);
    }
        
    if ( level == 0 )
    {
      LPC_COMPARATOR->CTRL1 &= ~(1 << 16);
    }
    else
    {
      LPC_COMPARATOR->CTRL1 |= (1 << 16);
    }
        
    LPC_COMPARATOR->CTRL1 &= ~(0x3 << 17);
    LPC_COMPARATOR->CTRL1 |= ((0x3 & edge) << 17);
    LPC_COMPARATOR->CTRL1 |= (1 << 19);
        
    NVIC_ClearPendingIRQ(CMP1_IRQn);
        
    NVIC_EnableIRQ(CMP1_IRQn); 
        
  }
  return;
}

/*****************************************************************************
** Function name:       CMP_SetHysteresis
**
** Descriptions:        Set hysteresis
**
** parameters:          num, level
** Returned value:      None
** 
*****************************************************************************/
void CMP_SetHysteresis( uint32_t num, uint32_t level )
{
  if ( num == 0 )
  {
    LPC_COMPARATOR->CTRL0 &= ~(0x3 << 13);
    LPC_COMPARATOR->CTRL0 |= ((0x3 & level) << 13);
  }
  else if ( num == 1 )
  {
    LPC_COMPARATOR->CTRL1 &= ~(0x3 << 13);
    LPC_COMPARATOR->CTRL1 |= ((0x3 & level) << 13);
  }
  return;
}

/*****************************************************************************
** Function name:       CMP_GetOutputStatus
**
** Descriptions:        Get output status
**
** parameters:          num
** Returned value:      Output status
** 
*****************************************************************************/
uint32_t CMP_GetOutputStatus( uint32_t num )
{
  uint32_t regVal = 0;
  
  if ( num == 0 )
  {
    regVal = ( LPC_COMPARATOR->CTRL0 & (1 << 3) ) >> 3;
  }
  else if ( num == 1 )
  {
    regVal = ( LPC_COMPARATOR->CTRL1 & (1 << 3) ) >> 3;
  }
  return regVal;
}

/*****************************************************************************
** Function name:       CMP_ConnectCapture
**
** Descriptions:        Connect timer capture to comparator output
**
** parameters:          num, capture
** Returned value:      None
** 
*****************************************************************************/
void CMP_ConnectCapture( uint32_t capture, uint32_t select )
{
  switch ( capture )
  {
    case T0CAP2:
      if ( select == 0 )
      {
        LPC_COMPARATOR->CTRL &= ~(1 << 12);
      }
      else if ( select == 1 )
      {
        LPC_COMPARATOR->CTRL |= (1 << 12);
      }
    break;
    case T0CAP3:
      if ( select == 0 )
      {
        LPC_COMPARATOR->CTRL &= ~(1 << 13);
      }
      else if ( select == 1 )
      {
        LPC_COMPARATOR->CTRL |= (1 << 13);
      }
    break;
    case T1CAP2:
      if ( select == 0 )
      {
        LPC_COMPARATOR->CTRL &= ~(1 << 14);
      }
      else if ( select == 1 )
      {
        LPC_COMPARATOR->CTRL |= (1 << 14);
      }
    break;
    case T1CAP3:
      if ( select == 0 )
      {
        LPC_COMPARATOR->CTRL &= ~(1 << 15);
      }
      else if ( select == 1 )
      {
        LPC_COMPARATOR->CTRL |= (1 << 15);
      }
    break;
    default:
    break;
  }
  return;
}

/*****************************************************************************
** Function name:       ROSCControl
**
** Descriptions:        ROSC Control
**
** parameters:          input, reset
** Returned value:      None
** 
*****************************************************************************/
void CMP_ROSCControl( uint32_t input, uint32_t reset )
{
  if ( input == 0 )
  {
    LPC_COMPARATOR->CTRL &= ~(1 << 8);
  }
  else if ( input == 1 )
  {
    LPC_COMPARATOR->CTRL |= (1 << 8);
  }
  if ( reset == 0 )
  {
    LPC_COMPARATOR->CTRL &= ~(1 << 9);
  }
  else if ( reset == 1 )
  {
    LPC_COMPARATOR->CTRL |= (1 << 9);
  }
  return ;
}

/*****************************************************************************
** Function name:       CMP_TempSenControl
**
** Descriptions:        Temp Sensor Control
**
** parameters:          enable, power
** Returned value:      None
** 
*****************************************************************************/
void CMP_TempSenControl( uint32_t enable, uint32_t power )
{
  LPC_COMPARATOR->CTRL &= ~(0x3 << 6);
  LPC_COMPARATOR->CTRL |= ((0x3 & enable) << 6);
  LPC_COMPARATOR->CTRL &= ~(0x3 << 4);
  LPC_COMPARATOR->CTRL |= ((0x3 & power) << 4);
  return ;
}

/*****************************************************************************
** Function name:       CMP_BangapControl
**
** Descriptions:        Band Gap Control
**
** parameters:          enable
** Returned value:      None
** 
*****************************************************************************/
void CMP_BangapControl( uint32_t enable )
{
  LPC_COMPARATOR->CTRL &= ~(0x3 << 2);
  LPC_COMPARATOR->CTRL |= ((0x3 & enable) << 2);
  return ;
}

/*****************************************************************************
** Function name:       CMP_CurrentSrcControl
**
** Descriptions:        Current Source Control
**
** parameters:          power
** Returned value:      None
** 
*****************************************************************************/
void CMP_CurrentSrcControl( uint32_t power )
{
  LPC_COMPARATOR->CTRL &= ~(0x3 << 0);  //the comparator current source is disabled
  LPC_COMPARATOR->CTRL |= ((0x3 & power) << 0);
  return ;
}

/******************************************************************************
**                            End Of File
******************************************************************************/

