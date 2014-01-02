/**********************************************************************
* $Id$      Crc_Demo.c  2011-06-02
*//**
* @file     Crc_Demo.c
* @brief    This example describe how to use CRC engine on LPC177x_8x/LPC407x_8x.
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
#include "lpc_crc.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"

/** @defgroup CRC_Demo  CRC Demo
 * @ingroup CRC_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS ***********************/
#define BLOCK_SIZE      0x40

/************************** PRIVATE VARIABLES *************************/
uint8_t menu[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" CRC Demo example: \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" Use CRC engine to calculate CRC for a 8-bit block data \n\r"
" You can choose one of three polynomial type: \n\r"
"\t - CRC-CCITT \n\r"
"\t - CRC-16\n\r"
"\t - CRC-32\n\r"
"********************************************************************************\n\r";
uint8_t BlockData[BLOCK_SIZE];
Bool CRC_init_flag = FALSE;

/************************** PRIVATE FUNCTION *************************/
void print_menu(void);

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Initialize block data
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Init_BlockData(void)
{
    uint32_t i;
    for(i=0;i<BLOCK_SIZE;i++)
    {
        BlockData[i] = i;
    }
}
/*********************************************************************//**
 * @brief       print menu
 * @param[in]   None
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG(menu);
}
/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main ADC program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    uint32_t result;
    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();
    print_menu();

    Init_BlockData();

    while(!CRC_init_flag)
    {
        _DBG_("\n\rChoose what polynomial that you want to use, type:");
        _DBG_("\t - '1': CRC-CCITT");
        _DBG_("\t - '2': CRC-16");
        _DBG_("\t - '3': CRC-32");
        _DBG_("\t - 'Q': Quit");
        switch(_DG)
        {
        case '1':
            CRC_Init(CRC_POLY_CRCCCITT);
            _DBG("CRC-CCITT Result: ");
            result = CRC_CalcBlockChecksum(BlockData, BLOCK_SIZE, CRC_WR_8BIT);
            _DBH32(result);_DBG_("");
            break;
        case '2':
            CRC_Init(CRC_POLY_CRC16);
            _DBG("CRC-16 Result: ");
            result = CRC_CalcBlockChecksum(BlockData, BLOCK_SIZE/2, CRC_WR_16BIT);
            _DBH32(result);_DBG_("");
            break;
        case '3':
            CRC_Init(CRC_POLY_CRC32);
            _DBG("CRC-32 Result: ");
            result = CRC_CalcBlockChecksum(BlockData, BLOCK_SIZE/4, CRC_WR_32BIT);
            _DBH32(result);_DBG_("");
            break;
        case'q':
        case'Q':
            _DBG_("Demo terminated!!!");
            CRC_init_flag = TRUE;
            break;
        default:
            _DBG_("Invalid input, pls type again!");
            break;
        }
    }
    while(1);
}

/* Support required entry point for other toolchain */
int main (void)
{
    c_entry();
    return 0;
}


/**
 * @}
 */
