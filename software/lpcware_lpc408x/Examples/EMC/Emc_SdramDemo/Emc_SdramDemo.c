/**********************************************************************
* $Id$      Emc_SdramDemo.c     2011-06-02
*//**
* @file     Emc_SdramDemo.c
* @brief    This example describes how to use EMC interface on LPC177x_8x/LPC407x_8x.
*               to connect with external SDRAM
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
#include "bsp.h"
#include "lpc_types.h"
#include "lpc_uart.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"

#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
#include "sdram_k4s561632j.h"
#elif (_CURR_USING_BRD == _QVGA_BOARD)
#include "sdram_mt48lc8m32lfb5.h"
#elif (_CURR_USING_BRD == _EA_PA_BOARD)
#include "sdram_is42s32800d.h"
#elif (_CURR_USING_BRD == _RB4088_BOARD)
#include "sdram_h57v2562gtr.h"
#endif


/** @defgroup EMC_SdramDemo EMC SDRAM Demo
 * @ingroup EMC_Examples
 * @{
 */


/************************** PRIVATE VARIABLES *************************/
const unsigned char menu[] =
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
" Test SDRAM K4S561632J with LPC1788 EMC \n\r"
#elif (_CURR_USING_BRD == _QVGA_BOARD)
" Test SDRAM MT48LC8M32LFB5 with LPC1788 EMC \n\r"
#elif (_CURR_USING_BRD == _EA_PA_BOARD)
" Test SDRAM IS42S32800D with LPC1788 EMC \n\r"
#elif (_CURR_USING_BRD == _RB4088_BOARD)
" Test SDRAM H57V2562GTR with LPC4088 EMC \n\r"
#endif
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Comunication: 115200 kbps \n\r"
" Write and verify data with on-board SDRAM\n\r"
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
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry (void)
{
    uint32_t i;
    volatile uint32_t *wr_ptr;
    volatile uint8_t *char_wr_ptr;
    volatile uint16_t *short_wr_ptr;

    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();
    print_menu();

    /* initialize memory */
    _DBG_("Init SDRAM...");
    SDRAMInit();

    wr_ptr = (uint32_t *)SDRAM_BASE_ADDR;
    char_wr_ptr = (uint8_t *)wr_ptr;
    /* Clear content before 8 bit access test */
    _DBG_("Clear content of SDRAM...");
    for ( i= 0; i < SDRAM_SIZE/4; i++ )
    {
      *wr_ptr++ = 0x00;
    }

    /* 8 bit write */
    _DBG_("Writing in 8 bits format...");
    for (i=0; i<SDRAM_SIZE/4; i++)
    {
      *char_wr_ptr++ = 0x11;
      *char_wr_ptr++ = 0x22;
      *char_wr_ptr++ = 0x33;
      *char_wr_ptr++ = 0x44;
    }

    /* verifying */
    _DBG_("Verifying data...");
    wr_ptr = (uint32_t *)SDRAM_BASE_ADDR;
    for ( i= 0; i < SDRAM_SIZE/8; i++ )
    {
        if ( *wr_ptr != 0x44332211 )    /* be aware of endianess */
        {
            /* byte comparison failure */
            _DBG_("Verifying fail, testing terminated!");
            while ( 1 );    /* fatal error */
        }
        wr_ptr++;
    }

    /* byte comparison succeed. */
    _DBG_("Continue writing in 16 bits format...");
    wr_ptr = (uint32_t *)SDRAM_BASE_ADDR;
    short_wr_ptr = (uint16_t *)wr_ptr;

    /* Clear content before 16 bit access test */
    _DBG_("Clear content of SRAM...");
    for ( i= 0; i < SDRAM_SIZE/4; i++ )
    {
        *wr_ptr++ = 0;
    }

    /* 16 bit write */
    _DBG_("Writing in 16 bits format...");
    for (i=0; i<(SDRAM_SIZE/4); i++)
    {
        *short_wr_ptr++ = 0x5AA5;
        *short_wr_ptr++ = 0xAA55;
    }

    /* Verifying */
    wr_ptr = (uint32_t *)SDRAM_BASE_ADDR;

    //wr_ptr -= SDRAM_BASE_ADDR/4;
    for ( i= 0; i < SDRAM_SIZE/4; i++ )
    {
        if ( *wr_ptr != 0xAA555AA5 )    /* be aware of endianess */
        {
            /* 16-bit half word failure */
            _DBG_("Verifying fail, testing termintated!");
        while ( 1 );    /* fatal error */
        }
        wr_ptr++;
    }

    /* 16-bit half word comparison succeed. */

    _DBG_("Verifying complete, testing terminated!");
        while(1);
}

int main(void)
{
    c_entry();
    return 0;
}
/*****************************************************************************
**                            End Of File
*****************************************************************************/

/**
 * @}
*/
