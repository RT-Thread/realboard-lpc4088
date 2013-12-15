/*----------------------------------------------------------------------------
 * Name:    SPIFI.c
 * Purpose: SPIFI initialization
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

#include <LPC407x_8x_177x_8x.h>
#include "spifi_rom_api.h"

SPIFIobj   obj;

/*----------------------------------------------------------------------------
  Initialize SPIFI API & Pins
 *----------------------------------------------------------------------------*/
void SPIFI_Init (void) {
#ifdef USE_SPIFI_LIB
  /* Use spifi function names directly */
#else
  SPIFI_RTNS * pSpifi;
  pSpifi = (SPIFI_RTNS *)(SPIFI_ROM_PTR);
  /* Call functions via spifi rom table */
  #define spifi_init pSpifi->spifi_init
#endif

/* init SPIFI clock and pins */
  LPC_SC->PCONP      |=  (1UL << 16);        /* enable SPIFI power/clock   */

  LPC_IOCON->P2_7    &= ~(7UL <<  0);
  LPC_IOCON->P2_7    |=  (5UL <<  0);        /* SPIFI_CSN = P2.7  (FUNC 5) */
  LPC_IOCON->P0_22   &= ~(7UL <<  0);
  LPC_IOCON->P0_22   |=  (5UL <<  0);        /* SPIFI_CLK = P0.22 (FUNC 5) */
  LPC_IOCON->P0_15   &= ~(7UL <<  0);
  LPC_IOCON->P0_15   |=  (5UL <<  0);        /* SPIFI_IO2 = P0.15 (FUNC 5) */
  LPC_IOCON->P0_16   &= ~(7UL <<  0);
  LPC_IOCON->P0_16   |=  (5UL <<  0);        /* SPIFI_IO3 = P0.16 (FUNC 5) */
  LPC_IOCON->P0_17   &= ~(7UL <<  0);
  LPC_IOCON->P0_17   |=  (5UL <<  0);        /* SPIFI_IO1 = P0.17 (FUNC 5) */
  LPC_IOCON->P0_18   &= ~(7UL <<  0);
  LPC_IOCON->P0_18   |=  (5UL <<  0);        /* SPIFI_IO0 = P0.18 (FUNC 5) */

  if (spifi_init(&obj, 3, S_RCVCLK | S_FULLCLK, 48)) {
    while (1);
  }

}
