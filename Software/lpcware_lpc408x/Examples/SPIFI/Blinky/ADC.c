/*----------------------------------------------------------------------------
 * Name:    ADC.c
 * Purpose: low level ADC functions
 * Note(s): ADC works in Interrupt mode
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

#include <LPC407x_8x_177x_8x.h>
#include "ADC.h"

uint16_t AD_last;                            /* Last converted value          */
uint8_t  AD_done = 0;                        /* AD conversion done flag       */

/*----------------------------------------------------------------------------
  Function that initializes ADC
 *----------------------------------------------------------------------------*/
void ADC_Init (void) {

  LPC_SC->PCONP    |= (( 1UL << 12) |        /* enable power to ADC           */
                       ( 1UL << 15)  );      /* enable power to IOCON         */

  LPC_IOCON->P0_25  =  ( 1UL <<  0);         /* P0.25 is AD0.2                */

  LPC_ADC->CR       = (( 1UL <<  2) |        /* select AD0.2 pin              */
                       ( 4UL <<  8) |        /* ADC clock is 60MHz / 5        */
                       ( 1UL << 21)  );      /* enable ADC                    */

  LPC_ADC->INTEN    =  ( 1UL <<  8);         /* global enable interrupt       */

  NVIC_EnableIRQ(ADC_IRQn);                  /* enable ADC Interrupt          */
}


/*----------------------------------------------------------------------------
  start AD Conversion
 *----------------------------------------------------------------------------*/
void ADC_StartCnv (void) {

  LPC_ADC->CR &= ~( 7UL << 24);              /* stop conversion               */
  LPC_ADC->CR |=  ( 1UL << 24);              /* start conversion              */
}


/*----------------------------------------------------------------------------
  get converted AD value
 *----------------------------------------------------------------------------*/
uint16_t ADC_GetCnv (void) {

  return(AD_last);
}


/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is done
 *----------------------------------------------------------------------------*/
void ADC_IRQHandler(void) {
  volatile uint32_t adstat;

  adstat = LPC_ADC->STAT;                        /* Read ADC clears interrupt     */

  AD_last = (LPC_ADC->GDR >> 4) & ADC_VALUE_MAX;   /* Store converted value   */

  AD_done = 1;
}
