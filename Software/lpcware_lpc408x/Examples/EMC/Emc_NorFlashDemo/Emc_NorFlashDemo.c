/**********************************************************************
* $Id$      Emc_NorFlashDemo.c      2011-06-02
*//**
* @file     Emc_NorFlashDemo.c
* @brief    This example describes how to use EMC interface on LPC177x_8x/LPC407x_8x.
            to connect with Nor Flash SST39VF3201 on EA board
* @version  1.0
* @date     02. June. 2011
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
#include "norflash_sst39vf3201.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"


/** @defgroup EMC_NorFlashDemo  EMC NorFlash Demo
 * @ingroup EMC_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
#define NORFLASH_RW_PAGE_SIZE   0x800

/************************** PRIVATE VARIABLES *************************/
const unsigned char menu[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" EMC NORFLASH example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif

"\t - UART Comunication: 115200 bps \n\r"
" Write and verify data with on-board NOR FLASH\n\r"
"********************************************************************************\n\r";

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

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main ADC program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    uint32_t i;
    volatile uint16_t *ip;
    uint32_t NorFlashAdr;

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

    _DBG_("Init NOR Flash...");
    NORFLASHInit();

    _DBG_("Read NOR Flash ID...");
    if ( NORFLASHCheckID() == FALSE )
    {
        _DBG_("Error in reading NOR Flash ID, testing terminated!");
        while ( 1 );        /* Fatal error */
    }

    _DBG_("Erase entire NOR Flash...");
    NORFLASHErase();        /* Chip erase */

    /* Write to flash with pattern 0xAA55 and 0xA55A */
    NorFlashAdr = NOR_FLASH_BASE;

    _DBG_("Write a block of 2K data to NOR Flash...");
    for ( i = 0; i < NORFLASH_RW_PAGE_SIZE/2; i++ )
    {
        NORFLASHWriteWord( NorFlashAdr, 0xAA55 );
        NorFlashAdr++;
        NORFLASHWriteWord( NorFlashAdr, 0xA55A );
        NorFlashAdr++;
    }

    /* Verify */
    _DBG_("Verify data...");
    NorFlashAdr = NOR_FLASH_BASE;
    for ( i = 0; i < NORFLASH_RW_PAGE_SIZE/2; i+=2 )
    {
        ip  = GET_ADDR(i);
        if ( (*ip & 0xFFFF) != 0xAA55 )
        {
            _DBG_("Verifying fail, testing terminated!");
            while ( 1 );    /* Fatal error */
        }

        ip  = GET_ADDR(i+1);
        if ( (*ip & 0xFFFF) != 0xA55A )
        {
            _DBG_("Verifying fail, testing terminated!");
            while ( 1 );    /* Fatal error */
        }
    }

    // terminated
    _DBG_("Verifying complete! Testing terminated!");

    while (1);
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
