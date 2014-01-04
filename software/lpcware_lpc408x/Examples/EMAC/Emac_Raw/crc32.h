/*****************************************************************************
 *   crc32.h:  Ethernet CRC module file for NXP LPC230x Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __CRC32_H
#define __CRC32_H

#include "lpc_types.h"

void   crc32_init(uint32_t *pCRC);
void   crc32_add(uint32_t *pCRC, uint8_t val8);
void   crc32_end(uint32_t *pCRC);
uint32_t  crc32_bfr(void *pBfr, uint32_t size);
uint32_t do_crc_behav( long long Addr );

#endif

/*-----------------------------------------------------------*/
