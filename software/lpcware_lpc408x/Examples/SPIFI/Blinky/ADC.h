/*----------------------------------------------------------------------------
 * Name:    ADC.h
 * Purpose: low level ADC definitions
 * Note(s):
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2012 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#ifndef __ADC_H
#define __ADC_H

#define ADC_VALUE_MAX      (0xFFF)

extern uint16_t AD_last;
extern uint8_t  AD_done;

extern void     ADC_Init    (void);
extern void     ADC_StartCnv(void);
extern uint16_t ADC_GetCnv  (void);

#endif
