/******************************************************************************
 
 efsl Demo-Application for NXP LPC1768 Cortex-M3
 
 * @note
 * Copyright (C) 2010 NXP Semiconductors(NXP). 
 * All rights reserved.
 
 *****************************************************************************/

#include "Core_CM3/lpc17xx.h"
#include "Terminal/monitor.h"
#include "UART/lpc17xx_uart.h"
#include "interfaces/lpc17xx_sd.h"
#include "efs.h"
#include "ls.h"

/* file name could be file.txt, /file.txt, dir1/file.txt, etc,
Note: does NOT support LFN */
#define FILE_NAME  "test07.txt"
//#define FILE_NAME  "/d1/d1_test1.txt"  // ok
#define FILE_RW_SIZE    (4*1024*1024)

#define READ_TEST_ENABLED  1    // 0 to disalbe
#define WRITE_TEST_ENABLED 1    // 0 to disalbe

EmbeddedFileSystem  efs;
EmbeddedFile filer, filew;
DirList             list;
uint8_t       file_name[13];


uint8_t Buff[1024];
uint32_t blen = sizeof(Buff);


volatile uint32_t Timer = 0;		/* Performance timer (1kHz increment) */

/* SysTick Interrupt Handler (1ms)    */
void SysTick_Handler (void) 
{           
	static uint32_t div10;

	Timer++;

	if (++div10 >= 10) {
		div10 = 0;
		disk_timerproc();		/* Disk timer function (100Hz) */
	}
}

int main()
{
	int8_t res;
    uint32_t n, m, p, cnt;

//	SystemInit();
    SysTick_Config(SystemCoreClock/1000 - 1); /* Generate interrupt each 1 ms   */

	LPC17xx_UART_Init(115200); // UART0
    xfunc_out = LPC17xx_UART_PutChar;
	xfunc_in  = LPC17xx_UART_GetChar; 

	xprintf("\nMMC/SD Card Filesystem Test (P:LPC1768 L:EFSL)\n");

	xprintf("\nCARD init...");

	// init file system
	if ( ( res = efs_init( &efs, 0 ) ) != 0 ) {
		xprintf("failed with %i\n",res);
	}
	else 
	{
		xprintf("ok\n");

        xprintf("Card type: ");
        switch (CardType)
        {
            case CARDTYPE_MMC:
                xprintf("MMC\n");
                break;
            case CARDTYPE_SDV1:
                xprintf("Version 1.x Standard Capacity SD card.\n");
                break;
            case CARDTYPE_SDV2_SC:
                xprintf("Version 2.0 or later Standard Capacity SD card.\n");
                break;
            case CARDTYPE_SDV2_HC:
                xprintf("Version 2.0 or later High/eXtended Capacity SD card.\n");
                break;
            default:
                break;            
        }
        xprintf("Sector size: %d bytes\n", CardConfig.sectorsize);
        xprintf("Sector count: %d\n", CardConfig.sectorcnt);
        xprintf("Block size: %d sectors\n", CardConfig.blocksize);
        xprintf("Card capacity: %d MByte\n\n", (((CardConfig.sectorcnt >> 10) * CardConfig.sectorsize)) >> 10);

		xprintf("\nDirectory of 'root':\n");
		
		/* list files in root directory */
		ls_openDir( &list, &(efs.myFs) , "/");
		while ( ls_getNext( &list ) == 0 ) {
			// list.currentEntry is the current file
			list.currentEntry.FileName[LIST_MAXLENFILENAME-1] = '\0';
			xprintf("%s, 0x%x bytes\n", list.currentEntry.FileName, list.currentEntry.FileSize ) ;
		}

#if WRITE_TEST_ENABLED!=0
        /* Write test*/  
        xprintf("\nFile write test:\n");
        xprintf("Open file %s ...", FILE_NAME);
        if (file_fopen( &filew, &efs.myFs , FILE_NAME , 'a' ) == 0 )
        {
            xprintf(" OK. \nWriting %lu bytes ...\n", FILE_RW_SIZE);

            xmemset(Buff, 0xAA, blen);
            n=FILE_RW_SIZE;
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
                p = file_write( &filew, cnt, Buff );
                m += p;
                if (p != cnt) break;
            }
            xprintf("%lu bytes written with %lu kB/sec.\n", m, Timer ? (m / Timer) : 0);

            file_fclose( &filew );                          

        } else {
            xprintf (" Failed.\n");
        }
#endif

#if READ_TEST_ENABLED!=0
        /* Read test */
        xprintf("\nFile read test:\n");
        xprintf("Open file %s ...", FILE_NAME);
        if (file_fopen( &filer, &efs.myFs , FILE_NAME , 'r' ) == 0 )
        {
            xprintf(" OK. \nReading %lu bytes ...\n", FILE_RW_SIZE);

            n=FILE_RW_SIZE; 
            m = 0;
            Timer = 0;
            while (n)
            {
                if (n>=blen) {cnt = blen; n -= blen;}
                else         {cnt = n; n = 0;}

                p =  file_read( &filer, cnt, Buff );
                m += p;
                if (p != cnt) break;                
            }
            xprintf("%lu bytes read with %lu kB/sec.\n", m, Timer ? (m / Timer) : 0);

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
