/******************************************************************************/
/* RETARGET.C: 'Retarget' layer for target-dependent low level functions      */
/******************************************************************************/
/* This file is part of the uVision/ARM development tools.                    */
/* Copyright (c) 2005-2007 Keil Software. All rights reserved.                */
/* This software may only be used under the terms of a valid, current,        */
/* end user licence from KEIL for a compatible version of KEIL software       */
/* development tools. Nothing else gives you the right to use this software.  */
/******************************************************************************/

#include <stdio.h>
#include <rt_misc.h>

#pragma import(__use_no_semihosting_swi)

// implementation depends on the microcontroller hardware
extern unsigned char  sendchar(unsigned char ch);  /* in serial.c */
extern unsigned char  getkey(void);

/**
 * @brief Handle Files
 */
struct __FILE { int handle; /* Add whatever you need here */ };

FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  return (sendchar(ch));
}

// simplified fgetc version that only redirects STDIN
#if 1
int fgetc(FILE * fp)
{
// redirect STDIN   
 return(getkey());
}
#else  // echo a char
int fgetc(FILE * fp)
{
 return(sendchar(getkey()));
}
#endif


int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
  sendchar(ch);
}


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}
