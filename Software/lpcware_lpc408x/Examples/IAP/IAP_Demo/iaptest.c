/**********************************************************************
* $Id$      iaptest.c           2011-11-21
*//**
* @file     lpc_iap.h
 * @brief          IAP demo
* @version  1.0
* @date     21. November. 2011
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

#include "LPC407x_8x_177x_8x.h"
#include "lpc_types.h"
#include "lpc_iap.h"
#include "debug_frmwrk.h"

 

/** The area will be erase and program */
#define FLASH_PROG_AREA_START       0x8000
#define FLASH_PROG_AREA_SIZE        0x1000


/** The origin buffer on RAM */
#define BUFF_SIZE           1024
#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=4
uint8_t buffer[BUFF_SIZE];
#else
uint8_t __attribute__ ((aligned (4))) buffer[BUFF_SIZE];
#endif

/** @defgroup IAP_Demo  IAP Demo
 * @ingroup IAP_Examples
 * @{
 */
 

/** Main menu */
uint8_t menu[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" IAP Demotrastion \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
"********************************************************************************\n\r";

/*********************************************************************//**
 * @brief       The entry of the program
 *
 * @param[in]None
 *
 * @return  None.
 *
 **********************************************************************/
void c_entry (void)
{               
  uint32_t result[4];
  uint8_t ver_major, ver_minor;
  uint32_t i;
  uint8_t *ptr;
  uint32_t flash_prog_area_sec_start;
  uint32_t flash_prog_area_sec_end;
  IAP_STATUS_CODE status;

  // Initialize
  debug_frmwrk_init();
  for (i = 0;i < sizeof(buffer);i++)
  {
    buffer[i] = (uint8_t)i;
  }
  flash_prog_area_sec_start = GetSecNum(FLASH_PROG_AREA_START);
  flash_prog_area_sec_end =  GetSecNum(FLASH_PROG_AREA_START + FLASH_PROG_AREA_SIZE);

  _DBG_(menu);

  status = ReadPartID(result);
  if(status != CMD_SUCCESS)
  {
     _DBG("Read Part ID failed with code is ");_DBD(status);_DBG_("");
     while(1);
  }

  _DBG("PartID: ");_DBH32(result[0]);_DBG_("");
  
  status = ReadBootCodeVer(&ver_major, &ver_minor);
  if(status != CMD_SUCCESS)
  {
     _DBG("Read Boot Code Version failed with code is ");_DBD(status);_DBG_("");
     while(1);
  }

  _DBG("Boot Code Version: ");_DBD(ver_major);_DBG(".");_DBD(ver_minor);_DBG_("");

  status = ReadDeviceSerialNum(result);
  if(status != CMD_SUCCESS)
  {
     _DBG("Read UID failed with code is ");_DBD(status);_DBG_("");
     while(1);
  }

  _DBG("UID: ");
  for(i = 0; i < 4; i++)
  {
     _DBD32(result[i]);
     if(i<3)
       _DBG("-");
  }
  _DBG_("");

  status = EraseSector(flash_prog_area_sec_start, flash_prog_area_sec_end); 
  if(status != CMD_SUCCESS)
  {
     _DBG("Erase chip failed with code is ");_DBD(status);_DBG_("");
     while(1); 
  }

  status = BlankCheckSector(flash_prog_area_sec_start, flash_prog_area_sec_end,
                                  &result[0], &result[1]);
  if(status != CMD_SUCCESS)
  {
     _DBG("Blank Check failed with code is ");_DBD(status);_DBG_("");
     if(status == SECTOR_NOT_BLANK)
     {
       _DBG(">>>>The first non-blank sector is sector ");
       _DBD(flash_prog_area_sec_start + result[0]);
       _DBG_("");
     }
     while(1); 
  }

  _DBG_("Erase chip: Success");


  /* Be aware that Program and ErasePage take long time to complete!!! If bigger
  RAM is present, allocate big buffer and reduce the number of Program blocks. */

  /* Program flash block by block until the end of the flash. */
  for ( i = 0; i < FLASH_PROG_AREA_SIZE/BUFF_SIZE; i++ )
  {
    ptr = (uint8_t*)(FLASH_PROG_AREA_START + i*BUFF_SIZE);
    status =  CopyRAM2Flash(ptr, buffer,IAP_WRITE_1024);
    if(status != CMD_SUCCESS)
    {
       _DBG("Program chip failed with code is ");_DBD(status);_DBG_("");
       while(1);
    }
  }
  // Compare
  for ( i = 0; i < FLASH_PROG_AREA_SIZE/BUFF_SIZE; i++ )
  {
    ptr = (uint8_t*)(FLASH_PROG_AREA_START + i*BUFF_SIZE);
    status =  Compare(ptr, buffer,BUFF_SIZE);
    if(status != CMD_SUCCESS)
    {
       _DBG("Compare memory failed with code is ");_DBD(status);_DBG_("");
       while(1);
    }
  }

   _DBG_("Program chip: Success");

  _DBG_("Demo termination");  
  
  while (1);
}

/*********************************************************************//**
 * @brief       The main program
 *
 * @param[in] None
 *
 * @return  None.
 *
 **********************************************************************/
 int main (void)
{
   c_entry();
   return 0;
}

