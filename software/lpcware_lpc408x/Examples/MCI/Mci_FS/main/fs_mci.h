/***
 * @file		fs_mci.h
 * @purpose		Drivers for SD
 * @version		1.0
 * @date		23. February. 2012
 * @author		NXP MCU SW Application Team
 *---------------------------------------------------------------------
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

#include "lpc_types.h"
#include "lpc_mci.h"
#include "diskio.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup MCI_FS	MCI File System
 * @ingroup MCI_Examples
 * @{
 */
 
 typedef struct tagCARDCONFIG
{
    uint32_t 		SectorSize;    /* size (in byte) of each sector, fixed to 512bytes */
    uint32_t 		SectorCount;     /* total sector number */  
    uint32_t 		BlockSize;     /* erase block size in unit of sector */
	uint32_t 		CardAddress;	/* Card Address */
	uint32_t 		OCR;			/* OCR */
	en_Mci_CardType CardType;		/* Card Type */
	st_Mci_CardId 	CardID;			/* CID */
	uint8_t  		CSD[16];		/* CSD */
} CARDCONFIG;

extern CARDCONFIG CardConfig;

Bool mci_read_configuration (void);
void disk_timerproc (void);

DSTATUS disk_initialize (
	BYTE drv		/* Physical drive number (0) */
);

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive number (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
);


DRESULT disk_read (
	BYTE drv,			/* Physical drive number (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
);
DSTATUS disk_status (
	BYTE drv		/* Physical drive number (0) */
);
#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive number (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
);
#endif




/******************************************************************************
**                            End Of File
******************************************************************************/

/**
 * @}
 */
 
