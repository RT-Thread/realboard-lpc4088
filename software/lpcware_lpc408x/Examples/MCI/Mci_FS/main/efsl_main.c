/******************************************************************************
 
 efsl Demo-Application for NXP LPC1768 Cortex-M3
 
 * @note
 * Copyright (C) 2010 NXP Semiconductors(NXP). 
 * All rights reserved.
 
 *****************************************************************************/

#include "LPC407x_8x_177x_8x.h"
#include "monitor.h"
#include "debug_frmwrk.h"
#include "fs_mci.h"
#include "efs.h"
#include "ls.h"
#include "lpc_clkpwr.h"

/** @defgroup MCI_EFSL  MCI EFSL 
 * @ingroup MCI_FS
 * Refer to @link Examples/MCI/Mci_FS/efsl/docs/manual.pdf @endlink
 * @{
 */
 

/* file name could be file.txt, /file.txt, dir1/file.txt, etc,
Note: does NOT support LFN */
#define FILE_NAME_R  "file_r.txt"
#define FILE_NAME_W  "file_w.txt"
//#define FILE_NAME  "/d1/d1_test1.txt"  // ok
#define FILE_RW_SIZE    (4*1024)

#define READ_TEST_ENABLED  1    // 0 to disalbe
#define WRITE_TEST_ENABLED 1    // 0 to disalbe

EmbeddedFileSystem  efs;
EmbeddedFile filer, filew;
DirList             list;
uint8_t       file_name[13];


uint8_t Buff[1024];
uint32_t blen = sizeof(Buff);


volatile uint32_t Timer = 0;        /* Performance timer (1kHz increment) */
uint8_t mciFsMenu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" MCI File System Example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif

"\t - UART Communicationi: 115200 bps \n\r"
" This example is used to demonstrate how to implement a filesystem using MCI.\n\r"
" EFLS,a library for file systems, is used in this example. \n\r"
"********************************************************************************\n\r";
/* SysTick Interrupt Handler (1ms)    */
void SysTick_Handler (void) 
{           
    static uint32_t div10;

    Timer++;

    if (++div10 >= 10) {
        div10 = 0;
        disk_timerproc();       /* Disk timer function (100Hz) */
    }
}
void put_char(unsigned char ch)
{
    _DBC(ch);    
}
unsigned char get_char(void)
{
   return _DG;    
}
int main()
{
    int8_t res;
    uint32_t n, m, p, cnt;
    uint32_t cclk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_CPU);
    uint32_t filesize = 0;
    uint32_t time_end;

//  SystemInit();
    SysTick_Config(cclk/1000 - 1); /* Generate interrupt each 1 ms   */

    debug_frmwrk_init(); // UART0
    xfunc_out = put_char;
    xfunc_in  = get_char; 

    xprintf("%s",mciFsMenu);

    xprintf("\nMMC/SD Card Filesystem Test (P:LPC1788 L:EFSL)\n");

    xprintf("\nCARD init...");

    // init file system
    if ( ( res = efs_init( &efs, 0 ) ) != 0 ) {
        xprintf("failed with %d\n",res);
    }
    else 
    {
        xprintf("ok\n");

        xprintf("Card type: ");
        switch (CardConfig.CardType)
        {
            case MCI_MMC_CARD:
                xprintf("MMC\n");
                break;
            case MCI_SDSC_V1_CARD:
                xprintf("Version 1.x Standard Capacity SD card.\n");
                break;
            case MCI_SDSC_V2_CARD:
                xprintf("Version 2.0 or later Standard Capacity SD card.\n");
                break;
            case MCI_SDHC_SDXC_CARD:
                xprintf("Version 2.0 or later High/eXtended Capacity SD card.\n");
                break;
            default:
                break;            
        }
        xprintf("Sector size: %d bytes\n", CardConfig.SectorSize);
        xprintf("Sector count: %d\n", CardConfig.SectorCount);
        xprintf("Block size: %d sectors\n", CardConfig.BlockSize);
        xprintf("Card capacity: %d MByte\n\n", (((CardConfig.SectorCount >> 10) * CardConfig.SectorSize)) >> 10);
        xprintf("\nDirectory of 'root':\n");
        
        /* list files in root directory */
        ls_openDir( &list, &(efs.myFs) , "/");
        while ( ls_getNext( &list ) == 0 ) {
            // list.currentEntry is the current file
            list.currentEntry.FileName[LIST_MAXLENFILENAME-1] = '\0';
            xprintf("%s, 0x%x bytes\n", list.currentEntry.FileName, list.currentEntry.FileSize ) ;
        }
#if READ_TEST_ENABLED!=0
        /* Read test */
        xprintf("\nFile read test:\n");
        xprintf("Open file %s ...", FILE_NAME_R);
        xmemset(Buff,0,sizeof(Buff));
        if (file_fopen( &filer, &efs.myFs , FILE_NAME_R , 'r' ) == 0 )
        {
            xprintf(" OK. \nReading %lu bytes ...\n", FILE_RW_SIZE);

            n=FILE_RW_SIZE; 
            m = 0;
            Timer = 0;
            xprintf("File's content:\n");
            while (n)
            {
                if (n>=blen) {cnt = blen; n -= blen;}
                else         {cnt = n; n = 0;}

                p =  file_read( &filer, cnt, &Buff[m] );
                xprintf("%s",&Buff[m]);
                m += p;
                if (p != cnt) break;                
            }
            filesize = m;
            time_end = Timer;
            xprintf("\n%lu bytes read in %lu milisec.\n", m, time_end);
            file_fclose( &filer ); 

        } else
        {
            xprintf (" Failed.\n");    
        }
#endif
#if WRITE_TEST_ENABLED!=0
        /* Write test*/  
        xprintf("\nFile write test:\n");
        xprintf("Open file %s ...", FILE_NAME_W);
        if (file_fopen( &filew, &efs.myFs , FILE_NAME_W , 'a' ) == 0 )
        {
            xprintf(" OK. \nWriting %lu bytes ...\n", filesize);
            n=filesize;
            m = 0;
            Timer = 0;
            while (n)
            {
                if (n>=blen) {
                    cnt = blen;
                    n -= blen;
                } else {
                    cnt = n;
                    n = 0;
                }
                p = file_write( &filew, cnt, &Buff[m] );
                m += p;
                if (p != cnt) break;
            }
            time_end = Timer;
            xprintf("%lu bytes written in %lu milisec.\n", m, time_end);

            file_fclose( &filew );                          

        } else {
            xprintf (" Failed.\n");
        }
#endif
#if READ_TEST_ENABLED!=0
        /* Read test */
        xprintf("\nFile read test:\n");
        xprintf("Open file %s ...", FILE_NAME_W);
        xmemset(Buff,0,sizeof(Buff));
        if (file_fopen( &filer, &efs.myFs , FILE_NAME_W , 'r' ) == 0 )
        {
            xprintf(" OK. \nReading %lu bytes ...\n", FILE_RW_SIZE);

            n=FILE_RW_SIZE; 
            m = 0;
            Timer = 0;
            xprintf("File's content:\n");
            while (n)
            {
                if (n>=blen) {cnt = blen; n -= blen;}
                else         {cnt = n; n = 0;}

                p =  file_read( &filer, cnt, &Buff[m] );
                xprintf("%s",&Buff[m]);
                m += p;
                if (p != cnt) break;                
            }
            filesize = m;
            time_end = Timer;
            xprintf("\n%lu bytes read in %lumiliseconds.\n", m, time_end);
            file_fclose( &filer ); 

        } else
        {
            xprintf (" Failed.\n");    
        }
#endif
        /* close file system */
        fs_umount( &efs.myFs ) ;
    }

    xprintf("\nEFSL test complete.\n");

    while (1);
}
/**
 * @}
 */
 

