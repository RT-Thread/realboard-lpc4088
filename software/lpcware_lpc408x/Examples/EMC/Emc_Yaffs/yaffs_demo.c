/**********************************************************************
* $Id$      yaffs_demo.c                    2012-12-06
*//**
* @file     yaffs_demo.c
* @brief    This example describes how to port yffs library on LPC177x_8x
* @version  1.0
* @date     06. December. 2012
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

#include "lpc_types.h"
#include "lpc_uart.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"
#include "yaffsfs.h"
#include "bsp.h"
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
#include "sdram_k4s561632j.h"
#elif (_CURR_USING_BRD == _QVGA_BOARD)
#include "sdram_mt48lc8m32lfb5.h"
#elif (_CURR_USING_BRD == _EA_PA_BOARD)
#include "sdram_is42s32800d.h"
#endif

/** @defgroup EMC_NandFlashDemo EMC NandFlash Demo
 * @ingroup EMC_Examples
 * @{
 */

#define FILE_NUM            2
#define FILE_BUFFER_SIZE    2048
/************************** PRIVATE VARIABLES *************************/
const unsigned char menu[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" Yaffs Porting \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif

"\t - UART Comunication: 115200 bps \n\r"
"********************************************************************************\n\r";

int FileHdl[FILE_NUM];

unsigned char FileBuffer[FILE_BUFFER_SIZE];

/************************** PRIVATE FUNCTION *************************/
void print_menu(void);

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
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
 * @brief       Create file name
 * @param[in]   None
 * @return      None
 **********************************************************************/
int make_file_name(uint8_t* buf, uint8_t i)
{
    int idx = 0;
    buf[idx++] = '/';
    buf[idx++] = 'p';
    buf[idx++] = '0';
    buf[idx++] = '/';
    buf[idx++] = 'F';
    buf[idx++] = 'i';
    buf[idx++] = 'l';
    buf[idx++] = 'e';
    buf[idx++] = '_';
    buf[idx++] = '0' + i;
    buf[idx++] = 0;
    return idx;
}
/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main ADC program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    uint32_t i, j;

    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    // print welcome screen
    print_menu();
    SDRAMInit();
    
    _DBG_("Startup Yaffs");
    yaffs_StartUp();
    
    _DBG("Mount to p0 ...");
    if(yaffs_mount("/p0") < 0)
    {
        _DBG_("Fail!");
        while(1);
    }
    _DBG_("OK!");
     
    _DBG("Create files...");
    for(i = 0; i < FILE_NUM; i++)
    {
        make_file_name(FileBuffer,i);
        FileHdl[i] = yaffs_open((char*)FileBuffer, O_CREAT | O_RDWR | O_TRUNC, S_IREAD | S_IWRITE);
        if(FileHdl[i] == -1)
        {
             _DBG_("Fail!");
            while(1);
        }
    }
    _DBG_("OK!");
    
    _DBG("Write files...");
    for(i = 0; i < FILE_NUM; i++)
    {
        for(j = 0; j < FILE_BUFFER_SIZE-1; j++)
        {
            FileBuffer[j] = '0' + i%10;
        }
        FileBuffer[FILE_BUFFER_SIZE-1] = 0;
        if(yaffs_write(FileHdl[i],FileBuffer,FILE_BUFFER_SIZE) < 0)
        {
             _DBG_("Fail!");
            while(1);
        }
    }
    _DBG_("OK!");
    
    _DBG("Close file...");
    for(i = 0; i < FILE_NUM; i++)
    {
        if(yaffs_close(FileHdl[i]) < 0)
        {
            _DBG_("Fail!");
            while(1);
        }
    }
    _DBG_("OK!");
    
    _DBG("Read files...");
    for(i = 0; i < FILE_NUM; i++)
    {
        make_file_name(FileBuffer,i);
        FileHdl[i] = yaffs_open((char*)FileBuffer, O_RDONLY, 0);
        if(FileHdl[i] == -1)
        {
             _DBG_("Fail!");
            while(1);
        }
    }
    _DBG_("");
    for(i = 0; i < FILE_NUM; i++)
    {
        if(yaffs_read(FileHdl[i],FileBuffer,sizeof(FileBuffer)) < 0)
        {
             _DBG_("Fail!");
            while(1);
        }
        _DBG("Content of file[");_DBD(i);_DBG_("]:");
        _DBG_(FileBuffer);
    }
    _DBG_("OK!");   
    
    
   _DBG("Close file...");
    for(i = 0; i < FILE_NUM; i++)
    {
        if(yaffs_close(FileHdl[i]) < 0)
        {
            _DBG_("Fail!");
            while(1);
        }
    }
    _DBG_("OK!");
    
    while(1);
}

/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
int main(void)
{
    c_entry();
    return 0;
}


/**
 * @}
*/
