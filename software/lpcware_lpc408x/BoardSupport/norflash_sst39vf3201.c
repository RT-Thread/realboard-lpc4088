/**********************************************************************
* $Id$      norflash_sst39vf3201.c          2011-06-02
*//**
* @file     norflash_sst39vf3201.c
* @brief    Contains all functions support for NOR Flash SamSung
*           SST39VF3201
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
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _EMC
#include "norflash_sst39vf3201.h"
#include "lpc_emc.h"
#include "lpc_clkpwr.h"
#include "lpc_pinsel.h"
#include "lpc_timer.h"

int32_t volatile timerdev = 0;

/*********************************************************************//**
 * @brief       Delay
 * @param[in]   delayCnt Delay value
 * @return      None
 **********************************************************************/
void delay(uint32_t delayCnt)
{
    volatile uint32_t i;

    for ( i = 0; i < delayCnt; i++ );
    return;
}

/*********************************************************************//**
 * @brief       Initialize external NOR FLASH memory
 * @param[in]   None
 * @return      None
 **********************************************************************/
void NORFLASHInit( void )
{
    TIM_TIMERCFG_Type TIM_ConfigStruct;
    EMC_STATIC_MEM_Config_Type config;

    /**************************************************************************
    * Initialize EMC for NOR FLASH
    **************************************************************************/
    config.CSn = 0;
    config.AddressMirror = 0;
    config.ByteLane = 1;
    config.DataWidth = 16;
    config.ExtendedWait = 0;
    config.PageMode = 0;
    config.WaitWEn = 2;
    config.WaitOEn = 2;
    config.WaitWr = 0x1f;
    config.WaitPage = 0x1f;
    config.WaitRd = 0x1f;
    config.WaitTurn = 0x1f; 
    StaticMem_Init(&config);

    // init timer
    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue  = 1;

        // Set configuration for Tim_config and Tim_MatchConfig
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
    TIM_Waitms(100);

    //delay time
    TIM_Waitms(10);

    // EMC Shift Control
    LPC_SC->SCS       |= 1;
    return;
}

/*********************************************************************//**
 * @brief       Toggle Bit check if the data is written or erased
 * @param[in]   Addr    address value
 * @param[in]   Data    expected data
 * @return      Checking result, could be:
 *                  - TRUE: Done
 *                  - FALSE: Timeout
 **********************************************************************/
uint32_t ToggleBitCheck( uint32_t Addr, uint16_t Data )
{
    volatile uint16_t *ip;
    uint16_t temp1, temp2;
    uint32_t TimeOut = PROGRAM_TIMEOUT;

    while( TimeOut > 0 )
    {
        ip = GET_ADDR(Addr);
        temp1 = *ip;
        ip = GET_ADDR(Addr);
        temp2 = *ip;

        if ( (temp1 == temp2) && (temp1 == Data) )
        {
            return( TRUE );
        }
        TimeOut--;
    }
    return ( FALSE );
}

/*********************************************************************//**
 * @brief       Check ID from external NOR FLASH memory
 * @param[in]   None
 * @return      Checking result, could be:
 *                  - TRUE: Correct
 *                  - FALSE: Incorrect
 **********************************************************************/
uint32_t NORFLASHCheckID( void )
{
  volatile uint16_t *ip;
  uint16_t SST_id1, SST_id2;

  /*  Issue the Software Product ID code to 39VF160   */
  ip  = GET_ADDR(0x5555);
  *ip = 0x00AA;
  ip  = GET_ADDR(0x2AAA);
  *ip = 0x0055;
  ip  = GET_ADDR(0x5555);
  *ip = 0x0090;
  delay(10);

  /* Read the product ID from 39VF160 */
  ip  = GET_ADDR(0x0000);
  SST_id1 = *ip & 0x00FF;
  ip  = GET_ADDR(0x0001);
  SST_id2 = *ip;

  /* Issue the Soffware Product ID Exit code thus returning the 39VF160 */
  /* to the read operating mode */
  ip  = GET_ADDR(0x5555);
  *ip = 0x00AA;
  ip  = GET_ADDR(0x2AAA);
  *ip = 0x0055;
  ip  = GET_ADDR(0x5555);
  *ip = 0x00F0;
  delay(10);

  /* Check ID */
  if ((SST_id1 == SST_ID) && (SST_id2 ==SST_39VF160))
    return( TRUE );
  else
    return( FALSE );
}

/*********************************************************************//**
 * @brief       Erase external NOR FLASH memory
 * @param[in]   None
 * @return      None
 **********************************************************************/
void NORFLASHErase( void )
{
  volatile uint16_t *ip;

  ip  = GET_ADDR(0x5555);
  *ip = 0x00AA;
  ip  = GET_ADDR(0x2AAA);
  *ip = 0x0055;
  ip  = GET_ADDR(0x5555);
  *ip = 0x0080;
  ip  = GET_ADDR(0x5555);
  *ip = 0x00AA;
  ip  = GET_ADDR(0x2AAA);
  *ip = 0x0055;
  ip  = GET_ADDR(0x5555);
  *ip = 0x0010;
  delay(10000000);              /* Use timer 1 */
  return;

}

/*********************************************************************//**
 * @brief       Program one 16-bit data into external NOR FLASH memory
 *              This "uint16_t" for the external flash is 16 bits!!!
 * @param[in]   Addr    Address value
 * @param[in]   Data    data value
 * @return      Program result, could be:
                    - TRUE: succesful
                    - FALSE: fail
 **********************************************************************/
uint32_t NORFLASHWriteWord( uint32_t Addr, uint16_t Data )
{
  volatile uint16_t *ip;

  ip  = GET_ADDR(0x5555);
  *ip = 0x00AA;
  ip  = GET_ADDR(0x2aaa);
  *ip = 0x0055;
  ip  = GET_ADDR(0x5555);
  *ip = 0x00A0;

  ip = GET_ADDR(Addr);      /* Program 16-bit word */
  *ip = Data;
  return ( ToggleBitCheck( Addr, Data ) );
}

#endif /*_EMC*/
/*********************************************************************************
**                            End Of File
*********************************************************************************/
