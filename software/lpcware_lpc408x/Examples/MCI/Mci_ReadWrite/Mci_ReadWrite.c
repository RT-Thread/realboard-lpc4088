/***
 * @file        Mci_ReadWrite.c
 * @purpose     This example describes how to work with Card Interface
 *              It supports to test read/write and see if they are ok
 * @version     1.0
 * @date        29. June. 2011
 * @author      NXP MCU SW Application Team
 *---------------------------------------------------------------------
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
/** @defgroup MCI_ReadWrite MCI Read Write
 * @ingroup MCI_Examples
 * @{
 */
 
#define DMA_SIZE        (1000UL)
#define DMA_SRC         LPC_PERI_RAM_BASE       /* This is the area original data is stored
                                        or data to be written to the SD/MMC card. */
#define DMA_DST         (DMA_SRC+DMA_SIZE)      /* This is the area, after writing to the SD/MMC,
                                        data read from the SD/MMC card. */

uint8_t mciRdWrMenu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" MCI Read Write Example: \n\r"
"\t - MCU: LPC \n\r"
"\t - Core: ARM CORTEX-M3/M4 \n\r"
"\t - UART Communicationi: 115200 bps \n\r"
" This example is used to test the Multimedia Card Interface (MCI) function.\n\r"
#if MCI_DMA_ENABLED
" It is able to use DMA to transfer data (read/write) with the card\n\r"
#else
" It is able transfer data (read/write) with the card directly via FIFO register\n\r"
#endif
"********************************************************************************\n\r";

#define WRITE_BLOCK_NUM  (4)
#define WRITE_LENGTH    (BLOCK_LENGTH*WRITE_BLOCK_NUM)
/* treat WriteBlock as a constant address */
volatile uint8_t *WriteBlock = (uint8_t *)(DMA_SRC);

/* treat ReadBlock as a constant address */
volatile uint8_t *ReadBlock  = (uint8_t *)(DMA_DST);


uint8_t wrBuf[WRITE_LENGTH]; 
uint8_t rdBuf[WRITE_LENGTH];


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
    uint32_t i, j;
    int32_t retVal = 0;
    uint8_t error = 0;

    st_Mci_CardId cidval;
    en_Mci_CardType cardType;
    uint32_t rcAddress;
    uint32_t csdVal[4];
    uint32_t errorState;

    // Initialize buffers for testing later
    
    for(i = 0; i < WRITE_LENGTH; i++)
    {
        wrBuf[i] = i / (4*WRITE_BLOCK_NUM);
        rdBuf[i] = 0;
    }

    debug_frmwrk_init();

    _DBG_("");_DBG(mciRdWrMenu);_DBG_("");

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
        _DBG("\t- Product Serial Number: ");_DBH32(cidval.PSN);_DBG_("");
    }

    /*---- Card is 'ident' state ----*/
    retVal = MCI_SetCardAddress();
    if(retVal != MCI_FUNC_OK)
    {
        _DBG("Set Card Address is FAILED, retVal = "); _DBH32(retVal);
        while(1);
    }
    else
    {
        rcAddress = MCI_GetCardAddress();
        _DBG("Set CARD ADDRESS OK with address "); _DBH32(rcAddress);
    }

    _DBG_("");

    retVal = MCI_GetCSD(csdVal);
    if(retVal != MCI_FUNC_OK)
    {
        _DBG("Get CSD FAILED, retVal = "); _DBH32(retVal);
        while(1);
    }
    else
    {
        _DBG_("Get Card Specific Data (CSD) Ok:");
        _DBG("\t[0] = "); _DBH32(csdVal[0]);_DBG_("");
        _DBG("\t[1] = "); _DBH32(csdVal[1]);_DBG_("");
        _DBG("\t[2] = "); _DBH32(csdVal[2]);_DBG_("");
        _DBG("\t[3] = "); _DBH32(csdVal[3]);_DBG_("");
    }
    
    retVal = MCI_Cmd_SelectCard();

    if(retVal != MCI_FUNC_OK)
    {
        _DBG("Card Selection is FAILED, retVal = "); _DBH32(retVal);
        while(1);
    }
    else
    {
        _DBG("Card has been selected successfully!!!\n\r");
    }

    if(cardType == MCI_SDSC_V1_CARD||
        cardType == MCI_SDSC_V2_CARD||
        cardType == MCI_SDHC_SDXC_CARD)
    {
        MCI_Set_MCIClock( MCI_NORMAL_RATE );
        
        if (MCI_SetBusWidth( SD_4_BIT ) != MCI_FUNC_OK )
        {
            _DBG("Set BandWidth is FAILED, retVal = "); _DBH32(retVal);
            while (1);  /* fatal error */
        }
        else
        {
            _DBG("SET Bandwidth!!!\n\r");
        }
    }

    retVal = MCI_SetBlockLen(BLOCK_LENGTH);
    if(retVal != MCI_FUNC_OK)
    {
        _DBG("Set Block Length is FAILED, retVal = "); _DBH32(retVal);
        while(1);
    }
    else
    {
        _DBG("Block Length is SET successfully!!!\n\r");
    }

    retVal = MCI_WriteBlock(wrBuf, 0, WRITE_BLOCK_NUM);
    if(retVal != MCI_FUNC_OK)
    {
        _DBG("Write Block is FAILED, retVal = "); _DBH32(retVal);
        while(1);
    }
    else
    {
        //while(MCI_GetBlockXferEndState() != 0);
        while(MCI_GetDataXferEndState() != 0);
        errorState = MCI_GetXferErrState();     
        if((WRITE_BLOCK_NUM > 1) || errorState);
        {
            MCI_Cmd_StopTransmission();
        }
        
        if(errorState)
        {
            _DBG("Write ");_DBD(WRITE_BLOCK_NUM);_DBG(" Failed (");_DBH32(errorState);_DBG_(")");
        }
        else
        {
            _DBG("Write ");_DBD(WRITE_BLOCK_NUM);_DBG(" Blocks successfully!!!\n\r");
        }
    }

    // Delay 500ms
    for ( i = 0; i < 0x500000; i++ );

    retVal = MCI_ReadBlock(rdBuf, 0, WRITE_BLOCK_NUM);
    if(retVal != MCI_FUNC_OK)
    {
        _DBG("Read Block is FAILED, retVal = "); _DBH32(retVal);
        while(1);
    }
    else
    {
        //while(MCI_GetBlockXferEndState() != 0);
        while(MCI_GetDataXferEndState() != 0);
        errorState = MCI_GetXferErrState();     
        if((WRITE_BLOCK_NUM > 1) || errorState);
        {
            MCI_Cmd_StopTransmission();
        }
        
        if(errorState)
        {
            _DBG("Read ");_DBD(WRITE_BLOCK_NUM);_DBG(" Failed (");_DBH32(errorState);_DBG_(")");
        }
        else
        {
            _DBG("Read ");_DBD(WRITE_BLOCK_NUM);_DBG(" Blocks successfully!!!\n\r");
        }
    }

    retVal = MCI_FUNC_OK;

    for (j = 0; j < WRITE_LENGTH; j++)
    {
        if(rdBuf[j] != wrBuf[j])
        {
            _DBG("ERROR on Read and Write at position: "); _DBH32(j);
            retVal = MCI_FUNC_FAILED;
            break;
        }
    }

    if(retVal == MCI_FUNC_OK)
    {
        _DBG("CHECKING is done! Read and Write correct!!!\n\r");
    }

    _DBG("\n\r>>> EXAMPLES is ENDED ");

    while(1);

}
int main(void)
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
 
