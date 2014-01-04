/*---------------------------------------------------------------------------*/
/* FAT file system sample project for FatFs (LPC17xx)    (C)ChaN, NXP, 2010  */
/*---------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "LPC407x_8x_177x_8x.h"
#include "system_LPC407x_8x_177x_8x.h"
#include "lpc_uart.h"
#include "lpc_rtc.h"
#include "monitor.h"
#include "integer.h"
#include "diskio.h"
#include "ff.h"
#include "debug_frmwrk.h"
#include "lpc_mci.h"

/** @defgroup MCI_FatFS	MCI FatFS 
 * @ingroup MCI_FS
 * Refer to @link Examples/MCI/Mci_FS/FatFs/doc/00index_e.htm @endlink
 * @{
 */

#if _USE_XSTRFUNC==0
#include <string.h>
#define xstrlen(x)      strlen(x)
#define xstrcpy(x,y)    strcpy(x,y)
#define xmemset(x,y,z)  memset(x,y,z)
#define xstrchr(x,y)    strchr(x,y)
#endif

/* LPC Definitions */
#define _LPC_RTC		(LPC_RTC)


/* buffer size (in byte) for R/W operations */
#define BUFFER_SIZE     4096 

DWORD acc_size;				/* Work register for fs command */
WORD acc_files, acc_dirs;
FILINFO Finfo;
#if _USE_LFN
char Lfname[_MAX_LFN+1];
#endif

char Line[128];				/* Console input buffer */

FATFS Fatfs[_VOLUMES];		/* File system object for each logical drive */

/* Increase the buff size will get faster R/W speed. */
#if 1 /* use local SRAM */
#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=4
static BYTE Buff[BUFFER_SIZE] ;		/* Working buffer */
#else
static BYTE Buff[BUFFER_SIZE] __attribute__ ((aligned (4))) ;		/* Working buffer */
#endif
static UINT blen = sizeof(Buff);
#else   /* use 16kB SRAM on AHB0 */
static BYTE *Buff = (BYTE *)0x20000000;	 
static UINT blen = 16*1024;
#endif

volatile UINT Timer = 0;		/* Performance timer (1kHz increment) */

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
" FatFs,a generic FAT file system module for small embedded systems, is used in \n\r"
" this example. \n\r"
" Press r to display commands which are supported.\n\r"
"********************************************************************************\n\r";

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


/* Get RTC time */
void rtc_gettime (RTC_TIME_Type *rtc)
{
	RTC_GetFullTime( _LPC_RTC, rtc);	
}

/* Set RTC time */
void rtc_settime (const RTC_TIME_Type *rtc)
{
	/* Stop RTC */
	RTC_Cmd(_LPC_RTC, DISABLE);

	/* Update RTC registers */
	RTC_SetFullTime (_LPC_RTC, (RTC_TIME_Type *)rtc);

	/* Start RTC */
	RTC_Cmd(_LPC_RTC, ENABLE);
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support a real time clock.          */
/* This is not required in read-only configuration.        */
DWORD get_fattime ()
{
	RTC_TIME_Type rtc;

	/* Get local time */
	rtc_gettime(&rtc);

	/* Pack date and time into a DWORD variable */
	return	  ((DWORD)(rtc.YEAR - 1980) << 25)
			| ((DWORD)rtc.MONTH << 21)
			| ((DWORD)rtc.DOM << 16)
			| ((DWORD)rtc.HOUR << 11)
			| ((DWORD)rtc.MIN<< 5)
			| ((DWORD)rtc.SEC>> 1);

}

static
void put_rc (		/* Put FatFs return code */
	FRESULT rc
)
{
	const char *str =
		"OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
		"INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
		"INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
		"LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";
	FRESULT i;

	for (i = (FRESULT)0; i != rc && *str; i++) {
		while (*str++) ;
	}
	xprintf("rc=%u FR_%s\n", (UINT)rc, str);
}


/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */
/*--------------------------------------------------------------------------*/
static
FRESULT scan_files (
	char* path		/* Pointer to the path name working buffer */
)
{
	DIR dirs;
	FRESULT res;
	BYTE i;
	char *fn;


	if ((res = f_opendir(&dirs, path)) == FR_OK) {
		i = xstrlen(path);
		while (((res = f_readdir(&dirs, &Finfo)) == FR_OK) && Finfo.fname[0]) {
			if (_FS_RPATH && Finfo.fname[0] == '.') continue;
#if _USE_LFN
			fn = *Finfo.lfname ? Finfo.lfname : Finfo.fname;
#else
			fn = Finfo.fname;
#endif
			if (Finfo.fattrib & AM_DIR) {
				acc_dirs++;
				*(path+i) = '/'; xstrcpy(path+i+1, fn);
				res = scan_files(path);
				*(path+i) = '\0';
				if (res != FR_OK) break;
			} else {
			//	xprintf("%s/%s\n", path, fn);
				acc_files++;
				acc_size += Finfo.fsize;
			}
		}
	}

	return res;
}
void put_char(unsigned char ch)
{
    _DBC(ch);    
}
unsigned char get_char(void)
{
   return _DG;    
}
static void IoInit(void) 
{
	RTC_TIME_Type  current_time;

	SysTick_Config(SystemCoreClock/1000 - 1); /* Generate interrupt each 1 ms   */

	RTC_Init(_LPC_RTC);
	current_time.SEC = 0;
	current_time.MIN = 0;
	current_time.HOUR= 0;
	current_time.DOM= 1;
	current_time.DOW= 0;
	current_time.DOY= 0;		/* current date 01/01/2010 */
	current_time.MONTH= 1;
	current_time.YEAR= 2010;
	RTC_SetFullTime (_LPC_RTC, &current_time);		/* Set local time */
	RTC_Cmd(_LPC_RTC,ENABLE);

	debug_frmwrk_init();	
    xfunc_out = put_char;
	xfunc_in  = get_char;    	
}
void print_commands()
{
    xputs("Commands: \n");
    xputs("          dd <phy_drv#> [<sector>] - Dump secrtor\n");
    xputs("          di <phy_drv#> - Initialize disk\n");
    xputs("          ds <phy_drv#> - Show disk status\n");
    xputs("          bd <addr> - Dump R/W buffer \n");
    xputs("          be <addr> [<data>] ... - Edit R/W buffer \n");
    xputs("          br <phy_drv#> <sector> [<n>] - Read disk into R/W buffer \n");
    xputs("          bw <phy_drv#> <sector> [<n>] - Write R/W buffer into disk \n");
    xputs("          bf <val> - Fill working buffer  \n");
    xputs("          fi <log drv#> - Initialize logical drive   \n");
    xputs("          fs - Show logical drive status   \n");
    xputs("          fl [<path>] - Directory listing   \n");
    xputs("          fo <mode> <file> - Open a file    \n");
    xputs("          fc - Close a file    \n");
    xputs("          fe - Seek file pointer    \n");
    xputs("          fd <len> - read and dump file from current fp    \n");
    xputs("          fr <len> - read file    \n");
    xputs("          fw <len> <val> - write file    \n");
    xputs("          fn <old_name> <new_name> - Change file/dir name     \n");
    xputs("          fu <name> - Unlink a file or dir     \n");
    xputs("          fv - Truncate file      \n");
    xputs("          fk <name> - Create a directory      \n");
    xputs("          fa <atrr> <mask> <name> - Change file/dir attribute       \n");
    xputs("          ft <year> <month> <day> <hour> <min> <sec> <name> - Change timestamp       \n");
    xputs("          fx <src_name> <dst_name> - Copy file       \n");
    xputs("          fg <path> - Change current directory       \n");
    #if _USE_MKFS
    xputs("          fm <partition rule> <cluster size> - Create file system       \n");
    #endif
    xputs("          fz [<rw size>] - Change R/W length for fr/fw/fx command       \n");
    xputs("          t [<year> <mon> <mday> <hour> <min> <sec>]        \n");
}
void disk_cmd_handle(char* ptr)
{
	long p1, p2;
	BYTE res, b1;    
	WORD w1;
	DWORD ofs = 0, sect = 0;
    switch (*ptr++) {
    case 'd' :
    	switch (*ptr++) {
    
    	case 'd' :	/* dd <phy_drv#> [<sector>] - Dump secrtor */
    		if (!xatoi(&ptr, &p1)) break;
    		if (!xatoi(&ptr, &p2)) p2 = sect;
    		res = disk_read((BYTE)p1, Buff, p2, 1);
    		if (res) { xprintf("rc=%d\n", (WORD)res); break; }
    		sect = p2 + 1;
            xprintf("Sector:%lu\n", p2);
    		for (ptr=(char*)Buff, ofs = 0; ofs < 0x200; ptr+=16, ofs+=16)
    			put_dump((BYTE*)ptr, ofs, 16);
    		break;
    
    	case 'i' :	/* di <phy_drv#> - Initialize disk */
    		if (!xatoi(&ptr, &p1)) break;
    		xprintf("rc=%d\n", (WORD)disk_initialize((BYTE)p1));
    		break;
    
    	case 's' :	/* ds <phy_drv#> - Show disk status */
    		if (!xatoi(&ptr, &p1)) break;
    		if (disk_ioctl((BYTE)p1, GET_SECTOR_COUNT, &p2) == RES_OK)
    			{ xprintf("Drive size: %lu sectors\n", p2); }
    		if (disk_ioctl((BYTE)p1, GET_SECTOR_SIZE, &w1) == RES_OK)
    			{ xprintf("Sector size: %u\n", w1); }
    		if (disk_ioctl((BYTE)p1, GET_BLOCK_SIZE, &p2) == RES_OK)
    			{ xprintf("Erase block: %lu sectors\n", p2); }
    		if (disk_ioctl((BYTE)p1, MMC_GET_TYPE, &b1) == RES_OK)
    			{ xprintf("Card type: %u\n", b1); }
    		if (disk_ioctl((BYTE)p1, MMC_GET_CSD, Buff) == RES_OK)
    			{ xputs("CSD:\n"); put_dump(Buff, 0, 16); }
    		if (disk_ioctl((BYTE)p1, MMC_GET_CID, Buff) == RES_OK)
    		{
                st_Mci_CardId* cidval =  (st_Mci_CardId*)Buff;
                xputs("CID:\n");
                xprintf("\n\r\t- Manufacture ID: 0x%x\n", cidval->MID);
        		xprintf("\n\r\t- OEM/Application ID: 0x%x\n", cidval->OID);
        		xprintf("\n\r\t- Product Name: 0x%x%x\n", cidval->PNM_H,cidval->PNM_L);
        		xprintf("\n\r\t- Product Revision: 0x%x\n", cidval->PRV);
        		xprintf("\n\r\t- Product Serial Number: 0x%x\n",cidval->PSN);
        		xprintf("\n\r\t- Manufacturing Date: 0x%x\n",cidval->MDT);
    
            }
    		if (disk_ioctl((BYTE)p1, MMC_GET_SDSTAT, Buff) == RES_OK) {
    			xputs("SD Status:\n");put_dump(Buff, 0, 2);
    			//for (s1 = 0; s1 < 64; s1 += 16) put_dump(Buff+s1, s1, 16);
    		}
    		break;
    	}
    	break;
   }
}

void buff_cmd_handle(char* ptr)
{
	long p1, p2, p3;
	UINT cnt; 
	DWORD ofs = 0;
    switch (*ptr++) {
     case 'b' :
		switch (*ptr++) {
		case 'd' :	/* bd <addr> - Dump R/W buffer */
			if (!xatoi(&ptr, &p1)) break;
			for (ptr=(char*)&Buff[p1], ofs = p1, cnt = 32; cnt; cnt--, ptr+=16, ofs+=16)
				put_dump((BYTE*)ptr, ofs, 16);
			break;

		case 'e' :	/* be <addr> [<data>] ... - Edit R/W buffer */
			if (!xatoi(&ptr, &p1)) break;
			if (xatoi(&ptr, &p2)) {
				do {
					Buff[p1++] = (BYTE)p2;
				} while (xatoi(&ptr, &p2));
				break;
			}
			for (;;) {
				xprintf("%04X %02X-", (WORD)(p1), (WORD)Buff[p1]);
				get_line(Line, sizeof(Line));
				ptr = Line;
				if (*ptr == '.') break;
				if (*ptr < ' ') { p1++; continue; }
				if (xatoi(&ptr, &p2))
					Buff[p1++] = (BYTE)p2;
				else
					xputs("???\n");
			}
			break;

		case 'r' :	/* br <phy_drv#> <sector> [<n>] - Read disk into R/W buffer */
			if (!xatoi(&ptr, &p1)) break;
			if (!xatoi(&ptr, &p2)) break;
			if (!xatoi(&ptr, &p3)) p3 = 1;
			xprintf("rc=%u\n", disk_read((BYTE)p1, Buff, p2, p3));
			break;

		case 'w' :	/* bw <phy_drv#> <sector> [<n>] - Write R/W buffer into disk */
			if (!xatoi(&ptr, &p1)) break;
			if (!xatoi(&ptr, &p2)) break;
			if (!xatoi(&ptr, &p3)) p3 = 1;
			xprintf("rc=%u\n", disk_write((BYTE)p1, Buff, p2, p3));
			break;

		case 'f' :	/* bf <val> - Fill working buffer */
			if (!xatoi(&ptr, &p1)) break;
			xmemset(Buff, (BYTE)p1, sizeof(Buff));
			break;
		}
		break;
   }
}
void file_cmd_handle(char* ptr)
{
   char  *ptr2;
	long p1, p2, p3, xlen;
	BYTE res;    
	UINT s1, s2, cnt; 
	DWORD ofs = 0;
    const BYTE ft[] = {0,12,16,32};
	static FATFS *fs;				/* Pointer to file system object */
    static FIL File1, File2;		/* File objects */
    static DIR Dir;				/* Directory object */
    uint32_t time_end;
    switch (*ptr++) {
    case 'f' :
		switch (*ptr++) {

		case 'i' :	/* fi <log drv#> - Initialize logical drive */
			if (!xatoi(&ptr, &p1)) break;
			put_rc(f_mount((BYTE)p1, &Fatfs[p1]));
			break;

		case 's' :	/* fs - Show logical drive status */
			while (_USE_LFN && *ptr == ' ') ptr++;
			res = f_getfree(ptr, (DWORD*)&p2, &fs);
			if (res) { put_rc((FRESULT)res); break; }
			xprintf("FAT type = FAT%u\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
					"Root DIR entries = %u\nSectors/FAT = %lu\nNumber of clusters = %lu\n"
					"FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\nData start (lba) = %lu\n\n...",
					ft[fs->fs_type & 3], (DWORD)fs->csize * 512, fs->n_fats,
					fs->n_rootdir, fs->fsize, (DWORD)fs->n_fatent - 2,
					fs->fatbase, fs->dirbase, fs->database
			);
			acc_size = acc_files = acc_dirs = 0;
			res = scan_files(ptr);
			if (res) { put_rc((FRESULT)res); break; }
			xprintf("\r%u files, %lu bytes.\n%u folders.\n"
					"%lu KB total disk space.\n%lu KB available.\n",
					acc_files, acc_size, acc_dirs,
					(((fs->n_fatent - 2) * fs->csize) / 2), ((p2 * fs->csize) / 2)
			);
			break;

		case 'l' :	/* fl [<path>] - Directory listing */
			while (*ptr == ' ') ptr++;
			res = f_opendir(&Dir, ptr);
			if (res) { put_rc((FRESULT)res); break; }
			p1 = s1 = s2 = 0;
			for(;;) {
				res = f_readdir(&Dir, &Finfo);
				if ((res != FR_OK) || !Finfo.fname[0]) break;
				if (Finfo.fattrib & AM_DIR) {
					s2++;
				} else {
					s1++; p1 += Finfo.fsize;
				}
				xprintf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s",
						(Finfo.fattrib & AM_DIR) ? 'D' : '-',
						(Finfo.fattrib & AM_RDO) ? 'R' : '-',
						(Finfo.fattrib & AM_HID) ? 'H' : '-',
						(Finfo.fattrib & AM_SYS) ? 'S' : '-',
						(Finfo.fattrib & AM_ARC) ? 'A' : '-',
						(Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
						(Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63,
						Finfo.fsize, &(Finfo.fname[0]));
#if _USE_LFN
				for (p2 = xstrlen(Finfo.fname); p2 < 14; p2++)
					xputc(' ');
				xprintf("%s\n", Lfname);
#else
				xputc('\n');
#endif
			}
            /* Note: For 4GB+ card, the displayed size may not be correct since
            the max size for 32bit is 4G */
            if(p1 <= (uint32_t)0xFFFFFFFF)
                xprintf("%4u File(s),%10lu bytes total\n", s1, p1);
            else
            {
               xprintf("%4u File(s),%10lu KB total\n", s1, p1/1024);
            }
			if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
            {
                uint64_t free_bytes = ((uint64_t)p1) * fs->csize * 512;
                if(free_bytes <= (uint32_t)0xFFFFFFFF) 
                    xprintf("%4u Dir(s), %10lu bytes free\n",s2, free_bytes);
                else
                    xprintf("%4u Dir(s), %10lu KB free\n",s2, free_bytes/1024);
            }
			break;

		case 'o' :	/* fo <mode> <file> - Open a file */
			if (!xatoi(&ptr, &p1)) break;
			while (*ptr == ' ') ptr++;
			put_rc(f_open(&File1, ptr, (BYTE)p1));
			break;

		case 'c' :	/* fc - Close a file */
			put_rc(f_close(&File1));
			break;

		case 'e' :	/* fe - Seek file pointer */
			if (!xatoi(&ptr, &p1)) break;
			res = f_lseek(&File1, p1);
			put_rc((FRESULT)res);
			if (res == FR_OK)
				xprintf("fptr=%lu(0x%lX)\n", File1.fptr, File1.fptr);
			break;

		case 'd' :	/* fd <len> - read and dump file from current fp */
			if (!xatoi(&ptr, &p1)) break;
			ofs = File1.fptr;
			while (p1) {
				if ((UINT)p1 >= 16) { cnt = 16; p1 -= 16; }
				else 				{ cnt = p1; p1 = 0; }
				res = f_read(&File1, Buff, cnt, &cnt);
				if (res != FR_OK) { put_rc((FRESULT)res); break; }
				if (!cnt) break;
				put_dump(Buff, ofs, cnt);
				ofs += 16;
			}
			break;

		case 'r' :	/* fr <len> - read file */
			if (!xatoi(&ptr, &p1)) break;
			p2 = 0;
			Timer = 0;
			while (p1) {
				if ((UINT)p1 >= blen) {
					cnt = blen; p1 -= blen;
				} else {
					cnt = p1; p1 = 0;
				}
				res = f_read(&File1, Buff, cnt, &s2);
				if (res != FR_OK) { put_rc((FRESULT)res); break; }
				p2 += s2;
				if (cnt != s2) break;
			}
            time_end = Timer;
			xprintf("%lu bytes read in %lu miliseconds.\n", p2, time_end);
            xprintf("File's content: \n%s\n",Buff);
			break;

		case 'w' :	/* fw <len> <val> - write file */
			if (!xatoi(&ptr, &p1)) break;
			while (*ptr == ' ') ptr++;
            xlen = xstrlen(ptr);
			p2 = 0;
			Timer = 0;
			while (p1) {
				if ((UINT)p1 >= xlen) {
					cnt = xlen; p1 -= xlen;
				} else {
					cnt = p1; p1 = 0;
				}
				res = f_write(&File1, ptr, cnt, &s2);
				if (res != FR_OK) { put_rc((FRESULT)res); break; }
				p2 += s2;
				if (cnt != s2) break;
			}
            time_end = Timer;
			xprintf("%lu bytes written in %lu miliseconds.\n", p2, time_end);
			break;

		case 'n' :	/* fn <old_name> <new_name> - Change file/dir name */
			while (*ptr == ' ') ptr++;
			ptr2 = xstrchr(ptr, ' ');
			if (!ptr2) break;
			*ptr2++ = 0;
			while (*ptr2 == ' ') ptr2++;
			put_rc(f_rename(ptr, ptr2));
			break;

		case 'u' :	/* fu <name> - Unlink a file or dir */
			while (*ptr == ' ') ptr++;
			put_rc(f_unlink(ptr));
			break;

		case 'v' :	/* fv - Truncate file */
			put_rc(f_truncate(&File1));
			break;

		case 'k' :	/* fk <name> - Create a directory */
			while (*ptr == ' ') ptr++;
			put_rc(f_mkdir(ptr));
			break;

		case 'a' :	/* fa <atrr> <mask> <name> - Change file/dir attribute */
			if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
			while (*ptr == ' ') ptr++;
			put_rc(f_chmod(ptr, p1, p2));
			break;

		case 't' :	/* ft <year> <month> <day> <hour> <min> <sec> <name> - Change timestamp */
			if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
			Finfo.fdate = ((p1 - 1980) << 9) | ((p2 & 15) << 5) | (p3 & 31);
			if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
			Finfo.ftime = ((p1 & 31) << 11) | ((p2 & 63) << 5) | ((p3 >> 1) & 31);
            while (*ptr == ' ') ptr++;
			put_rc(f_utime(ptr, &Finfo));
			break;

		case 'x' : /* fx <src_name> <dst_name> - Copy file */
			while (*ptr == ' ') ptr++;
			ptr2 = xstrchr(ptr, ' ');
			if (!ptr2) break;
			*ptr2++ = 0;
			while (*ptr2 == ' ') ptr2++;
			xprintf("Opening \"%s\"", ptr);
			res = f_open(&File1, ptr, FA_OPEN_EXISTING | FA_READ);
			xputc('\n');
			if (res) {
				put_rc((FRESULT)res);
				break;
			}
			xprintf("Creating \"%s\"", ptr2);
			res = f_open(&File2, ptr2, FA_CREATE_ALWAYS | FA_WRITE);
			xputc('\n');
			if (res) {
				put_rc((FRESULT)res);
				f_close(&File1);
				break;
			}
			xprintf("Copying file...");
			Timer = 0;
			p1 = 0;
			for (;;) {
				res = f_read(&File1, Buff, blen, &s1);
				if (res || s1 == 0) break;   /* error or eof */
				res = f_write(&File2, Buff, s1, &s2);
				p1 += s2;
				if (res || s2 < s1) break;   /* error or disk full */
			}
            time_end = Timer;
			xprintf("%lu bytes copied in %lu miliseconds.\n", p1, time_end);
            xprintf("Close \"%s\": ", ptr);
			put_rc(f_close(&File1));
            xprintf("Close \"%s\": ", ptr2);
			put_rc(f_close(&File2));
			break;

#if _FS_RPATH
		case 'g' :	/* fg <path> - Change current directory */
			while (*ptr == ' ') ptr++;
			put_rc(f_chdir(ptr));
			break;

		case 'j' :	/* fj <drive#> - Change current drive */
			if (xatoi(&ptr, &p1)) {
				put_rc(f_chdrive((BYTE)p1));
			}
			break;
#endif
#if _USE_MKFS
		case 'm' :	/* fm <partition rule> <cluster size> - Create file system */
			if (!xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
			xprintf("The card will be formatted. Are you sure? (Y/n)=");
			get_line(ptr, sizeof(Line));
			if (*ptr == 'Y')
				put_rc(f_mkfs(0, (BYTE)p2, (WORD)p3));
			break;
#endif
		case 'z' :	/* fz [<rw size>] - Change R/W length for fr/fw/fx command */
			if (xatoi(&ptr, &p1) && p1 >= 1 && (unsigned long)p1 <= sizeof(Buff))
				blen = p1;
			xprintf("blen=%u\n", blen);
			break;
		}
   }
}
void other_cmd_handle(char* ptr)
{
	long p1 ;
	RTC_TIME_Type rtc;
    switch (*ptr++) {
    case 't' :	/* t [<year> <mon> <mday> <hour> <min> <sec>] */
		if (xatoi(&ptr, &p1)) {
			rtc.YEAR = (WORD)p1;
			xatoi(&ptr, &p1); rtc.MONTH= (BYTE)p1;
			xatoi(&ptr, &p1); rtc.DOM= (BYTE)p1;
			xatoi(&ptr, &p1); rtc.HOUR= (BYTE)p1;
			xatoi(&ptr, &p1); rtc.MIN= (BYTE)p1;
			if (!xatoi(&ptr, &p1)) break;
			rtc.SEC = (BYTE)p1;
			rtc_settime(&rtc);
		}
		rtc_gettime(&rtc);
		xprintf("%u/%u/%u %02u:%02u:%02u\n", rtc.YEAR, rtc.MONTH, rtc.DOM, rtc.HOUR, rtc.MIN, rtc.SEC);
		break;
    case 'r':
        print_commands();
        break;
   }
}
/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/
int main ()
{
	char *ptr;
	IoInit();
    xprintf("%s",mciFsMenu);
  
#if _USE_LFN
	Finfo.lfname = Lfname;
	Finfo.lfsize = sizeof(Lfname);
#endif

	for (;;) {
		xputc('>');
		ptr = Line;
		get_line(ptr, sizeof(Line));

		disk_cmd_handle(ptr);
        buff_cmd_handle(ptr);
		file_cmd_handle(ptr);
        other_cmd_handle(ptr);
	}											  
	
}
/**
 * @}
 */
 

