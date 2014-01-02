/*****************************************************************************
 *   crc32.c:  Ethernet CRC module file for NXP LPC230x Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include "lpc_emac.h"                        /* LPC23xx/24xx definitions */
#include "crc32.h"

/******************************************************************************
** Function name:       CRC_init
**
** Descriptions:        Begin CRC calculation.
**
** parameters:          pointer to the CRC area.
** Returned value:      None
**
******************************************************************************/
void crc32_init(uint32_t *pCRC)
{
    *pCRC = 0xffffffff;
}

/******************************************************************************
** Function name:       CRC32_add
**
** Descriptions:        Calculate CRC value one at a time
**
** parameters:          pointer to the CRC area, and passing value to get the CRC
** Returned value:      None
**
******************************************************************************/
void crc32_add(uint32_t *pCRC, uint8_t val8)
{
    uint32_t i, poly;
    uint32_t entry;
    uint32_t crc_in;
    uint32_t crc_out;

    crc_in = *pCRC;
    poly = 0xEDB88320L;
    entry = (crc_in ^ ((uint32_t) val8)) & 0xFF;
    for (i = 0; i < 8; i++)
    {
        if (entry & 1)
            entry = (entry >> 1) ^ poly;
        else
            entry >>= 1;
    }
    crc_out = ((crc_in>>8) & 0x00FFFFFF) ^ entry;
    *pCRC = crc_out;
    return;
}

/******************************************************************************
** Function name:       CRC32_end
**
** Descriptions:        Finish CRC calculation
**
** parameters:          pointer to the CRC area.
** Returned value:      None
**
******************************************************************************/
void crc32_end(uint32_t *pCRC)
{
    *pCRC ^= 0xffffffff;
}

/******************************************************************************
** Function name:       CRC32_bfr
**
** Descriptions:        Get the CRC value based on size of the string.
**
** parameters:          Pointer to the string, size of the string.
** Returned value:      CRC value
**
******************************************************************************/
uint32_t crc32_bfr(void *pBfr, uint32_t size)
{
    uint32_t crc32;
    uint8_t  *pu8;

    crc32_init(&crc32);
    pu8 = (uint8_t *) pBfr;
    while (size-- != 0)
    {
        crc32_add(&crc32, *pu8);
        pu8++ ;
    }
    crc32_end(&crc32);
    return ( crc32 );
}

/*********************************************************************************
**                            End Of File
*********************************************************************************/
