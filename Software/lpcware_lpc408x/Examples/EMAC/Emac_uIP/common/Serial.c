/******************************************************************************/
/* SERIAL.C: Low Level Serial Routines                                        */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include "debug_frmwrk.h"

#define CR     0x0D


void init_serial (void)  {               /* Initialize Serial Interface       */
    debug_frmwrk_init();
}


/* Implementation of putchar (also used by printf function to output data)    */
int sendchar (int ch)  {                 /* Write character to Serial Port    */
#if 1
   if (ch == '\n')  {
   _DBC(CR);                          /* output CR */
  }
#endif
    _DBC(ch);
    return (ch);
}


int getkey (void)  {                     /* Read character from Serial Port   */
  return (_DG);
}

unsigned char getchar(void)
{
    return getkey();
}
