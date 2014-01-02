/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : logo.h
 *    Description : Logo picture include file
 *
 *    History :
 *    1. Date        : 7, March 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 24636 $
 **************************************************************************/
#ifndef __LOGO_H
#define __LOGO_H

#include "lpc_lcd.h"

#define   LOGO_BPP       16

#define   BMP_BYTES_PP   4	

#if (LOGO_BPP < 24)
#undef    BMP_BYTES_PP
#define   BMP_BYTES_PP   (LOGO_BPP/8)
#endif

#endif // __LOGO_H
