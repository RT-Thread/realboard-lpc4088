/**********************************************************************
* $Id$      usbhost_main.c          2011-09-05
*//**
* @file     usbhost_main.c
* @brief    Demo for USB Host Controller.
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
*                                       INCLUDE HEADER FILES
**************************************************************************************************************
*/
#include "debug_frmwrk.h"
#include  "usbhost_main.h"

/** @defgroup USBHost_MassStorage   USB Host Controller for Mass Storage Device
 * @ingroup USBHostLite_Examples 
 * @{
 */

/** @defgroup USBHost_Fat    Fat File System
 * @ingroup USBHost_MassStorage
 * @{
 */

/**
 * @}
 */


/** @defgroup USBHost_Ms  USB Host Mass Storage Class
 * @ingroup USBHost_MassStorage
 * @{
 */

/**
 * @}
 */

/** @defgroup USBHost_Uart  USB Host Debug
 * @ingroup USBHost_MassStorage
 * @{
 */

/**
 * @}
 */

uint8_t menu[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" UART Host Lite example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" This example used to test USB Host function.\n\r"
"********************************************************************************\n\r";
static uint8_t file_w[80];
static uint8_t* file_r;

/*********************************************************************//**
 * @brief       Print menu
 * @param[in]   None
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG_(menu);
}
/*********************************************************************//**
 * @brief           This function is the main function where the execution begins
 * @param[in]       None
 * @return          None
 **********************************************************************/
int main()
{
    int32_t  rc;
    uint32_t  numBlks, blkSize;                 \
    uint8_t  inquiryResult[INQUIRY_LENGTH];

    //SystemInit();

    debug_frmwrk_init();
        
    print_menu();
        
    Host_Init();               /* Initialize the lpc17xx host controller                                    */

    PRINT_Log("Host Initialized\n");
    PRINT_Log("Connect a Mass Storage device\n");
    
    rc = Host_EnumDev();       /* Enumerate the device connected                                            */
    if ((rc == USB_HOST_FUNC_OK) && 
    (Host_GetDeviceType() == MASS_STORAGE_DEVICE)) {
        
    PRINT_Log("Mass Storage device connected\n");

    /* Initialize the mass storage and scsi interfaces */
        rc = MS_Init( &blkSize, &numBlks, inquiryResult );
        if (rc == MS_FUNC_OK) {
            rc = FAT_Init();   /* Initialize the FAT16 file system                                          */
            if (rc == FAT_FUNC_OK) {
                Main_ReadDir((uint8_t*)".");
            } else {
                return (0);
            }
        } else {
            return (0);
        }
    } else {
        PRINT_Log("Not a Mass Storage device\n");                           
        return (0);
    }

    Main_Copy();
    while(1);
}

/*********************************************************************//**
 * @brief           Read directory 
 * @param[in]       None
 * @return          None
 **********************************************************************/
void Main_ReadDir(uint8_t* pName)
{
    int32_t  rc, i = 0, cnt = 0;
    DIR_ENTRY *pEntry;

    rc = DIR_Open(pName);   
    PRINT_Log("Files/Folders in the directory \"%s\":\n",pName);
    PRINT_Log("%3s\t%10s\t%6s\t%s\n","No.","Size","DIR","Name");
    if(rc == 0)
    {
        while(1)
        {
            
            pEntry = DIR_ReadEntry(i);
            if(pEntry == NULL)
                return; 
            if(pEntry->info.FileAttr & ATTR_DIRECTORY)
            {
                 cnt++;
                 PRINT_Log("%3d\t%10s\t%6s\t%s\n",cnt," ","<DIR>", pEntry->name);
            }
            else
            {
                if(pEntry->info.FileSize)
                {
                    cnt++;
                    PRINT_Log("%3d\t%10d\t%6s\t%s\n",cnt,pEntry->info.FileSize," ", pEntry->name);
                }
            }
            i++;
        }
    }
}
/*********************************************************************//**
 * @brief           Get the name of files which are objects of copying
 * @param[in]       None
 * @return          TRUE/FALSE
 **********************************************************************/
static Bool get_objects(void)
{
    int32_t  fdw;
    uint32_t i = 0, j = 0, k = 0;
    int32_t exts_idx = -1;
    DIR_ENTRY *pEntry;

     // Get name of the first file in flash drive
    while(1)
    {
        pEntry = DIR_ReadEntry(i++);
        if(pEntry == NULL)
        {
            return FALSE;   
        }
       if(((pEntry->info.FileAttr & ATTR_DIRECTORY) == 0) &&
            (pEntry->info.FileSize > 0))
        {
           file_r = pEntry->name; 
           break;
        }
    }

    // Get the name of output file
    i = 0;
    while(1)
    {
        if(file_r[i] == '.')
        {
            exts_idx = i;
            break;
        }
        else if ((file_r[i] == 0)||(i >= 79))
        {
            break;
        }
        file_w[i] = file_r[i]; 
        i++;
    }
    file_w[i++] = '_';
    j = 1;
    while(1)
    {
        file_w[i] = '0'+j;
        if(exts_idx >= 0)
        {
            for(k = 0; k <= 4; k++)
                file_w[i+k+1] = file_r[exts_idx + k];
        }
        file_w[i+k+1] = 0;

        fdw = FILE_Open(file_w, RDONLY);
        if(fdw > 0)
        {
            FILE_Close(fdw);
            j++;
            if(j >= 10)
            {
                i++;
                j = 1;
            }
        }
        else
        {
            break;
        }
    }
    return TRUE;
}
/*********************************************************************//**
 * @brief           This function is used by the user to copy a file 
 * @param[in]       None
 * @return          None
 **********************************************************************/

void  Main_Copy (void)
{
    int32_t  fdr;
    int32_t  fdw;
    uint32_t  bytes_read;
    
    if(!get_objects())
    {
        PRINT_Log("No file to copy\n");
        return;
    }
    
    // Copy file
    fdr = FILE_Open(file_r, RDONLY);
    if (fdr > 0) {
        fdw = FILE_Open(file_w, RDWR);
        if (fdw > 0) {
            PRINT_Log("Copying file %s to %s...\n", file_r, file_w);
            do {
                bytes_read = FILE_Read(fdr, UserBuffer, MAX_BUFFER_SIZE);
                FILE_Write(fdw, UserBuffer, bytes_read);
            } while (bytes_read);
            FILE_Close(fdw);
        } else {
            PRINT_Log("Could not open file %s\n", file_w);
            return;
        }
        FILE_Close(fdr);
        PRINT_Log("Copy completed\n");
    } else {
        PRINT_Log("Could not open file %s\n", file_r);
        return;
    }
}
