/**********************************************************************
* $Id$      nandflash_k9f1g08u0a.c          2011-06-02
*//**
* @file     nandflash_k9f1g08u0a.c
* @brief    This c file contains all functions support for Nand Flash 
*           SamSung K9F1G08U0A
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
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _EMC

#include "nandflash_k9f1g08u0a.h"
#include "lpc_emc.h"
#include "lpc_clkpwr.h"
#include "lpc_pinsel.h"
#include "lpc_timer.h"

uint8_t InvalidBlockTable[NANDFLASH_NUMOF_BLOCK];

/*********************************************************************//**
 * @brief       Ready/Busy check, no timeout, basically, R/B bit should
 *              once to bail out from this routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void NandFlash_WaitForReady( void )
{

    while( FIO2PIN & (1 << 21) );       /* from high to low once */

    while( !(FIO2PIN & (1 << 21)) );    /* from low to high once */

    return;
}

/*********************************************************************//**
 * @brief       Initialize external NAND FLASH memory
 * @param[in]   None
 * @return      None
 **********************************************************************/
void NandFlash_Init( void )
{
    uint32_t i;
    TIM_TIMERCFG_Type TIM_ConfigStruct;
    EMC_STATIC_MEM_Config_Type config;

    /**************************************************************************
    * Initialize EMC for NAND FLASH
    **************************************************************************/
    config.CSn = 1;
    config.AddressMirror = 0;
    config.ByteLane = 1;
    config.DataWidth = 8;
    config.ExtendedWait = 0;
    config.PageMode = 0;
    config.WaitWEn = 2;
    config.WaitOEn = 2;
    config.WaitWr = 0x1f;
    config.WaitPage = 0x1f;
    config.WaitRd = 0x1f;
    config.WaitTurn = 0x1f; 
    StaticMem_Init(&config);
     // init timer
    TIM_ConfigStruct.PrescaleOption = TIM_PRESCALE_USVAL;
    TIM_ConfigStruct.PrescaleValue  = 1;

    // Set configuration for Tim_config and Tim_MatchConfig
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE,&TIM_ConfigStruct);

    // wait 2ms
    TIM_Waitms(2);

    /* assume all blocks are valid to begin with */
    for ( i = 0; i < NANDFLASH_NUMOF_BLOCK; i++ )
    {
        InvalidBlockTable[i] = FALSE;
    }

    return;
}

/*********************************************************************//**
 * @brief       Issue Reset command to NAND FLASH memory
 * @param[in]   None
 * @return      None
 **********************************************************************/
void NandFlash_Reset( void )
{
    volatile uint8_t *pCLE;

    /* Reset NAND FLASH  through NAND FLASH command */
    pCLE = K9F1G_CLE;
    *pCLE = K9FXX_RESET;

    TIM_Waitms(2);
    return;
}

/*********************************************************************//**
 * @brief       Read status from NAND FLASH memory
 * @param[in]   Cmd command for read operation, should be:
 *                  -  K9FXX_BLOCK_PROGRAM_1
 *                  -  K9FXX_BLOCK_ERASE_1
 *                  -  K9FXX_READ_1
 * @return      Status, could be:
 *              - TRUE: pass
 *              - FALSE: Failure
 **********************************************************************/
Bool NandFlash_ReadStatus(uint32_t Cmd)
{
    volatile uint8_t *pCLE;
    volatile uint8_t *pDATA;
    uint8_t StatusData;

    pCLE  = K9F1G_CLE;
    pDATA = K9F1G_DATA;

    *pCLE = K9FXX_READ_STATUS;

#if (_CUR_USING_NANDFLASH == _RUNNING_NANDFLASH_K9F1G08U0C)
    while ( (*pDATA & 0xC0) != 0xC0 );
#else
    /* Wait until bit 5 and 6 are 1, READY, bit 7 should be 1 too, not protected */
    /* if ready bit not set, it gets stuck here */
    while ( (*pDATA & 0xE0) != 0xE0 );
#endif

    StatusData = *pDATA;

    switch (Cmd)
    {
        case K9FXX_BLOCK_PROGRAM_1:
        case K9FXX_BLOCK_ERASE_1:
            if (StatusData & 0x01)  /* Erase/Program failure(1) or pass(0) */
                return(FALSE);
            else
                return(TRUE);

        case K9FXX_READ_1:              /* bit 5 and 6, Read busy(0) or ready(1) */
            return(TRUE);

        default:
            break;
    }

    return(FALSE);
}
/*********************************************************************//**
 * @brief       Read ID from external NAND FLASH memory
 * @param[in]   None
 * @return      ID value
 **********************************************************************/
uint32_t NandFlash_ReadId( void )
{
    uint8_t a, b, c, d;
    volatile uint8_t *pCLE;
    volatile uint8_t *pALE;
    volatile uint8_t *pDATA;

    pCLE  = K9F1G_CLE;
    pALE  = K9F1G_ALE;
    pDATA = K9F1G_DATA;

    *pCLE = K9FXX_READ_ID;
    *pALE = 0;

    a = *pDATA;
    b = *pDATA;
    d = *pDATA;
    c = *pDATA;

    return ((a << 24) | (b << 16) | (c << 8) | d);
}

/*********************************************************************//**
 * @brief       Erase the whole NAND FLASH memory block based on the
 *              block number
 * @param[in]   blockNum    number of block that will be erased, should
 *              be in range: 0 .. 1023
 * @return      Erase status, could be:
 *                  - TRUE: pass
 *                  - FALSE: failure
 **********************************************************************/
Bool NandFlash_BlockErase( uint32_t blockNum )
{
    volatile uint8_t *pCLE;
    volatile uint8_t *pALE;
    uint32_t rowAddr;

    pCLE  = K9F1G_CLE;
    pALE  = K9F1G_ALE;

    rowAddr = blockNum*NANDFLASH_PAGE_PER_BLOCK;

    *pCLE = K9FXX_BLOCK_ERASE_1;

    *pALE = (uint8_t)(rowAddr & 0x00FF);            /* column address low */

    *pALE = (uint8_t)((rowAddr & 0xFF00) >> 8); /* column address high */

    *pCLE = K9FXX_BLOCK_ERASE_2;

    NandFlash_WaitForReady();

    return(NandFlash_ReadStatus(K9FXX_BLOCK_ERASE_1));
}

/*********************************************************************//**
 * @brief       This routine is used to check if the block is valid or
 *              not.
 * @param[in]   None
 * @return      Checking status, could be:
 *                  - TRUE: all blocks are valid
 *                  - FALSE: invalid block is found, an initial invalid
 *                           table will be created
 **********************************************************************/
Bool NandFlash_ValidBlockCheck( void )
{
    uint32_t block, page;
    Bool retValue = TRUE;

    uint8_t data = 0;

    for ( block = 0; block < NANDFLASH_NUMOF_BLOCK; block++ )
    {
        InvalidBlockTable[block] = FALSE;
        for ( page = 0; page < 2; page++ )
        {
            /* Check column address 2048 at first page and second page */
            NandFlash_PageReadFromAddr(block, page, NANDFLASH_INVALIDBLOCK_CHECK_COLUMM, &data, 1);

            if(data != 0xFF)
            {
                // found invalid block number, mark block number in the invalid
                // block table
                InvalidBlockTable[block] = TRUE;

                //At least one block is invalid
                retValue = FALSE;
            }
        }
    }

    return(retValue);
}

/*********************************************************************//**
 * @brief       Write a full page of program into NAND flash based on the
 *              page number, write up to 2112 bytes of data at a time.
 * @param[in]   pageNum     number of page that will be programmed, should
 *              be in range: 0..63
 * @param[in]   blockNum    number of block that will be programmed, should
 *              be in range: 0..1023
 * @param[in]   bufPtr      pointer to the buffer that contain data will be
 *              programmed in flash memory
 * @param[in]   bSpareProgram   enable programming spare data 
 * @return      Program status, could be:
 *                  - TRUE: success
 *                  - FALSE: fail
 **********************************************************************/
Bool NandFlash_PageProgram( uint32_t blockNum, uint32_t pageNum, uint8_t *bufPtr, Bool bSpareProgram  )
{
    volatile uint8_t *pCLE;
    volatile uint8_t *pALE;
    volatile uint8_t *pDATA;
    uint32_t i, curRow, curColumm;
    uint16_t programSize = NANDFLASH_RW_PAGE_SIZE;

    pCLE  = K9F1G_CLE;
    pALE  = K9F1G_ALE;
    pDATA = K9F1G_DATA;

    curColumm = 0;
    curRow = blockNum*NANDFLASH_PAGE_PER_BLOCK + pageNum;
    
    if(bSpareProgram)
        programSize = NANDFLASH_PAGE_FSIZE;
    
    *pCLE = K9FXX_BLOCK_PROGRAM_1;

    *pALE =  (uint8_t)(curColumm & 0x000000FF);     /* column address low */

    *pALE = (uint8_t)((curColumm & 0x00000F00) >> 8);   /* column address high */

    *pALE = (uint8_t)((curRow & 0x00FF));    /* row address low */

    *pALE = (uint8_t)((curRow & 0xFF00) >> 8);    /* row address high */

    for ( i = 0; i < programSize; i++ )
    {
        *pDATA = *bufPtr++;
    }

    *pCLE = K9FXX_BLOCK_PROGRAM_2;

    NandFlash_WaitForReady();

    return( NandFlash_ReadStatus( K9FXX_BLOCK_PROGRAM_1 ) );
}

/*********************************************************************//**
 * @brief       Read the whole NAND FLASH memory page based on the
 *              page number, the data will be stored in the pointer
 *              to the buffer.
 * @param[in]   pageNum     number of page that will be read, should
 *              be in range: 0..63
 * @param[in]   blockNum    number of block that will be read, should
 *              be in range: 0..1023
 * @param[in]   bufPtr      pointer to the buffer that contain data will be
 *              read from flash memory
 * @return      Read status, could be:
 *                  - TRUE: success
 *                  - FALSE: fail
 **********************************************************************/
Bool NandFlash_PageRead( uint32_t blockNum, uint32_t pageNum, uint8_t *bufPtr )
{
    return ((NandFlash_PageReadFromBeginning(blockNum, pageNum, bufPtr) != 0) ? TRUE:FALSE);
}

/*********************************************************************//**
 * @brief       Read the whole NAND FLASH memory page based on the
 *              page number, the data will be stored in the pointer
 *              to the buffer.
 * @param[in]   pageNum     number of page that will be read, should
 *              be in range: 0..63
 * @param[in]   blockNum    number of block that will be read, should
 *              be in range: 0..1023
 * @param[in]   bufPtr      pointer to the buffer that contain data will be
 *              read from flash memory
 * @return      number of byte(s) read til the end of the page
 **********************************************************************/
int NandFlash_PageReadFromBeginning(uint32_t blockNum, uint32_t pageNum, uint8_t* bufPtr)
{
    return (NandFlash_PageReadFromAddr(blockNum, pageNum, 0, bufPtr, NANDFLASH_PAGE_FSIZE));
}

/*********************************************************************//**
 * @brief       Read the whole NAND FLASH memory page based on the
 *              page number, the data will be stored in the pointer
 *              to the buffer.
 * @param[in]   blockNum    number of block that will be read, should
 *                          be in range: 0..1023
 * @param[in]   pageNum     number of page that will be read, should
 *              be in range: 0..63
 * @param[in]   addrInPage  the address in NandFlash to be read,
 *                          calculated from the beginning of page
 * @param[in]   bufPtr      pointer to the buffer that contain data will be
 *                          read from flash memory
 * @param[in]   size    the number of byte(s) to be read and stored to the buffer
 * @return      number of byte(s) read til the end of the page
 **********************************************************************/
int NandFlash_PageReadFromAddr(uint32_t blockNum, uint32_t pageNum,
                                            uint32_t addrInPage, uint8_t* bufPtr, uint32_t size)
{
    volatile uint8_t *pCLE;
    volatile uint8_t *pALE;
    volatile uint8_t *pDATA;
    uint32_t i, curColumm, curRow;    

    i = 0;

    pCLE  = K9F1G_CLE;
    pALE  = K9F1G_ALE;
    pDATA = K9F1G_DATA;

    curColumm = addrInPage;
    curRow = blockNum*NANDFLASH_PAGE_PER_BLOCK + pageNum;

    *pCLE = K9FXX_READ_1;

    *pALE =  (uint8_t)(curColumm & 0x000000FF);     /* column address low */

    *pALE = (uint8_t)((curColumm & 0x00000F00) >> 8);   /* column address high */

    *pALE = (uint8_t)((curRow & 0x00FF));    /* row address low */

    *pALE = (uint8_t)((curRow & 0xFF00) >> 8);    /* row address high */

    *pCLE = K9FXX_READ_2;

    NandFlash_WaitForReady();

    //Get data from the current address in the page til the end of the page
    for ( i = 0; i < (NANDFLASH_PAGE_FSIZE - curColumm); i++ )
    {
        *bufPtr = *pDATA;

        bufPtr++;

        if((i + 1) >= size)
            break;
    }

    // Ok, return
    return i;
}

#endif /*_EMC*/
