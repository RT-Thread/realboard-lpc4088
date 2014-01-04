/**********************************************************************
* $Id$      Mci_CidCard.c   2011-06-02
*//**
* @file     Mci_CidCard.c
* @brief    This example describes how to test MCI card
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

#include "LPC407x_8x_177x_8x.h"
#include "lpc_types.h"
#include "lpc_mci.h"
#include "lpc_gpdma.h"
#include "debug_frmwrk.h"
#include "bsp.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup MCI_CidCard   MCI Card Identifier
 * @ingroup MCI_Examples
 * @{
 */

#define DMA_SIZE       (1000UL)

#define DMA_SRC         LPC_PERI_RAM_BASE       /* This is the area original data is stored
                                        or data to be written to the SD/MMC card. */
#define DMA_DST         (DMA_SRC+DMA_SIZE)      /* This is the area, after writing to the SD/MMC,
                                        data read from the SD/MMC card. */

uint8_t mciCidCardMenu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" MCI CID Card \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communicationi: 115200 bps \n\r"
" This example is used to test the Multimedia Card Interface (MCI) function.\n\r"
" It is able to check, show the CID that retrieved from the card\n\r"
"********************************************************************************\n\r";


/* treat WriteBlock as a constant address */
volatile uint8_t *WriteBlock = (uint8_t *)(DMA_SRC);

/* treat ReadBlock as a constant address */
volatile uint8_t *ReadBlock  = (uint8_t *)(DMA_DST);

#if MCI_DMA_ENABLED
/******************************************************************************
**  DMA Handler
******************************************************************************/
void DMA_IRQHandler (void)
{
   MCI_DMA_IRQHandler();
}
#endif

/******************************************************************************
**   Main Function  main()
******************************************************************************/
void c_entry (void)
{
    uint8_t error = 0;
    st_Mci_CardId cidval;
    en_Mci_CardType cardType;

    debug_frmwrk_init();

    _DBG_("");_DBG(mciCidCardMenu);_DBG_("");

#if MCI_DMA_ENABLED
    /* on DMA channel 0, source is memory, destination is MCI FIFO. */
    /* On DMA channel 1, source is MCI FIFO, destination is memory. */
    GPDMA_Init();
#endif

    /* For the SD card I tested, the minimum required block length is 512 */
    /* For MMC, the restriction is loose, due to the variety of SD and MMC
    card support, ideally, the driver should read CSD register to find the
    right speed and block length for the card, and set them accordingly.
    In this driver example, it will support both MMC and SD cards, and it
    does read the information by send SEND_CSD to poll the card status,
    however, to simplify the example, it doesn't configure them accordingly
    based on the CSD register value. This is not intended to support all
    the SD and MMC cards. */

    if(MCI_Init(BRD_MCI_POWERED_ACTIVE_LEVEL) != MCI_FUNC_OK)
    {
        _DBG_("MCI_Init FAILED");

        while( 1 );         /* fatal error */
    }

    cardType = MCI_GetCardType();

    switch (cardType)
    {
        case MCI_SDHC_SDXC_CARD:
            _DBG_("Currently the SDXC/SDHC CARD ver2.0 is being used");
            break;
        case MCI_SDSC_V2_CARD:
            _DBG_("Currently the SD CARD ver2.0 is being used");
            break;
        case MCI_SDSC_V1_CARD:
            _DBG_("Currently the SD CARD ver1.0 is being used");
            break;

        case MCI_MMC_CARD:
            _DBG_("Currently the MMC CARD is being used");
            break;

        case MCI_CARD_UNKNOWN:
            _DBG_("No CARD is being plugged, Please check!!!");
            error = 1;
            break;
    }
    if(error)
        while(1);

    if (MCI_GetCID(&cidval) != MCI_FUNC_OK)
    {
        _DBG_("Get CID Failed");

        while ( 1 );        /* fatal error */
    }
    else
    {
        _DBG("\n\r\t- Manufacture ID: ");_DBH32(cidval.MID);_DBG_("");
        _DBG("\n\r\t- OEM/Application ID: ");_DBH32(cidval.OID);_DBG_("");
        _DBG("\n\r\t- Product Name: ");_DBH(cidval.PNM_H);_DBH32_(cidval.PNM_L);_DBG_("");
        _DBG("\n\r\t- Product Revision: ");_DBH32(cidval.PRV);_DBG_("");
        _DBG("\n\r\t- Product Serial Number: ");_DBH32(cidval.PSN);_DBG_("");
        _DBG("\n\r\t- Manufacturing Date: ");_DBH32(cidval.MDT);_DBG_("");
    }

    while(1);

}
int main (void)
{
  c_entry();
  return 0;
}

/******************************************************************************
**                            End Of File
******************************************************************************/

/**
 * @}
 */

