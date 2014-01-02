/**********************************************************************
* $Id$		Usbhost_fat.h			2011-09-05
*//**
* @file		Usbhost_fat.h
* @brief		Provide APIs to operate on FAT file system.
* @version	1.0
* @date		05. September. 2011
* @author	NXP MCU SW Application Team
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

#ifndef  USBHOST_FAT_H
#define  USBHOST_FAT_H

/*
**************************************************************************************************************
*                                       INCLUDE HEADER FILES
**************************************************************************************************************
*/

#include  "usbhost_inc.h"

/** @addtogroup USBHost_Fat
 * @{
 */

/* Public macros ------------------------------------------------------------- */
/** @defgroup FAT_Macros FAT Macros
 * @{
 */

/*********************************************************************//**
 *  FAT Types
 **********************************************************************/
/**  FAT12 File System */
#define    FAT_12					0
/**  FAT16 File System */
#define    FAT_16                   1
/**  FAT32 File System */
#define    FAT_32					2

/*********************************************************************//**
 *  Entry Types
 **********************************************************************/
/**  The last entry. */
#define    LAST_ENTRY               1
/**  The directory entry is free. */
#define    FREE_ENTRY               2
/**  The long file name entry. */
#define    LFN_ENTRY                3
/**  The short file name entry. */
#define    SFN_ENTRY                4


/**  Entry size  */
#define   FAT_ENTRY_SIZE            32

/*********************************************************************//**
 *  Short Directory Entry Index
 **********************************************************************/
#define   DIR_IDX             0
#define   DIR_ATT_IDX         11
#define   DIR_FST_CLUS_HI_IDX 20
#define   DIR_FST_CLUS_LO_IDX 26
#define   DIR_SIZE_IDX        28
#define   SHORT_FILE_NAME_LEN       11
#define   SHORT_NAME_PART_LEN       8
#define   SHORT_EXTENSION_PART_LEN  3


/*********************************************************************//**
 *  Long Directory Entry Index
 **********************************************************************/
#define   LDIR_ORD_IDX              0
#define   LDIR_NAME_PART1_IDX       1
#define   LDIR_NAME_PART1_END_IDX   (LDIR_NAME_PART1_IDX + 10 - 1)
#define   LDIR_ATT_IDX              11
#define   LDIR_CHK_SUM_IDX          13
#define   LDIR_NAME_PART2_IDX       14
#define   LDIR_NAME_PART2_END_IDX   (LDIR_NAME_PART2_IDX + 12 -1)
#define   LDIR_NAME_PART3_IDX       28
#define   LDIR_NAME_PART3_END_IDX   (LDIR_NAME_PART3_IDX + 4 -1)
#define   LDIR_FILE_NAME_LEN	     13
#define   LONG_FILE_NAME_MAX_LEN    80

/*********************************************************************//**
 *   Entry Attributes
 **********************************************************************/
#define    ATTR_READ_ONLY           0x01
#define    ATTR_HIDDEN              0x02
#define    ATTR_SYSTEM              0x04
#define    ATTR_VOLUME_ID           0x08
#define    ATTR_DIRECTORY           0x10
#define    ATTR_ARCHIVE             0x20
#define    ATTR_LONG_NAME           ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM |ATTR_VOLUME_ID

/*********************************************************************//**
 *   FAT SPECIFIC ERROR CODES
 **********************************************************************/
#define  FAT_FUNC_OK             0
#define  MATCH_FOUND             0
#define  NOT_FOUND         	     -20
#define  ERR_FAT_NOT_SUPPORTED   -21
#define  ERR_OPEN_LIMIT_REACHED  -22
#define  ERR_INVALID_BOOT_SIG    -23
#define  ERR_INVALID_BOOT_SEC    -24
#define  ERR_DIR_FULL            -25
#define  LAST_ENTRY_FOUND		 -26
#define  ERR_INVALID_FILE_NAME	 -27


/**  open file for read only. */
#define    RDONLY                   1
/**  open file for read write. */
#define    RDWR                     2


/**
 * @}
 */


/* Public Types --------------------------------------------------------------- */
/** @defgroup  FAT_Public_Types FAT Public Types
 * @{
 */

 /**
 * @brief BOOT SECTOR Type */
 typedef struct boot_sec {
    uint32_t    BootSecOffset;             /* Offset of the boot sector from sector 0                     */
	uint8_t		OEMName[8];				   /* OEM Name					*/
    uint16_t    BytsPerSec;                /* Bytes per sector                                            */
    uint8_t     SecPerClus;                /* Sectors per cluster                                         */
    uint32_t    BytsPerClus;               /* Bytes per cluster                                           */
    uint16_t    RsvdSecCnt;                /* Reserved sector count                                       */
    uint8_t     NumFATs;                   /* Number of FAT copies                                        */
    uint16_t    RootEntCnt;                /* Root entry count                                            */
    uint16_t    TotSec16;                  /* Total sectors in the disk. !=0 if TotSec32 = 0              */
    uint32_t    TotSec32;                  /* Total sectors in the disk. !=0 if TotSec16 = 0              */
    uint16_t    FATSz16;                   /* Sectors occupied by single FAT table                        */
    uint32_t	FATSz32;				   /* Sectors occupied by ONE FAT.				*/
    uint16_t	ExtFlags;					/* Number of Active FAT		*/
    uint16_t	FSVer;						/* Version number of FAT volume*/
    uint32_t	RootClus;					/* The cluster number of the first cluster of the root directory*/
    uint16_t	FSInfo;						/* Sector number of FSINFO structure */
    uint8_t     FATType;                   /* File system type                                            */
    uint32_t    RootDirSec;                /* Sectors occupied by root directory                          */
    uint32_t    RootDirStartSec;           /* Starting sector of the root directory                       */
    uint32_t    FirstDataSec;              /* Starting sector of the first data cluster                   */
} BOOT_SEC;

/**
 * @brief FILE ENTRY Type */

typedef  struct  file_entry {
    uint32_t  FileSize;                    /* Total size of the file                                      */
    uint8_t   FileAttr;                    /* File attributes                 */
    uint32_t  CurrClus;                    /* Current cluster of the cluster offset                       */
    uint32_t  CurrClusOffset;              /* Current cluster offset                                      */
    uint32_t  EntrySec;                    /* Sector where the file entry is located                      */
    uint32_t  EntrySecOffset;              /* Offset in the entry sector from where the file is located   */
    uint8_t   FileStatus;                  /* File's open status                                          */
} FILE_ENTRY;

typedef struct  {
	uint8_t     name[LONG_FILE_NAME_MAX_LEN];
    FILE_ENTRY  info;
}DIR_ENTRY;

typedef struct  {
	uint32_t entry_num;					/* the number of sub-entries */
	DIR_ENTRY *entries;					/* The list of sub-entries */
	FILE_ENTRY dir_info;				/* file entry for the dir */
} DIR;
/**
 * @}
 */
 


/* Public Functions ----------------------------------------------------------- */
/** @defgroup FAT_Public_Functions FAT Public Functions
 * @{
 */

int32_t  FAT_Init        (void);
uint8_t  FAT_GetFATType (void);
int32_t  FILE_Open(void  *file_name, uint8_t   flags);
uint32_t FILE_Read(int32_t   fd, volatile  uint8_t  *buffer, uint32_t   num_bytes);
uint32_t FILE_Write(int32_t   fd,volatile  uint8_t  *buffer,uint32_t   num_bytes);
void     FILE_Close(int32_t   fd);
int32_t  DIR_Open (uint8_t  *ent_name_given);
DIR_ENTRY*  DIR_ReadEntry (uint32_t index);

/**
 * @}
 */
 /**
 * @}
 */

#endif
