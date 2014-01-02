/**********************************************************************
* $Id$      usbhost_uart.c          2011-09-05
*//**
* @file     usbhost_uart.c
* @brief        Provide APIs for Printing debug information.
* @version  1.0
* @date     05. September. 2011
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


/*
**************************************************************************************************************
*                                           INCLUDE HEADER FILES
**************************************************************************************************************
*/
#include <stdio.h>
#include <stdarg.h>
#include "usbhost_uart.h"

#include "system_LPC407x_8x_177x_8x.h"
#include "debug_frmwrk.h"

/** @addtogroup USBHost_Uart
 * @{
 */


/*********************************************************************//**
 * @brief           print a single charecter through UART
 * @param[in]       ch    charecter to be printed
 * @return      None.
 **********************************************************************/
void  UART_PrintChar (uint8_t ch)
{
  _DBC(ch);
}


/*********************************************************************//**
 * @brief           print a string
 * @param[in]        str    Pointer to the string
 * @return      None.
 **********************************************************************/
void  UART_PrintStr (const uint8_t * str)
{

   while ((*str) != 0) {
      if (*str == '\n') {
         UART_PrintChar(*str++);
         UART_PrintChar('\r');
      } else {
         UART_PrintChar(*str++);
      }    
   }
}

/*********************************************************************//**
 * @brief           print formatted string. This function takes variable length arguments
 * @param[in]       variable length arguments
 * @return      None.
 **********************************************************************/
void  UART_Printf (const  void *format, ...)
{
    static  uint8_t  buffer[40 + 1];
            va_list     vArgs;

    va_start(vArgs, format);
    vsprintf((char *)buffer, (char const *)format, vArgs);
    va_end(vArgs);
    UART_PrintStr((uint8_t *) buffer);
}
/**
* @}
*/

