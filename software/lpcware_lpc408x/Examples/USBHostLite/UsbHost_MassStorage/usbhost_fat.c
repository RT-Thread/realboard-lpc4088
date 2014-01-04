/**********************************************************************
* $Id$      usbhost_fat.c           2011-09-05
*//**
* @file     usbhost_fat.c
* @brief        Provide APIs to operate on FAT file system.
* @version  1.0
* @date     05. September. 2011
* @author   NXP MCU SW Application Team
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

/*
**************************************************************************************************************
*                                           INCLUDE HEADER FILES
**************************************************************************************************************
*/

#include  "usbhost_fat.h"

/** @addtogroup USBHost_Fat
 * @{
 */


/*
**************************************************************************************************************
*                                              Definition, constants
**************************************************************************************************************
*/
                                                                                  // Get the size of a FAT
#define GET_FAT_SIZE()      ((FAT_BootSec.FATType == FAT_32) ? FAT_BootSec.FATSz32 : FAT_BootSec.FATSz16)
                
                                                                                // Get the size of an entry
#define GET_FAT_ENT_SIZE()  ((FAT_BootSec.FATType == FAT_32) ? 4 : 2)           
                                                                                // Get the offset in FAT of the entry  
                                                                                //which describes the given cluster
#define GET_FAT_OFFSET(ClusNum) ((FAT_BootSec.FATType == FAT_32) ? (ClusNum*4): (ClusNum*2))        
                                                
                                                                                // Get the sector containing the entry 
#define GET_FAT_SEC_NUM(FATOffset)  (FAT_BootSec.RsvdSecCnt +  (FATOffset/FAT_BootSec.BytsPerSec))

                                                                                // Get the offset of the entry
#define GET_FAT_ENT_OFFSET(FATOffset)   (FATOffset%FAT_BootSec.BytsPerSec)      

                                                                                // The maximum number of openned.
#define MAX_FILE_DESCRIPTORS            2   
#define MAX_SUB_LEN         3
#define MAX_NAME_LEN            80
#define MAX_ENTRY_NUM           128
DIR_ENTRY DIR_Buffer[MAX_ENTRY_NUM];
static uint8_t  name_tmp[LONG_FILE_NAME_MAX_LEN];

/*
**************************************************************************************************************
*                                              GLOBAL VARIABLES
**************************************************************************************************************
*/

static  BOOT_SEC    FAT_BootSec;                                                // Store information in the boot sector.
static  FILE_ENTRY  FAT_FileEntry[MAX_FILE_DESCRIPTORS];                        // Store information of files which are being openned.
static  DIR         FAT_CurDirEntry;                                            // Store information of current dir
static  uint8_t     g_filePath[MAX_SUB_LEN][MAX_NAME_LEN];                      // Buffer to store file path after analyzing
static  uint8_t     g_fileSubLevel;

/*
**************************************************************************************************************
*                                             Function declartions
**************************************************************************************************************
*/
int32_t  FAT_FindEntry( uint8_t  *ent_name_given, FILE_ENTRY  *entry);
int32_t  FAT_FindEntryInSector(uint32_t* sec_num, uint8_t  *ent_name_given,FILE_ENTRY  *entry);
void     FAT_GetSFN(volatile  uint8_t  *entry, uint8_t  *name);
void     FAT_GetSfnName(volatile  uint8_t  *entry, uint8_t  *name);
void     FAT_GetSfnExt   (volatile  uint8_t  *entry, uint8_t  *ext_ptr);
uint8_t  FAT_GetLfn (volatile  uint8_t  **entry, uint32_t* sec_num, volatile uint8_t * start_buf, volatile uint8_t * end_buf, uint8_t  *name);
uint8_t  FAT_LFN_ChkSum (uint8_t* name);
int32_t  FAT_StrCaseCmp(uint8_t  *str1, uint8_t  *str2);
uint8_t  FAT_GetStringLen(uint8_t* str);
int32_t  FAT_GetFilePath( uint8_t  *name);
uint32_t FAT_ChkEntType  (volatile  uint8_t  *ent);
uint32_t FAT_ClusRead( uint32_t   curr_clus, uint32_t clus_offset,volatile  uint8_t  *buffer, uint32_t   num_bytes);
uint32_t FAT_GetNextClus (uint32_t   clus_no);
uint32_t FAT_ClusWrite(uint32_t   curr_clus,uint32_t   clus_offset,
                             volatile  uint8_t  *buffer,uint32_t   num_bytes);
void     FAT_UpdateEntry (FILE_ENTRY  *entry);
void     FAT_UpdateFileSize (FILE_ENTRY  *entry);
void     FAT_UpdateFAT   (uint32_t   curr_clus,uint32_t   value);
uint32_t FAT_GetFreeClus (void);
int32_t  FAT_FindFreeEntry(uint32_t clus_no, FILE_ENTRY  *entry);
int32_t  FAT_FindFreeEntry (uint32_t sec_num,FILE_ENTRY  *entry);
void     FAT_PutSFN(uint8_t  *ent_name_given, FILE_ENTRY  *entry);
void     FAT_PutLFN (uint8_t  *ent_name_given, FILE_ENTRY  *entry);

int32_t  FAT_CreateEntry (uint8_t  *ent_name_given,FILE_ENTRY  *entry);
uint32_t FAT_GetEndClus  (uint32_t   clus_no);



/*********************************************************************//**
 * @brief           Read the boot sector and initialize global variables
 * @param[in]       None
 * @return      Return code
 *                               - ERR_INVALID_BOOT_SIG
 *                               - ERR_FAT_NOT_SUPPORTED
 *                    - FAT_FUNC_OK
 **********************************************************************/
int32_t  FAT_Init (void)
{
    uint16_t  boot_sig;
    int32_t  rc, i;
    FILE_ENTRY  *entry;

    // Initialize FAT entry table
    FAT_CurDirEntry.entry_num = 0;
    FAT_CurDirEntry.entries = DIR_Buffer;
    entry = &FAT_CurDirEntry.dir_info;
    
    entry->CurrClus       = 0;
    entry->CurrClusOffset = 0;
    entry->FileSize       = 0;
    entry->EntrySec       = 0;
    entry->EntrySecOffset = 0;
    entry->FileStatus     = 0;
        
    for (i=0;i<MAX_FILE_DESCRIPTORS;i++) {
        entry = &FAT_FileEntry[i];
        entry->CurrClus       = 0;
        entry->CurrClusOffset = 0;
        entry->FileSize       = 0;
        entry->EntrySec       = 0;
        entry->EntrySecOffset = 0;
        entry->FileStatus     = 0;
    }

    /*  Read Boot Sector    */
    MS_BulkRecv(0, 1, FATBuffer);
    
    boot_sig = ReadLE16U(&FATBuffer[510]);
    if (boot_sig != 0xAA55) {
        rc = ERR_INVALID_BOOT_SIG;
    } else {
    
        if (FATBuffer[0] != 0xEB && FATBuffer[0] != 0xE9) {
            FAT_BootSec.BootSecOffset = ReadLE32U(&FATBuffer[454]);
            MS_BulkRecv(FAT_BootSec.BootSecOffset, 1, FATBuffer);
        }
        
        for(i = 0; i< 8; i++) {
            FAT_BootSec.OEMName[i] =  FATBuffer[3 + i];
        }
        
        FAT_BootSec.BytsPerSec      = ReadLE16U(&FATBuffer[11]);          /* Bytes per sector              */
        FAT_BootSec.SecPerClus      = FATBuffer[13];                      /* Sectors per cluster            */
                                                                          /* Reserved sector count          */
        FAT_BootSec.RsvdSecCnt      = ReadLE16U(&FATBuffer[14]) + FAT_BootSec.BootSecOffset;
        FAT_BootSec.NumFATs         = FATBuffer[16];                      /* Number of FAT copies           */
        FAT_BootSec.RootEntCnt      = ReadLE16U(&FATBuffer[17]);          /* Root entry count               */
        FAT_BootSec.TotSec16        = ReadLE16U(&FATBuffer[19]);          /* Total FAT16 sectors            */
        FAT_BootSec.TotSec32        = ReadLE32U(&FATBuffer[32]);          /* Total FAT32 sectors            */  
                                                                          /* Bytes per cluster              */
        FAT_BootSec.BytsPerClus     = (FAT_BootSec.BytsPerSec * FAT_BootSec.SecPerClus);
                                                                          /* Root directory starting sector */
        FAT_BootSec.FATType         = FAT_GetFATType();                   /* Type of FAT                    */

        
                                                                          /* Size of the FAT table      */
        if((FAT_BootSec.FATType == FAT_12)||(FAT_BootSec.FATType == FAT_16)) {
            
            FAT_BootSec.FATSz16         = ReadLE16U(&FATBuffer[22]);          /* Size of the FAT table (FAT16)*/
    
        } else {
            FAT_BootSec.FATSz32         = ReadLE32U(&FATBuffer[36]);          /* Size of the FAT table (FAT32)*/
            FAT_BootSec.ExtFlags        = ReadLE16U(&FATBuffer[40]);
            FAT_BootSec.FSVer           = ReadLE16U(&FATBuffer[42]);        /* the version number of the FAT32 volume*/
            FAT_BootSec.RootClus        = ReadLE32U(&FATBuffer[44]);        /*The cluster number of the root directory*/
            FAT_BootSec.FSInfo          = ReadLE16U(&FATBuffer[48]);        /*Sector number of FSINFO structure*/
                
        }
                                                                            /* The first sector of root directory*/
        FAT_BootSec.RootDirStartSec = FAT_BootSec.RsvdSecCnt + (GET_FAT_SIZE() * FAT_BootSec.NumFATs);
        
                                                                      /* Sectors occupied by root directory */
        FAT_BootSec.RootDirSec      = ((FAT_BootSec.RootEntCnt * 32) + (FAT_BootSec.BytsPerSec - 1)) /
                                       (FAT_BootSec.BytsPerSec);        /* used for FAT16, FAT32 is always 0*/
                                                                          /* First data sector              */
        FAT_BootSec.FirstDataSec    = FAT_BootSec.RootDirStartSec + FAT_BootSec.RootDirSec;
    
        rc = FAT_FUNC_OK;
        
        if (FAT_BootSec.FATType == FAT_16) {
            
        } else if (FAT_BootSec.FATType == FAT_32) {
    
        } else {
            rc = ERR_FAT_NOT_SUPPORTED;
        }   
    }
    return (rc);
}

/*********************************************************************//**
 * @brief           Get the file system type with which the disk is formatted.
 * @param[in]       None.
 * @return      The system type.
 *                               - FAT_12
 *                               - FAT_16
 *                               - FAT_32
 **********************************************************************/
uint8_t  FAT_GetFATType (void)
{
    uint8_t  fat_type;
    uint32_t  tot_sec;
    uint32_t  data_sec;
    uint32_t  count_of_cluster;


    if (FAT_BootSec.TotSec16 != 0) {                             /* Get total sectors in the disk           */
        tot_sec = FAT_BootSec.TotSec16;
    } else {
        tot_sec = FAT_BootSec.TotSec32;
    }

    data_sec = tot_sec - FAT_BootSec.FirstDataSec;
    
    count_of_cluster = data_sec / (FAT_BootSec.SecPerClus);          /* Get total data clusters in the disk     */
                                                                 /* If total data clusters >= 4085 and      */
                                                                 /* < 65525, then it is FAT16 file system   */
    if(count_of_cluster < 4085) {
    fat_type = FAT_12;
    } else if (count_of_cluster < 65525) {
        fat_type = FAT_16;
    } else {
        fat_type = FAT_32;
    } 

    return (fat_type);
}

/*
**************************************************************************************************************
*                                              FATs Processing
**************************************************************************************************************
*/

/*********************************************************************//**
 * @brief           Get the first sector of the given cluster.
 * @param[in]       clus_no  The cluster number
 * @return      The sector number.
 **********************************************************************/
uint32_t FAT_GetFirstSectorOfCluster(uint32_t clus_no)
{
    return (clus_no-2) * FAT_BootSec.SecPerClus + FAT_BootSec.FirstDataSec;
}


/*********************************************************************//**
 * @brief           Read the entry of given cluster in FAT.
 * @param[in]       clus_no  The cluster number
 * @return      The entry value.
 **********************************************************************/
uint32_t FAT_ReadFAT(uint32_t clus_no)
{       
    uint32_t    FATOff, FATSecNum, FATEntryOff;
    uint32_t    FATClusEntryVal;
        
    // Get FAT entry    
    FATOff = GET_FAT_OFFSET( clus_no);  
    FATSecNum = GET_FAT_SEC_NUM(FATOff);    
    FATEntryOff = GET_FAT_ENT_OFFSET(FATOff);   
        
    // Read the FAT Sector  for the cluster
    MS_BulkRecv(FATSecNum, 1, FATBuffer);   
    if(FAT_BootSec.FATType == FAT_32)   
        FATClusEntryVal = (*((uint32_t *) &FATBuffer[FATEntryOff])) & 0x0FFFFFFF;
    else    
        FATClusEntryVal = *((uint16_t *) &FATBuffer[FATEntryOff]);
        
    return FATClusEntryVal; 
}   

/*********************************************************************//**
 * @brief           Get the next cluster of the current cluster in the cluster chain. If the current
*              cluster is the last cluster then this function returns 0
 * @param[in]       clus_no    The cluster number for which the next cluster to be found
 * @return      next_clus  if the current cluster is not the last cluster
 *               0          if the current cluster is the last cluster
 *                         Note: In practical cluster number 0 doesn't exist
 **********************************************************************/
uint32_t  FAT_GetNextClus (uint32_t  clus_no)
{
    uint32_t FATEntValue;
    uint32_t  next_clus;
    
    FATEntValue = FAT_ReadFAT(clus_no);
    next_clus = FATEntValue;                /* Read the next cluster                   */
                                                        /* Check EOC */
    if(((FAT_BootSec.FATType == FAT_12)&& (next_clus >= 0x0FF8)) ||
        ((FAT_BootSec.FATType == FAT_16)&& (next_clus >= 0xFFF8)) ||
        ((FAT_BootSec.FATType == FAT_32)&& (next_clus >= 0x0FFFFFF8)))
    {      
        next_clus = 0;                                     /* Current cluster is the end cluster            */
    }
    return (next_clus);
}

/*********************************************************************//**
 * @brief           Get the end cluster in the cluster chain of a cluster
 * @param[in]       clus_no    Starting cluster of the cluster chain in which end cluster to be found
 * @return      End cluster in the cluster chain
 **********************************************************************/
uint32_t  FAT_GetEndClus (uint32_t  clus_no)
{
    uint32_t  next_clus;


    next_clus = clus_no;
    while (next_clus) {
        next_clus = FAT_GetNextClus(clus_no);
        if (next_clus) {
            clus_no = next_clus;
        }
    }
    return (clus_no);
}

/*********************************************************************//**
 * @brief           Get the the free cluster if available
 * @param[in]       None.
 * @return      free_clus    if available
 *                         0            if not available(means the disk is full)
 **********************************************************************/
uint32_t  FAT_GetFreeClus (void)
{
    uint32_t  num_sec;
    uint32_t  cnt;
    uint32_t  sec_num;
    uint32_t  free_clus;
    uint8_t   entry_size = GET_FAT_ENT_SIZE();
    uint32_t  entry_val;


    sec_num = FAT_BootSec.RsvdSecCnt;
    num_sec = GET_FAT_SIZE()* FAT_BootSec.NumFATs;
    while (sec_num < (FAT_BootSec.RsvdSecCnt + num_sec)) {
        MS_BulkRecv(sec_num, 1, FATBuffer);
        for (cnt = 0; cnt < FAT_BootSec.BytsPerSec; cnt += entry_size) {
            entry_val = ReadLE32U(&FATBuffer[cnt]) & 0x0FFFFFFF;
            if (entry_val == 0) {
                free_clus = (((sec_num - FAT_BootSec.RsvdSecCnt) * FAT_BootSec.BytsPerSec) + cnt) / entry_size;
                return (free_clus);
            }
        }
        sec_num++;
    }
    return (0);
}


/*********************************************************************//**
 * @brief           Update a entry in FAT.
 * @param[in]       clus_no  The cluster number
 * @return      None.
 **********************************************************************/
void  FAT_UpdateFAT (uint32_t  curr_clus,
                     uint32_t  value)
{
    uint32_t    FATOff, FATSecNum, FATEntryOff;
    uint32_t    FATClusEntryVal;
        
    // Get FAT entry    
    FATOff = GET_FAT_OFFSET( curr_clus);    
    FATSecNum = GET_FAT_SEC_NUM(FATOff);    
    FATEntryOff = GET_FAT_ENT_OFFSET(FATOff);

    // Read entry
    MS_BulkRecv(FATSecNum, 1, FATBuffer);

    // Update value
    if(FAT_BootSec.FATType == FAT_32)  {
    FATClusEntryVal = FATBuffer[FATEntryOff];
    FATClusEntryVal = FATClusEntryVal & 0xF0000000;
    value = value & 0x0FFFFFFF;
    FATClusEntryVal |= value;
    WriteLE32U(&FATBuffer[FATEntryOff], FATClusEntryVal);
    } else
    WriteLE16U(&FATBuffer[FATEntryOff], value);
    
    MS_BulkSend(FATSecNum, 1, FATBuffer);
}


/*
**************************************************************************************************************
*                                              Root Directory Processing
**************************************************************************************************************
*/

/*********************************************************************//**
 * @brief           Find the entry of the given file in FAT file system.
 * @param[in]       ent_name_given         File name.
 * @param[in]       entry         point to the  buffer to store entry information
 * @return      Return code.
 *                               - MATCH_FOUND
 *                               - LAST_ENTRY_FOUND
 *                               - NOT_FOUND
 **********************************************************************/
int32_t  FAT_FindEntry (uint8_t  *ent_name_given,
                           FILE_ENTRY  *entry)
{
    uint32_t   first_sec_num, end_sec_num, sec_num, clus_no;
    int32_t   ret_code; 
    uint8_t    sub_folder_cnt = 0;
    
    if(ent_name_given)
        FAT_GetFilePath(ent_name_given);

    if(FAT_BootSec.FATType == FAT_32) {
        clus_no =  FAT_BootSec.RootClus;
        first_sec_num = FAT_GetFirstSectorOfCluster(clus_no);
        end_sec_num = first_sec_num + FAT_BootSec.SecPerClus;
    } else {
        clus_no =  FAT_BootSec.RootDirStartSec/FAT_BootSec.SecPerClus;
        first_sec_num = FAT_BootSec.RootDirStartSec;
        end_sec_num = FAT_BootSec.RootDirStartSec + FAT_BootSec.RootDirSec;
    }
    
    while(1)
   {
        for(sec_num = first_sec_num;
                sec_num < end_sec_num;
                sec_num++)
        {
            ret_code = FAT_FindEntryInSector(&sec_num, (uint8_t*)g_filePath[sub_folder_cnt], entry);
            if(ret_code == FAT_FUNC_OK && sub_folder_cnt < g_fileSubLevel) { // Found a folder
                sub_folder_cnt++;
                break;
            } else if(ret_code != NOT_FOUND) {
                return ret_code;
            }
        }

        if(ret_code == FAT_FUNC_OK) {
           clus_no = entry->CurrClus;
        } else {
           clus_no = FAT_GetNextClus(clus_no);
        }
        
        if(clus_no == 0)
            break;
        
        first_sec_num = FAT_GetFirstSectorOfCluster(clus_no);
        end_sec_num = first_sec_num + FAT_BootSec.SecPerClus;
    }
    return (NOT_FOUND);
}


/*********************************************************************//**
 * @brief           Find the entry of the given file in the given sector.
 * @param[in]       sec_num         Sector number.
 * @param[in]       ent_name_given         File name.
 * @param[in]       entry         point to the  buffer to store entry information
 * @param[in]       is_dir        search for a file or a directory
 * @return      Return code.
 *                               - MATCH_FOUND
 *                               - LAST_ENTRY_FOUND
 *                               - NOT_FOUND
 **********************************************************************/
int32_t FAT_FindEntryInSector(uint32_t* sec_num, uint8_t  *ent_name_given,
                        FILE_ENTRY  *entry)
{
    uint8_t   ent_type;
    uint8_t   ent_name_read[80];
    uint8_t   len = 0;
    uint8_t   sum = 0;
    volatile uint8_t    *buf;

     MS_BulkRecv(*sec_num, 1, FATBuffer);                  /* Read one sector                             */
     buf = FATBuffer;
     while (buf < (FATBuffer + FAT_BootSec.BytsPerSec)) {
            ent_type = FAT_ChkEntType(buf);                  /* Check for the entry type                    */
            if (ent_type == SFN_ENTRY) {                     /* If it is short entry get short file name    */
                 FAT_GetSFN(buf, ent_name_read);
                                                    /* Compare given name with this name case insensitively */
                if (FAT_StrCaseCmp(ent_name_given, ent_name_read) == MATCH_FOUND) {
                    entry->CurrClus = ReadLE16U(&buf[DIR_FST_CLUS_HI_IDX])<<16 & 0xFFFF0000;    /*  High word of the first cluster number*/
                    entry->CurrClus |= ReadLE16U(&buf[DIR_FST_CLUS_LO_IDX]);   /* Low word of the first cluster number     */
                    entry->CurrClusOffset = 0;
                    entry->FileSize  = ReadLE32U(&buf[DIR_SIZE_IDX]);  /* Get file size                               */   
                    entry->EntrySec = *sec_num;           /* Get sector number where the filename is located */
                    entry->EntrySecOffset = buf - FATBuffer; /* Get offset in this sector where the filename is located */
                    return (MATCH_FOUND);
                }
            } else if (ent_type == LFN_ENTRY) {
                sum = buf[LDIR_CHK_SUM_IDX];
                len = FAT_GetLfn(&buf,sec_num,FATBuffer,FATBuffer+FAT_BootSec.BytsPerSec,ent_name_read);
                if(len > 0) {
                    if (FAT_StrCaseCmp(ent_name_given, ent_name_read) == MATCH_FOUND) {
                        FAT_GetSFN(buf, ent_name_read);
                        ent_type = FAT_ChkEntType(buf);
                        if(ent_type == SFN_ENTRY && sum == FAT_LFN_ChkSum((uint8_t*)buf))
                        {
                            entry->CurrClus = ReadLE16U(&buf[DIR_FST_CLUS_HI_IDX])<<16 & 0xFFFF0000;    /*  High word of the first cluster number*/
                            entry->CurrClus |= ReadLE16U(&buf[DIR_FST_CLUS_LO_IDX]);   /* Low word of the first cluster number     */
                            entry->CurrClusOffset = 0;
                            entry->FileSize  = ReadLE32U(&buf[DIR_SIZE_IDX]);  /* Get file size                               */   
                            entry->EntrySec = *sec_num;           /* Get sector number where the filename is located */
                            entry->EntrySecOffset = buf - FATBuffer; /* Get offset in this sector where the filename is located */
                            return (MATCH_FOUND);
                        } else {
                            buf -= FAT_ENTRY_SIZE;
                        }
                    }
                }
            }
            if (ent_type == LAST_ENTRY) {    /* If it is the last entry, no more entries will exist. Return */
                return (LAST_ENTRY_FOUND);
            }
            buf = buf + FAT_ENTRY_SIZE;                                  /* Move to the next entry                      */
        }
     return NOT_FOUND;
}

/*********************************************************************//**
 * @brief           Find a free entry in the given sector of FAT file system.  If a free 
 *     entry is found, the sector number and sector offset where the entry is located will be stored
 * @param[in]       entry         point to the  buffer to store entry information
 * @param[in]       clus_no     Cluster number of parent directory. If it equals 0, find in root
 *                                         directory.
 * @return      Return code.
 *                               - FAT_FUNC_OK
 *                               - LAST_ENTRY_FOUND
 *                               - NOT_FOUND
 *                    - ERR_DIR_FULL
 **********************************************************************/
int32_t  FAT_FindFreeEntry (uint32_t clus_no, FILE_ENTRY  *entry)
{
    uint32_t   first_sec_num, end_sec_num, sec_num;
    int32_t   ret_code; 
    volatile  uint8_t  *buf;
    uint8_t   ent_type;

    if(clus_no == 0) {  // Find in root directory
        if(FAT_BootSec.FATType == FAT_32) {
            first_sec_num = FAT_GetFirstSectorOfCluster(FAT_BootSec.RootClus);
            end_sec_num = first_sec_num + FAT_BootSec.SecPerClus;
        } else {
            first_sec_num = FAT_BootSec.RootDirStartSec;
            end_sec_num = FAT_BootSec.RootDirStartSec + FAT_BootSec.RootDirSec;
        }
    } else {
        first_sec_num = FAT_GetFirstSectorOfCluster(clus_no);
        end_sec_num = first_sec_num + FAT_BootSec.SecPerClus;
    }
    
    for(sec_num = first_sec_num;
        sec_num < end_sec_num;
        sec_num++)
    {
         ret_code = NOT_FOUND;
         MS_BulkRecv(sec_num, 1, FATBuffer);
         buf = FATBuffer;
         while (buf < (FATBuffer + FAT_BootSec.BytsPerSec)) {
            ent_type = FAT_ChkEntType(buf);
            if (ent_type == FREE_ENTRY || ent_type == LAST_ENTRY) {
                entry->EntrySec       = sec_num;
                entry->EntrySecOffset = buf - FATBuffer;
                ret_code = FAT_FUNC_OK;
                break;
            }
            buf += 32;
         }
         if(ret_code != NOT_FOUND)
         {
            return ret_code;
         }
    }
    
    return (ERR_DIR_FULL);
}

/*********************************************************************//**
 * @brief           creates a file entry in the root directory if the file does not exist.
 * @param[in]       ent_name_given    The file name with which the entry is to be created
 * @param[in]        entry             Pointer to FILE ENTRY structure
 * @return      Return code.
 *                               - FAT_FUNC_OK
 *                               - MATCH_FOUND
 *                               - LAST_ENTRY_FOUND
 *                               - NOT_FOUND
 *                    - ERR_DIR_FULL
 **********************************************************************/
int32_t  FAT_CreateEntry (uint8_t  *ent_name_given,
                             FILE_ENTRY  *entry)
{
    int32_t  rc = 0;
    uint8_t  file_sub_level;

    FAT_GetFilePath(ent_name_given);
    file_sub_level = g_fileSubLevel;
    
    rc = FAT_FindEntry(ent_name_given, entry);        /* Find for the given file name in the root directory */
    if (rc == MATCH_FOUND) {                          /* If match found, return                             */
        return (rc);
    } else {
        if(file_sub_level > 0 ) {
            g_fileSubLevel -= 1;
            rc = FAT_FindEntry(NULL, entry);        /* Find the parent folder */            
            if(rc != MATCH_FOUND) 
                return NOT_FOUND;
            g_fileSubLevel += 1;
            
            rc = FAT_FindFreeEntry(entry->CurrClus, entry);
            if (rc != FAT_FUNC_OK) {
                return (rc);
            } 
                            
        } else {
            rc = FAT_FindFreeEntry(0, entry); 
            if (rc != FAT_FUNC_OK) {
                return (rc);
            } 
        }
        if(FAT_GetStringLen(g_filePath[g_fileSubLevel]) <= SHORT_FILE_NAME_LEN)
           FAT_PutSFN(g_filePath[g_fileSubLevel], entry);        /* Store the given short file name in that entry      */
        else
           FAT_PutLFN(g_filePath[g_fileSubLevel], entry);
        return (rc);
    }
}

/*********************************************************************//**
 * @brief           updates the file entry that is located in the root directory.
 * @param[in]       entry    Pointer to the FILE ENTRY structure which contains the information about the file
 * @return      None.
 **********************************************************************/
void  FAT_UpdateEntry (FILE_ENTRY  *entry)
{
    uint32_t  sec_num;
    uint32_t  offset;
    
    sec_num = entry->EntrySec;
    offset  = entry->EntrySecOffset;
    // Read the entry
    MS_BulkRecv(sec_num, 1, FATBuffer);

    // Write the cluster number
    WriteLE16U(&FATBuffer[offset + DIR_FST_CLUS_HI_IDX], entry->CurrClus>>16);
    WriteLE16U(&FATBuffer[offset + DIR_FST_CLUS_LO_IDX], entry->CurrClus & 0x0000FFFF);

    // Write the file size
    WriteLE32U(&FATBuffer[offset + DIR_SIZE_IDX], entry->FileSize);
    MS_BulkSend(sec_num, 1, FATBuffer);
}


/*********************************************************************//**
 * @brief           updates the file size into the file entry that is located in the root directory.
 * @param[in]       entry    Pointer to the FILE ENTRY structure which contains the information about the file
 * @return      None.
 **********************************************************************/
void  FAT_UpdateFileSize (FILE_ENTRY  *entry)
{
    uint32_t  sec_num;
    uint32_t  offset;
    
    sec_num = entry->EntrySec;
    offset  = entry->EntrySecOffset;
    // Read the entry
    MS_BulkRecv(sec_num, 1, FATBuffer);

    // Write the file size
    WriteLE32U(&FATBuffer[offset + DIR_SIZE_IDX], entry->FileSize);
    MS_BulkSend(sec_num, 1, FATBuffer);
}

/*********************************************************************//**
 * @brief           Reads the short file name and extension corresponding to a file.
 * @param[in]       entry    buffer which contains the 32 byte entry of a file
 * @param[in]         name       buffer to store the file name and extension of a file
 * @return      None.
 **********************************************************************/
void  FAT_GetSFN (volatile  uint8_t  *entry,
                            uint8_t  *name)
{
    uint8_t   ext[4];                          /* Buffer to store the extension of a file                */
    uint8_t  *ext_ptr;


    ext_ptr = ext;

    FAT_GetSfnName(entry, name);                  /* Get file name into "name" buffer                       */
    FAT_GetSfnExt(entry, ext_ptr);                /* Get extension into "ext" buffer                        */

    while (*name) {                               /* Goto the end of the filename                           */
        name++;
    }
    if (*ext_ptr) {                               /* If the extension exists, put a '.' charecter           */
        *name = '.';
        name++;
    }
    while (*ext_ptr) {                            /* Append the extension to the file name                  */
        *name = *ext_ptr;
        name++;
        ext_ptr++;
    }
    *name = '\0';
}

/*********************************************************************//**
 * @brief           Reads the short file name of a file.
 * @param[in]       entry    buffer which contains the 32 byte entry of a file
 * @param[in]         name       buffer to store the file name and extension of a file
 * @return      None.
 **********************************************************************/
void  FAT_GetSfnName (volatile  uint8_t  *entry,
                                uint8_t  *name)
{
    uint32_t  cnt;


    cnt = 0;
    while (cnt < SHORT_NAME_PART_LEN) {
        *name = *entry;                     /* Get first 8 charecters of an SFN entry                       */
        name++;
        entry++;
        cnt++;
    }
    *name = 0;
    name--;
    while (*name == 0x20) {                 /* If any spaces exist after the file name, replace them with 0 */
        *name = 0;
        name--;
    }
}

/*********************************************************************//**
 * @brief           Reads the long file name .
 * @param[in]       entry    buffer which contains the 32 byte entry of a file
 * @param[in]       name       buffer to store the file name and extension of a file
 * @return      the number bytes used for storing the long file name.
 **********************************************************************/
 uint8_t  FAT_GetLfn (volatile  uint8_t  **entry, uint32_t* sec_num, volatile uint8_t * start_buf, volatile uint8_t * end_buf,
                                uint8_t  *name)
{
    uint32_t  cnt, num_bytes = 0;
    volatile uint8_t *buf = *entry;
    uint8_t   total_len = 0,end_string;
    int8_t i = 0, j = 0, end_part;
    uint8_t  sum = 0;
    uint8_t part_no = 0, ord;

    for(i = 0; i < LONG_FILE_NAME_MAX_LEN; i++)
        name_tmp[i] = 0;

    sum = buf[LDIR_CHK_SUM_IDX];
    ord = buf[LDIR_ORD_IDX];
    
    // deleted LFN?
    if(ord&0x80)
        return 0;

    // Not Last LFN?
    if((ord&0x40) == 0)
        return 0;
    
    part_no = ord & 0x1F;
    while (buf[LDIR_ATT_IDX] == 0x0F )  // Long directory entry
    {
        end_string = 0;
        
        if(sum != buf[LDIR_CHK_SUM_IDX])
            return 0;

        // Check LFN number
        ord = buf[LDIR_ORD_IDX];
        if((ord&0x1F) != part_no)
            return 0;
        part_no--;
        
        // Get characters 1-5
        for(cnt = LDIR_NAME_PART1_IDX; 
            cnt <= LDIR_NAME_PART1_END_IDX; cnt+=2) {
            if(buf[cnt] == 0) {
                end_string = 1;
                break;
            }
            name_tmp[total_len++] = buf[cnt];
        }
        // Get characters 6-11
        for(cnt = LDIR_NAME_PART2_IDX; 
            (cnt <= LDIR_NAME_PART2_END_IDX && end_string == 0); cnt+=2) {
            if(buf[cnt] == 0) {
                end_string = 1;
                break;
            }
            name_tmp[total_len++] = buf[cnt];
        }
        // Get characters 12-13
        for(cnt = LDIR_NAME_PART3_IDX; 
            (cnt <= LDIR_NAME_PART3_END_IDX && end_string == 0); cnt+=2) {
            if(buf[cnt] == 0) {
                end_string = 1;
                break;
            }
            name_tmp[total_len++] = buf[cnt];
        }
        
        name_tmp[total_len++] = 0;

        // Get the next part
        buf +=FAT_ENTRY_SIZE;
        num_bytes += FAT_ENTRY_SIZE;

        if(end_buf <= buf)
        {
            *entry = start_buf;
            buf = start_buf;
            *sec_num = *sec_num + 1;
            MS_BulkRecv(*sec_num, 1, start_buf); 
        }
    }
    
    // Revert parts of file name
    if(total_len > 0) {
        
        end_part = total_len;
        for(i = total_len - 1; i >= 0; i--) {

            // End of a part of file name?
            if(name_tmp[i] == 0 || i==0 ) {
                if(i == 0) {
                    j = i;
                } else {
                    j = i + 1;
                }
                
                // Copy the part to destination
                for(; j < end_part; j++) {
                    if(name_tmp[j]) {
                        *name = name_tmp[j];
                        name++;
                    }
                }
                
                end_part = i;
            }
        }
        *name = 0;
        
    }
    *entry = buf;
    return num_bytes;
}

/*********************************************************************//**
 * @brief           Reads the extension of a file.
 * @param[in]       entry    buffer which contains the 32 byte entry of a file
 * @param[in]         ext_ptr       buffer to store the extension of a file
 * @return      None.
 **********************************************************************/
void  FAT_GetSfnExt (volatile  uint8_t  *entry,
                               uint8_t  *ext_ptr)
{
    uint32_t  cnt;


    cnt = 0;
    while (cnt < SHORT_NAME_PART_LEN) {                  /* Goto the beginning of the file extension                          */
        entry++;
        cnt++;
    }
    cnt = 0;
    while (cnt < SHORT_EXTENSION_PART_LEN) {                  /* Get 3 charecters from there                                       */
        *ext_ptr = *entry;
        ext_ptr++;
        entry++;
        cnt++;
    }
    *ext_ptr = 0;
    ext_ptr--;
    while (*ext_ptr == ' ') {          /* If any spaces exist after the file extension, replace them with 0 */
        *ext_ptr = 0;
        ext_ptr--;
    }
}

/*********************************************************************//**
 * @brief           Compares two strings case insensitively.
 * @param[in]       str1               Pointer to the first string
 * @param[in]       str2               Pointer to the second string
 * @return      NOT_FOUND       if both the strings are different
 *                         MATCH_FOUND   if both the strings are same
 **********************************************************************/
int32_t  FAT_StrCaseCmp (uint8_t  *str1,
                            uint8_t  *str2)
{
    while (*str1 && *str2) {
        if (*str1 == *str2 || *str1 == (*str2 + 32) || *str1 == (*str2 - 32)) {
            str1++;
            str2++;
            continue;
        } else {
            return (NOT_FOUND);
        }
    }
    if (*str1 == 0 && *str2 == 0) {
        return (MATCH_FOUND);
    } else {
        return (NOT_FOUND);
    }
}
/*********************************************************************//**
 * @brief           Copy str2 to str1.
 * @param[in]       str1               Pointer to the first string
 * @param[in]       str2               Pointer to the second string
 * @return      len
 **********************************************************************/
int32_t  FAT_StrCopy (uint8_t  *str1,
                            uint8_t  *str2)
{
    uint32_t len = 0;
    while (*str2) {
        *str1++ = *str2++;
        len++;
    }
    *str1 = 0;
    return len;
}


/*********************************************************************//**
 * @brief           Fills the file entry with the short file name given by the user.
 * @param[in]       ent_name_given    File name given by the user
 * @param[in]       entry             Pointer to the FILE_ENTRY structure
 * @return      None.
 **********************************************************************/
void  FAT_PutSFN (uint8_t  *ent_name_given,
                  FILE_ENTRY  *entry)
{
    uint32_t  idx;

                                           /* Read the sector from the directory containing the free entry */
    MS_BulkRecv(entry->EntrySec, 1, FATBuffer);
    for (idx = 0; idx < 8; idx++) {        /* Fill the first eight charecters of the entry with file name   */
        if (*ent_name_given == '.') {
            while (idx < 8) {
                FATBuffer[entry->EntrySecOffset + idx] = 0x20;
                idx++;
            }
            ent_name_given++;
        } else {
            FATBuffer[entry->EntrySecOffset + idx] = *ent_name_given;
            ent_name_given++;
        }
    }

    for (idx = 8; idx < 11; idx++) {                      /* Fill the next 3 charecters with file extension */
        if (*ent_name_given == '.') {
            while (idx < 11) {
                FATBuffer[entry->EntrySecOffset + idx] = 0x20;
                idx++;
            }
        } else {
            FATBuffer[entry->EntrySecOffset + idx] = *ent_name_given;
            ent_name_given++;
        }
    }
    FATBuffer[entry->EntrySecOffset + idx] = 0x20;
    for (idx = 12; idx < 32; idx++) {                           /* Fill all the remaining bytes with 0's    */
        FATBuffer[entry->EntrySecOffset + idx] = 0;
    }
    MS_BulkSend(entry->EntrySec, 1, FATBuffer);                 /* Write the sector into the root directory */
}
/*********************************************************************//**
 * @brief           Get the basic name from long file name.
 * @param[in]       ent_name_given    buffer to store long file name
 * @param[in]       short_name           buffer to store basic name
 * @return      None.
 **********************************************************************/
void FAT_GetBasicName(uint8_t  *ent_name_given, uint8_t  *short_name)
{
    uint8_t name_len = 0, ext_len = 0;
    uint8_t i, char_idx, idx;
    
    // Get the file name length
    name_len = FAT_GetStringLen(ent_name_given);
    idx = 0;

    for(i = 0; i < SHORT_FILE_NAME_LEN; i++)
        short_name[i] = 0;

    // Strip all leading periods
    while( ent_name_given[idx] == '.')
        idx++;

    // Copy primary portion
    char_idx = 0;
    while(idx < name_len &&
          ent_name_given[idx] != '.' &&
          char_idx < SHORT_NAME_PART_LEN)
    {
        if(ent_name_given[char_idx] != ' ') {    // Strip all leading and embedded spaces
            if( ent_name_given[char_idx] >= 'a' &&
                ent_name_given[char_idx] >= 'z' ) {
                short_name[char_idx] =  ent_name_given[idx] - ('a' - 'A'); // converted to upper case
            } else 
                short_name[char_idx] = ent_name_given[idx];
            char_idx++;
        }
        idx++;
    }

    // Scan for the last embedded period
    idx = name_len - 1;
    while( ent_name_given[idx] != '.') {
        if(idx == 0)
            break;
        idx--;
    }
    
    // Copy extension portion
    ext_len = name_len - idx; 
    idx++;
    if( ext_len > 0 && idx != 0) {
        for(i = 0; i < ext_len; i++) { 
             short_name[SHORT_NAME_PART_LEN + i] = ent_name_given[idx++];
        }
    }

    // Insert the numeric tail  
    for(;char_idx < SHORT_NAME_PART_LEN - 2; char_idx++)
        short_name[char_idx] = ' ';
    short_name[SHORT_NAME_PART_LEN - 2] = '~';
    short_name[SHORT_NAME_PART_LEN - 1] = '1';  // should find the number
    
}

/*********************************************************************//**
 * @brief           Fills the file entry with the long file name given by the user.
 * @param[in]       ent_name_given    File name given by the user
 * @param[in]       entry             Pointer to the first FILE_ENTRY
 * @return      None.
 **********************************************************************/
 void  FAT_PutLFN (uint8_t  *ent_name_given,
                  FILE_ENTRY  *entry)
{
    uint8_t name_len = 0, part_num; 
    uint8_t part_idx, char_idx, idx;
    uint8_t sum;
    uint8_t short_name[SHORT_FILE_NAME_LEN];
    uint32_t sec_num = 0, sec_ofs = 0;

    // Get the file name length
    name_len = FAT_GetStringLen(ent_name_given);
    
    FAT_GetBasicName(ent_name_given,short_name);

    part_num = (name_len + LDIR_FILE_NAME_LEN - 1)/LDIR_FILE_NAME_LEN;

    // Read the sector
    sec_num = entry->EntrySec;
    sec_ofs = entry->EntrySecOffset;
    MS_BulkRecv(sec_num, 1, FATBuffer);

    idx = 0;
    part_idx = 0;
    sum = FAT_LFN_ChkSum(short_name);
    for(part_idx = part_num; part_idx > 0; part_idx--) {
        idx = (part_idx - 1) * LDIR_FILE_NAME_LEN;

        // Put characters 1-5
        for(char_idx = LDIR_NAME_PART1_IDX; 
               char_idx <= LDIR_NAME_PART1_END_IDX; char_idx+=2) {
            if(idx <= name_len) {
                FATBuffer[sec_ofs + char_idx] = ent_name_given[idx];
                FATBuffer[sec_ofs + char_idx + 1] = 0;
            } else {
                FATBuffer[sec_ofs + char_idx] = 0xFF;
                FATBuffer[sec_ofs + char_idx + 1] = 0xFF;
            }
            idx++;
        }

        // Get characters 6-11
        for(char_idx = LDIR_NAME_PART2_IDX; 
              char_idx <= LDIR_NAME_PART2_END_IDX; char_idx+=2) {
            if(idx <= name_len) {
                FATBuffer[sec_ofs + char_idx] = ent_name_given[idx];
                FATBuffer[sec_ofs + char_idx + 1] = 0;
            } else {
                FATBuffer[sec_ofs + char_idx] = 0xFF;
                FATBuffer[sec_ofs + char_idx + 1] = 0xFF;
            }
            idx++;
        }

        // Get characters 12-13
        for(char_idx = LDIR_NAME_PART3_IDX; 
                char_idx <= LDIR_NAME_PART3_END_IDX ; char_idx+=2) {
            if(idx <= name_len) {
                FATBuffer[sec_ofs + char_idx] = ent_name_given[idx];
                FATBuffer[sec_ofs + char_idx + 1] = 0;
            } else {
                FATBuffer[sec_ofs + char_idx] = 0xFF;
                FATBuffer[sec_ofs + char_idx + 1] = 0xFF;
            }
            idx++;
        }

        FATBuffer[sec_ofs + LDIR_ATT_IDX] = ATTR_LONG_NAME;

        FATBuffer[sec_ofs + LDIR_CHK_SUM_IDX] = sum;

        if(part_idx == part_num)
            FATBuffer[sec_ofs + LDIR_ORD_IDX] = 0x40 + part_idx;
        else
            FATBuffer[sec_ofs+ LDIR_ORD_IDX] = part_idx;

        sec_ofs += FAT_ENTRY_SIZE;
        if(sec_ofs >= FAT_BootSec.BytsPerSec)
        {
            MS_BulkSend(entry->EntrySec, 1, FATBuffer);             
            sec_num++;
            // Read the sector
            MS_BulkRecv(sec_num, 1, FATBuffer);
            sec_ofs = 0;
        }
        

    }


    // Write short directory entry
    for(char_idx = 0; char_idx < SHORT_FILE_NAME_LEN; char_idx++) {
        FATBuffer[sec_ofs + char_idx] = short_name[char_idx];
    }
    FATBuffer[sec_ofs + DIR_ATT_IDX] = ATTR_ARCHIVE;
    
    MS_BulkSend(sec_num, 1, FATBuffer); 
    entry->EntrySec = sec_num;
    entry->EntrySecOffset = sec_ofs;
    
}

/*********************************************************************//**
 * @brief           Get the path of file, store to g_filePath.
 * @param[in]       name    File name given by the user
 * @return      FAT_FUNC_OK if success.
 *                         ERR_INVALID_FILE_NAME if failed
 **********************************************************************/
 int32_t FAT_GetFilePath( uint8_t  *name)
{
    uint8_t i, sub_folder_cnt = 0, file_name_cnt = 0;
    uint32_t max_len = MAX_NAME_LEN * MAX_SUB_LEN;

    // Initialize
    for(i = 0; i < MAX_SUB_LEN; i++) {
        g_filePath[i][0] = 0;
    }

    // Parse to get file path
    for(i = 0; i < max_len; i++) {
        
        // string ends ?
        if(name[i] == 0) {
            g_filePath[sub_folder_cnt][file_name_cnt] = 0;
            break;
        }

        // new sub folder?
        if(name[i] == '\\') {
            // Set the end of string of the current sub folder
            g_filePath[sub_folder_cnt][file_name_cnt] = 0;

            // Change to the next sub-folder
            sub_folder_cnt++;
            file_name_cnt = 0;
            
            if(sub_folder_cnt >= MAX_SUB_LEN) {
                return ERR_INVALID_FILE_NAME;
            }
        } else {

            // copy file name
            g_filePath[sub_folder_cnt][file_name_cnt] = name[i];
            file_name_cnt++;
        
            if(file_name_cnt >=  MAX_NAME_LEN) {
                if(name[i] != '\\' && name[i] != 0) {
                    return ERR_INVALID_FILE_NAME;
                }
            }
        }
    }
    g_fileSubLevel = sub_folder_cnt;

    //for(i = 0; i < MAX_SUB_LEN; i++) {
        //PRINT_Log("%s\n",g_filePath[i]);
    //}
    return FAT_FUNC_OK;
}

/*********************************************************************//**
 * @brief           Get the length of a string.
 * @param[in]       str    a string
 * @return      string's length
 **********************************************************************/
 uint8_t FAT_GetStringLen(uint8_t* str) {
    
    uint8_t len = 0;
    
    while(*str) {
        len++;
        str++;
    }
    return len;
}

/*********************************************************************//**
 * @brief           Get the check sum value for a given name.
 * @param[in]       name    a short file name
 * @return      check sum value
 **********************************************************************/
 uint8_t FAT_LFN_ChkSum (uint8_t* name)
{
    int8_t len;
    uint8_t sum = 0;
    
    for (len = SHORT_FILE_NAME_LEN; len != 0; len--) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *name++;
    }
    return (sum);
}


/*********************************************************************//**
 * @brief           Checks the type of file entry.
 * @param[in]       ent           Pointer to the buffer containing the entry
 * @return      Return code.
 *                               -  LAST_ENTRY    if the entry is last entry
 *                               -  FREE_ENTRY    if the entry is free entry
 *                               -  LFN_ENTRY     if the entry is long file name entry
 *                               -  SFN_ENTRY     if the entry is short file name entry
 **********************************************************************/
uint32_t  FAT_ChkEntType (volatile  uint8_t  *ent)
{
    if (ent[0] == 0x00) {                              /* First byte is 0 means it is the last entry        */
        return (LAST_ENTRY);
    }

    if (ent[0] == 0xE5) {                              /* First byte is 0xE5 means it is the free entry     */
        return (FREE_ENTRY);
    }

    if (0x0F == ent[11]) {                             /* If 11th byte of an entry is 0x0F, it is LFN       */
        return (LFN_ENTRY);

    } else {
        return (SFN_ENTRY);                            /* Else it is the SFN                                */
    }
}

/*
**************************************************************************************************************
*                                              File Processing
**************************************************************************************************************
*/

/*********************************************************************//**
 * @brief           Store the attributes of a file, such as starting cluster, file size, 
 *                           sector where the file entry is stored and offset of the entry in that sector
 * @param[in]       file_name    Name of the file. The file must be in root directory.
 *              flags           RDONLY/RDWR
 * @return      a file descriptor which is the INDEX+1 to the entry (on success).
 *                         error code (on fail)
 **********************************************************************/
int32_t  FILE_Open (void  *file_name,
                       uint8_t   flags)
{
    int32_t   rc = 0;
    FILE_ENTRY  *entry = 0;
    int32_t  fd = -1;

    do {
        if (FAT_FileEntry[++fd].FileStatus == 0)
            entry = &FAT_FileEntry[fd];
    } while ((entry == 0) && (fd < MAX_FILE_DESCRIPTORS-1));
    if (entry == 0) {
        return (ERR_OPEN_LIMIT_REACHED);
    }
    if (flags == RDONLY) {                       /* Search for a file. If it doesn't exist, don't create it */
        rc = FAT_FindEntry(file_name, entry);
        if (rc == MATCH_FOUND) {
            entry->FileStatus = 1;
            rc = fd+1;
        }
    } else {                                     /* Search for a file. If it doesn't exist, create it       */
        rc = FAT_CreateEntry(file_name, entry);
        
        if (rc == MATCH_FOUND) {
            entry->FileStatus = 1;
            rc = fd+1;
        }
    }
    return (rc);
}

/*********************************************************************//**
 * @brief           Reads data requested by the application from the file pointed by file descriptor.
 * @param[in]       fd                  file descriptor that points to a file.
 *              buffer              buffer into which the data is to be read.
 *                         num_bytes           number of bytes requested by the application.
 * @return      total_bytes_read    Total bytes actually read.
 **********************************************************************/
uint32_t  FILE_Read (          int32_t  fd,
                       volatile  uint8_t  *buffer,
                                 uint32_t   num_bytes)
{
    uint32_t   total_bytes_to_read;            /* Total bytes requested by the application                */
    uint32_t   total_bytes_read;               /* Total bytes read                                        */
    uint32_t   bytes_read;                     /* Bytes read from one cluster                             */
    uint32_t   bytes_to_read;                  /* Bytes to be read in one cluster                         */
    FILE_ENTRY  *entry;                          /* Entry that contains the file attribute information      */
    uint32_t   next_clus;                     /* Next cluster of the current cluster in the cluster chain */

    entry = &FAT_FileEntry[fd-1];                /* Get file entry from file descriptor                     */
    total_bytes_read = 0;

    if (entry->FileSize == 0) {
        return (0);
    }
    
    if (num_bytes < entry->FileSize) {
        total_bytes_to_read = num_bytes;
    } else {
        total_bytes_to_read = entry->FileSize;
    }
    do {
        next_clus = FAT_GetNextClus(entry->CurrClus);     /* Get next cluster                               */
        
        if (next_clus == 0) {                             /* If the current cluster is the last cluster     */
                                                          /* If the offset is at the end of the file        */
            if (entry->CurrClusOffset == (entry->FileSize % FAT_BootSec.BytsPerClus)) {
                return (0);                               /* No more bytes to read                          */
            }                               /* If requested number is > remaining bytes in the last cluster */
            if (total_bytes_to_read > ((entry->FileSize % FAT_BootSec.BytsPerClus) - entry->CurrClusOffset)) {
                total_bytes_to_read = (entry->FileSize % FAT_BootSec.BytsPerClus) - entry->CurrClusOffset;
            }
            bytes_to_read = total_bytes_to_read;
                                         /* If requested number is > remaining bytes in the current cluster */
        } else if (total_bytes_to_read > (FAT_BootSec.BytsPerClus - entry->CurrClusOffset)) {
            bytes_to_read = FAT_BootSec.BytsPerClus - entry->CurrClusOffset;
        } else {
            bytes_to_read = total_bytes_to_read;
        }
        bytes_read = FAT_ClusRead(entry->CurrClus,       /* Read bytes from a single cluster                */
                                  entry->CurrClusOffset,
                                  buffer,
                                  bytes_to_read);
        buffer              += bytes_read;
        total_bytes_read    += bytes_read;
        total_bytes_to_read -= bytes_read;
                                             /* If the cluster offset reaches end of the cluster, make it 0 */
        if (entry->CurrClusOffset + bytes_read >= FAT_BootSec.BytsPerClus) {
            entry->CurrClusOffset = 0;
        } else {
            entry->CurrClusOffset += bytes_read;        /* Else increment the cluster offset                */
        }
        if (entry->CurrClusOffset == 0) {
            entry->CurrClus = (next_clus > 0) ? next_clus : entry->CurrClus;
        }
    } while (total_bytes_to_read);
    return (total_bytes_read);
}


/*********************************************************************//**
 * @brief           Writes data requested by the application to the file pointed by file descriptor.
 * @param[in]       fd                  file descriptor that points to a file.
 *              buffer              buffer into which the data is to be written.
 *                         num_bytes           number of bytes requested by the application.
 * @return      total_bytes_written    Total bytes actually written.
 **********************************************************************/
uint32_t  FILE_Write (          int32_t  fd,
                        volatile  uint8_t  *buffer,
                                  uint32_t   num_bytes)
{
    uint32_t   total_bytes_to_write;                      /* Total bytes requested by application         */
    uint32_t   total_bytes_written;                       /* Total bytes written                          */
    uint32_t   bytes_written;                             /* Bytes written in a single cluster            */
    uint32_t   bytes_to_write;                            /* Bytes to write in a single cluster           */
    FILE_ENTRY  *entry;                               /* Entry that contains the file attribute information */
    uint32_t   free_clus;                                 /* Free cluster available in the disk           */


    entry = &FAT_FileEntry[fd-1];                           /* Get file entry from file descriptor                     */
    total_bytes_written  = 0;
    total_bytes_to_write = num_bytes;

    if (num_bytes) {
        if (entry->FileSize == 0) {
            free_clus = FAT_GetFreeClus();
            FAT_UpdateFAT(free_clus, 0xFFFFFFFF);
            entry->CurrClus  = free_clus;
            FAT_UpdateEntry(entry);
        }
    } else {
        return (0);
    }
    // Write to the end of file
    entry->CurrClus = FAT_GetEndClus(entry->CurrClus);           /* Make the current cluster as end cluster */
    entry->CurrClusOffset = entry->FileSize % FAT_BootSec.BytsPerClus;   /* Move cluster offset to file end */

    // start to write to a new cluster.
    if(entry->CurrClusOffset == 0 && entry->FileSize > 0)
    {
        if (total_bytes_to_write > 0 && entry->CurrClusOffset == 0) {
            free_clus = FAT_GetFreeClus();
            if (free_clus == 0) {
                return (total_bytes_written);
            }
            FAT_UpdateFAT(entry->CurrClus, free_clus);
            FAT_UpdateFAT(free_clus, 0x0FFFFFFF);
            entry->CurrClus = free_clus;
        }
    }
    do {
        if (total_bytes_to_write > FAT_BootSec.BytsPerClus - entry->CurrClusOffset) {
            bytes_to_write = FAT_BootSec.BytsPerClus - entry->CurrClusOffset;
        } else {
            bytes_to_write = total_bytes_to_write;
        }

        bytes_written = FAT_ClusWrite(entry->CurrClus,
                                      entry->CurrClusOffset,
                                      buffer,
                                      bytes_to_write);
        buffer               += bytes_written;
        total_bytes_written  += bytes_written;
        total_bytes_to_write -= bytes_written;
        entry->FileSize      += bytes_written;
        FAT_UpdateFileSize(entry);
        
        if (entry->CurrClusOffset + bytes_written >= FAT_BootSec.BytsPerClus) {
            entry->CurrClusOffset = 0;
        } else {
            entry->CurrClusOffset += bytes_written;
        }

        // Start to write to new cluster
        if (total_bytes_to_write > 0 && entry->CurrClusOffset == 0) {
            free_clus = FAT_GetFreeClus();
            if (free_clus == 0) {
                return (total_bytes_written);
            }
            FAT_UpdateFAT(entry->CurrClus, free_clus);
            FAT_UpdateFAT(free_clus, 0x0FFFFFFF);
            entry->CurrClus = free_clus;
        }
    } while (total_bytes_to_write);
    return (total_bytes_written);
}

/*********************************************************************//**
 * @brief           Closes the opened file by making all the elements of FILE_ENTRY structure to 0.
 * @param[in]       fd    File descriptor which points to the file to be closed.
 * @return      None.
 **********************************************************************/
void  FILE_Close (int32_t  fd)
{
    FILE_ENTRY  *entry;


    entry = &FAT_FileEntry[fd-1];
    FAT_UpdateFileSize(entry);
    entry->CurrClus       = 0;
    entry->CurrClusOffset = 0;
    entry->FileSize       = 0;
    entry->EntrySec       = 0;
    entry->EntrySecOffset = 0;
    entry->FileStatus     = 0;
}
/*********************************************************************//**
 * @brief           Read DIR contents.
 * @return      Return code.
 *                  NOT_FOUND
 *                  MATCH_FOUND
 **********************************************************************/
int32_t  DIR_Open (uint8_t  *ent_name_given)
{
    int32_t rc;
    uint32_t sec_num, first_sec_num,end_sec_num ;
    FILE_ENTRY* entry = &FAT_CurDirEntry.dir_info;
    uint8_t   ent_type;
    uint8_t   ent_name_read[80];
    uint8_t   len = 0;
    uint8_t   sum = 0;
    volatile uint8_t    *buf, *afile;
    DIR_ENTRY* ptr = FAT_CurDirEntry.entries;

    // Find the entry for the given folder
    if(entry->FileStatus == 0 )
    {
        if(FAT_StrCaseCmp(ent_name_given,(uint8_t*)".") == 0)
        {
            if(FAT_BootSec.FATType == FAT_32) {
                entry->CurrClus=  FAT_BootSec.RootClus;
            } else {
                entry->CurrClus =  FAT_BootSec.RootDirStartSec/FAT_BootSec.SecPerClus;
            }
            entry->CurrClusOffset = 0;
            entry->FileStatus = 1;
        }
        else
        {
            rc = FAT_FindEntry(ent_name_given, entry);
            if (rc == MATCH_FOUND) {
               entry->FileStatus = 1;
            }
            else
            {
                return NOT_FOUND;
            }
        }
        
    }
    else
    {
        uint16_t    i = 0;
        
        // Find in the current directory
        for(i = 0; i < FAT_CurDirEntry.entry_num; i++)
        {
            if(FAT_StrCaseCmp(ent_name_given,FAT_CurDirEntry.entries[i].name) == 0)
            {
                *entry = FAT_CurDirEntry.entries[i].info;
                entry->FileStatus = 1;
                break;
            }
            
        }
    }
    
    FAT_CurDirEntry.entry_num = 0;
    first_sec_num = FAT_GetFirstSectorOfCluster(entry->CurrClus);
    end_sec_num = first_sec_num + FAT_BootSec.SecPerClus;

    for(sec_num = first_sec_num; sec_num < end_sec_num; sec_num++)
    {
        // Read all sub-entries
        MS_BulkRecv(sec_num, 1, FATBuffer);                  /* Read one sector                             */
        buf = FATBuffer;
        while (buf < (FATBuffer + FAT_BootSec.BytsPerSec)) {
                rc = NOT_FOUND;
                ent_type = FAT_ChkEntType(buf);                  /* Check for the entry type                    */
                if(ent_type == SFN_ENTRY ||
                    ent_type == LFN_ENTRY)
                {
                    afile = buf;
                    if (ent_type == SFN_ENTRY) {                     /* If it is short entry get short file name    */
                         FAT_GetSFN(buf, ent_name_read);
                         // add the entry list   
                         FAT_StrCopy(ptr->name,ent_name_read);  
                         rc = MATCH_FOUND;
                    } else if (ent_type == LFN_ENTRY) {
                        sum = buf[LDIR_CHK_SUM_IDX];
                        len = FAT_GetLfn(&buf,&sec_num,FATBuffer,FATBuffer+FAT_BootSec.BytsPerSec,ent_name_read);                   
                        if(len > 0) {
                            FAT_StrCopy(ptr->name,ent_name_read);
                            afile = buf;
                            FAT_GetSFN(buf, ent_name_read);
                            ent_type = FAT_ChkEntType(buf);
                            if(ent_type == SFN_ENTRY && sum == FAT_LFN_ChkSum((uint8_t*)buf))
                            {
                                // add the entry list
                                rc = MATCH_FOUND;
                            }
                        }
                    }

                    if(rc == MATCH_FOUND)
                    {
                        ptr->info.FileAttr = afile[DIR_ATT_IDX];
                        ptr->info.CurrClus = ReadLE16U(&afile[DIR_FST_CLUS_HI_IDX])<<16 & 0xFFFF0000;   /*  High word of the first cluster number*/
                        ptr->info.CurrClus |= ReadLE16U(&afile[DIR_FST_CLUS_LO_IDX]);   /* Low word of the first cluster number     */
                        ptr->info.CurrClusOffset = 0;
                        ptr->info.FileSize  = ReadLE32U(&afile[DIR_SIZE_IDX]);  /* Get file size                               */   
                        ptr->info.EntrySec = sec_num;           /* Get sector number where the filename is located */
                        ptr->info.EntrySecOffset = afile - FATBuffer; /* Get offset in this sector where the filename is located */
                        ptr++;
                        FAT_CurDirEntry.entry_num++;
                    }
                }
                    
                if (ent_type == LAST_ENTRY) {    /* If it is the last entry, no more entries will exist.  */
                    break;
                }
                buf = buf + FAT_ENTRY_SIZE;                                  /* Move to the next entry                      */
            }
    }
    return MATCH_FOUND;
}
/*********************************************************************//**
 * @brief           Read the next entry
 * @return      Return code.
 *                  NOT_FOUND
 *                  MATCH_FOUND
 **********************************************************************/
DIR_ENTRY*  DIR_ReadEntry (uint32_t index)
{
    DIR_ENTRY *pEntry;
    if(index >= FAT_CurDirEntry.entry_num)
    {
        return NULL;
    }

    pEntry =  &FAT_CurDirEntry.entries[index];
    return pEntry;
}

/*********************************************************************//**
 * @brief           Reads the data from a single cluster.
 * @param[in]       curr_clus         Current cluster from which the data has to read
 * @param[in]       clus_offset       Position in the current cluster from which the data has to read
 * @param[in]       buffer            Buffer into which the data has to read
 * @param[in]       num_bytes         Number of bytes to read
 * @return      tot_bytes_read    Total bytes read from the current cluster.
 **********************************************************************/
uint32_t  FAT_ClusRead (          uint32_t   curr_clus,
                                    uint32_t   clus_offset,
                          volatile  uint8_t  *buffer,
                                    uint32_t   num_bytes)
{
    uint32_t  tot_bytes_read;                              /* total bytes read in the current cluster     */
    uint32_t  n_bytes;                                     /* Bytes to read in the current sector         */
    uint32_t  start_sec;                                   /* Starting sector of the current cluster      */
    uint32_t  sec_num;                                     /*Current sector number                        */
    uint16_t  num_sec;                                     /* Number of sectors to be read                */
    uint32_t  sec_offset;                                  /* Offset in the current sector                */
    uint32_t  cnt;


    tot_bytes_read = 0;
    start_sec  = ((curr_clus - 2) * FAT_BootSec.SecPerClus) + FAT_BootSec.FirstDataSec;
    sec_num    = start_sec + (clus_offset / FAT_BootSec.BytsPerSec);
    num_sec    = num_bytes / FAT_BootSec.BytsPerSec;
    sec_offset = clus_offset % FAT_BootSec.BytsPerSec;

    if (sec_offset) {                                 /* If the sector offset is at the middle of a sector  */
        MS_BulkRecv(sec_num, 1, FATBuffer);           /* Read the first sector                              */
        n_bytes = (FAT_BootSec.BytsPerSec - sec_offset <= num_bytes) ?
                  (FAT_BootSec.BytsPerSec - sec_offset) : num_bytes;
        for (cnt = sec_offset; cnt < sec_offset + n_bytes; cnt++) {
            *buffer = FATBuffer[cnt];                 /* Copy the required bytes to user buffer             */
             buffer++;
        }
        tot_bytes_read += n_bytes;
        clus_offset    += n_bytes;
        num_bytes      -= n_bytes;
        sec_num++;
    }

    if (num_bytes / FAT_BootSec.BytsPerSec) {         /* Read all the remaining full sectors                */

        num_sec = num_bytes / FAT_BootSec.BytsPerSec;
        MS_BulkRecv(sec_num, num_sec, buffer);
        buffer         += (num_sec * FAT_BootSec.BytsPerSec);
        tot_bytes_read += (num_sec * FAT_BootSec.BytsPerSec);
        clus_offset    += (num_sec * FAT_BootSec.BytsPerSec);
        num_bytes      -= (num_sec * FAT_BootSec.BytsPerSec);
        sec_num        += num_sec;
    }

    if (num_bytes) {                                  /* Read the last sector for the remaining bytes       */
        MS_BulkRecv(sec_num, 1, FATBuffer);
        for (cnt = 0; cnt < num_bytes; cnt++) {
            *buffer = FATBuffer[cnt];                 /* Copy the required bytes to user buffer             */
             buffer++;
        }
        tot_bytes_read += num_bytes;
    }

    return (tot_bytes_read);
}

/*********************************************************************//**
 * @brief           Writes the data to a single cluster.
 * @param[in]       curr_clus         Current cluster from which the data has to write
 * @param[in]       clus_offset       Position in the current cluster from which the data has to write
 * @param[in]       buffer            Buffer into which the data has to write
 * @param[in]       num_bytes         Number of bytes to write
 * @return      tot_bytes_read    Total bytes written into the current cluster.
 **********************************************************************/
uint32_t  FAT_ClusWrite (          uint32_t   curr_clus,
                                     uint32_t   clus_offset,
                           volatile  uint8_t  *buffer,
                                     uint32_t   num_bytes)
{
    uint32_t  tot_bytes_written;
    uint32_t  n_bytes;
    uint32_t  start_sec;
    uint32_t  sec_num;
    uint16_t  num_sec;
    uint32_t  sec_offset;
    uint32_t  cnt;


    tot_bytes_written = 0;
    start_sec  = FAT_GetFirstSectorOfCluster(curr_clus);
    sec_num    = start_sec + (clus_offset / FAT_BootSec.BytsPerSec);
    num_sec    = num_bytes / FAT_BootSec.BytsPerSec;
    sec_offset = clus_offset % FAT_BootSec.BytsPerSec;

    if (sec_offset) {
        MS_BulkRecv(sec_num, 1, FATBuffer);
    n_bytes = (FAT_BootSec.BytsPerSec - sec_offset <= num_bytes) ?
                  (FAT_BootSec.BytsPerSec - sec_offset) : num_bytes;
    for (cnt = sec_offset; cnt < (sec_offset + n_bytes); cnt++) {
            FATBuffer[cnt] = *buffer;
        buffer++;
    }
        MS_BulkSend(sec_num, 1, FATBuffer);
    tot_bytes_written += n_bytes;
    clus_offset       += n_bytes;
        num_bytes         -= n_bytes;
    sec_num++;
    }
    if (num_bytes / FAT_BootSec.BytsPerSec) {
    num_sec = num_bytes / FAT_BootSec.BytsPerSec;
    MS_BulkSend(sec_num, num_sec, buffer);
    buffer            += (num_sec * FAT_BootSec.BytsPerSec);
    tot_bytes_written += (num_sec * FAT_BootSec.BytsPerSec);
    clus_offset       += (num_sec * FAT_BootSec.BytsPerSec);
        num_bytes         -= (num_sec * FAT_BootSec.BytsPerSec);
    sec_num           += num_sec;
    }
    if (num_bytes) {
        MS_BulkRecv(sec_num, 1, FATBuffer);

    for (cnt = 0; cnt < num_bytes; cnt++) {
            FATBuffer[cnt] = *buffer;
        buffer++;
    }
        MS_BulkSend(sec_num, 1, FATBuffer);
    tot_bytes_written += num_bytes;
    }
    return (tot_bytes_written);
}
/**
* @}
*/


