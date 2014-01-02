/**********************************************************************
* $Id$      Emc_NandFlashDemo.c 2011-06-02
*//**
* @file     Emc_NandFlashDemo.c
* @brief    This example describes how to use EMC interface on LPC177x_8x/LPC407x_8x.
*           to connect with Nand Flash K9F1G08U0A on EA board
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
#include "nandflash_k9f1g08u0a.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"


/** @defgroup EMC_NandFlashDemo EMC NandFlash Demo
 * @ingroup EMC_Examples
 * @{
 */
#define TEST_BLOCK_NUM      0
#define TEST_PAGE_NUM       0

/************************** PRIVATE VARIABLES *************************/
const unsigned char menu[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
#ifdef _RUNNING_NANDFLASH_K9F1G08U0C
" # NANDFLASH K9F1G08U0C testing \n\r"
#else
" # NANDFLASH K9F1G08U0A testing \n\r"
#endif
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Comunication: 115200 bps \n\r"
" Write and verify data with on-board NAND FLASH\n\r"
"********************************************************************************\n\r";
uint8_t ReadBuf[NANDFLASH_PAGE_FSIZE], WriteBuf[NANDFLASH_PAGE_FSIZE];
extern uint8_t InvalidBlockTable[NANDFLASH_NUMOF_BLOCK];

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
    uint32_t FlashID;
    uint32_t i;
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

    _DBG_("Init NAND Flash...");

    /* initialize memory */
    NandFlash_Init();
    NandFlash_Reset();

    _DBG("Read NAND Flash ID:  ");
    FlashID = NandFlash_ReadId();
    if ( (FlashID & 0xFFFF0000) != K9FXX_ID )
    {
        _DBG_("Error in reading NAND Flash ID, testing terminated!");
        while( 1 ); /* Fatal error */
    }

    _DBH32_(FlashID);_DBG_("");

    _DBG_("Checking valid block...");
    if ( NandFlash_ValidBlockCheck() == FALSE )
    {
        _DBG_("Valid block checking error at block(s): ");

        for(i = 0; i < NANDFLASH_NUMOF_BLOCK; i++)
        {
            if (InvalidBlockTable[i] == TRUE)
            {
                _DBD32(i);_DBG("   ");
            }
        }
        _DBG_("");
    }

    /**************************************************************
    * NandFlash_BlockErase() could scrub off all the invalid        *
    * block infomation including the factory initial invalid        *
    * block table information. Per Samsung's K9F1G08 Users Manual,*
    * "Any intentional erasure of the initial invalid block     *
    * information is prohibited.                                    *
    *   However, during the driver debugging, it may create lot of  *
    *   invalid blocks. Below NandFlash_BlockErase() is used to deal *
    * with situation like that.                                   *
    *                                                               *
    ***************************************************************/
    /* Erase the entire NAND FLASH */
    _DBG_("Erase entire NAND Flash...");
    for ( i = 0; i < NANDFLASH_NUMOF_BLOCK; i++ )
    {
        if ( (InvalidBlockTable[i] == FALSE) && ( NandFlash_BlockErase(i) == FALSE ))
        {
            _DBG("Erase NAND Flash fail at block: ");_DBD32(i);_DBG_("");
        }
    }

    /* For the test program, the pattern for the whole page 2048 bytes
    is organized as: 0x0, 0x1, ... 0xFF, 0x0, 0x01...... */
    for ( i = 0; i < NANDFLASH_RW_PAGE_SIZE; i++ )
    {
        ReadBuf[i] = 0;
        WriteBuf[i] = i;
    }

    /* If it's a valid block, program all the pages of this block,
    read back, and finally validate. */

    if ( InvalidBlockTable[TEST_BLOCK_NUM] == FALSE )
    {
        _DBG_("Write a block of 2K data to NAND Flash...");
        if ( NandFlash_PageProgram(  TEST_BLOCK_NUM, TEST_PAGE_NUM, &WriteBuf[0],FALSE) == FALSE )
           {
                _DBG_("Writing fail, testing terminated!");
                while ( 1 );    /* Fatal error */
            }
        _DBG_("Read back a block of 2K data from NAND Flash...");
        if ( NandFlash_PageRead(  TEST_BLOCK_NUM, TEST_PAGE_NUM,&ReadBuf[0] ) == FALSE )
            {
                _DBG_("Reading fail, testing terminated!");
                while ( 1 );    /* Fatal error */
            }

        /* Comparison read and write buffer */
        _DBG_("Verify data...");
        for ( i = 0; i < NANDFLASH_RW_PAGE_SIZE; i++ )
        {
            if ( ReadBuf[i] != WriteBuf[i] )
            {
                _DBG_("Verifying fail, testing terminated!");
                while ( 1 );    /* Fatal error */
            }
        }
    }

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
