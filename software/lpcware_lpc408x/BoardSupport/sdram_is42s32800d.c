/**********************************************************************
* $Id$      sdram_is42s32800d.c         2011-08-22
*//**
* @file     sdram_is42s32800d.c
* @brief    Contains all functions support for ISSI IS42S32800D
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
#include "bsp.h"
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#if (_CURR_USING_BRD == _EA_PA_BOARD)
#ifdef _EMC

#include "bsp.h"
#include "lpc_emc.h"
#include "lpc_clkpwr.h"
#include "lpc_pinsel.h"
#include "lpc_timer.h"
#include "sdram_is42s32800d.h"

/* Public Functions ----------------------------------------------------------- */
/** @addtogroup Sdram_IS42S32800D
 * @{
 */

/*********************************************************************//**
 * @brief       Initialize external SDRAM memory ISSI IS42S32800D
 *              256Mbit(8M x 32)
 * @param[in]   None
 * @return      None
 **********************************************************************/
void SDRAMInit( void )
{
    volatile uint32_t i;
    volatile uint32_t dwtemp;
    EMC_DYN_MEM_Config_Type config;
    TIM_TIMERCFG_Type TIM_ConfigStruct;
      
    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue  = 1;
      
    // Set configuration for Tim_config and Tim_MatchConfig
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);
      
    config.ChipSize = 256;
    config.AddrBusWidth = 32;
    config.AddrMap = EMC_ADD_MAP_ROW_BANK_COL;
    config.CSn = 0;
    config.DataWidth = 16;
    config.TotalSize = SDRAM_SIZE;

    config.CASLatency= 2;
    config.RASLatency= 1;
    config.Active2ActivePeriod = 10;
    config.ActiveBankLatency =2;
    config.AutoRefrehPeriod = 10;
    config.DataIn2ActiveTime = 5;
    config.DataOut2ActiveTime = 5;
    config.WriteRecoveryTime = 2;
    config.ExitSelfRefreshTime = EMC_NS2CLK( 70);
    config.LoadModeReg2Active = 2;
    config.PrechargeCmdPeriod = 3;
    config.ReadConfig = 1;  /* Command delayed strategy, using EMCCLKDELAY */
    config.RefreshTime = EMC_SDRAM_REFRESH( 64);
    config.Active2PreChargeTime = 7;
    config.SeftRefreshExitTime = EMC_NS2CLK( 70);
    DynMem_Init(&config);
     
    EMC_DynCtrlSDRAMInit(EMC_DYNAMIC_CTRL_SDRAM_NOP); /* Issue NOP command */

    TIM_Waitms(100);                           /* wait 200ms */
    EMC_DynCtrlSDRAMInit(EMC_DYNAMIC_CTRL_SDRAM_PALL); /* Issue Pre-charge command */

    for(i = 0; i < 0x80; i++);          /* wait 128 AHB clock cycles */
    
    TIM_Waitms(100);    
    EMC_DynCtrlSDRAMInit(EMC_DYNAMIC_CTRL_SDRAM_MODE); /* Issue MODE command */

    dwtemp = *((volatile uint32_t *)(SDRAM_BASE_ADDR | (0x22<<(2+2+9)))); /* Mode Register Setting: 4 burst, 2 CAS latency */
    //Timing for 48/60/72MHZ Bus
    EMC_DynCtrlSDRAMInit(EMC_DYNAMIC_CTRL_SDRAM_NORMAL); /* Issue NORMAL command */

    //enable buffers
    EMC_DynMemConfigB(0, EMC_DYNAMIC_CFG_BUFF_ENABLED);
    
    
    TIM_DeInit(LPC_TIM0);
}

#endif /*_EMC*/
#endif /*(_CURR_USING_BRD == _EA_PA_BOARD)*/
/**
 * @}
 */

/*********************************************************************************
**                            End Of File
*********************************************************************************/
