#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#include "bsp.h"
#if (_CUR_USING_NANDFLASH == _RUNNING_NANDFLASH_HY27UF081G2A)
#ifdef _EMC

#include "nandflash_hy27uf081g.h"

#include "lpc_emc.h"
#include "lpc_clkpwr.h"
#include "lpc_pinsel.h"

//#include <string.h>

/* nandflash confg */
#define PAGES_PER_BLOCK         64
#define PAGE_DATA_SIZE          2048
#define PAGE_OOB_SIZE           64

#define NAND_COMMAND            *((volatile unsigned char *) 0x81000000)
#define NAND_ADDRESS            *((volatile unsigned char *) 0x82000000)
#define NAND_DATA               *((volatile unsigned char *) 0x80000000)

uint8_t InvalidBlockTable[NANDFLASH_NUMOF_BLOCK];

static void system_delay(unsigned int cnt)
{
	cnt = cnt * 100;
    while(cnt--);
}

/*********************************************************************//**
 * @brief       Ready/Busy check, no timeout, basically, R/B bit should
 *              once to bail out from this routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void NandFlash_WaitForReady( void )
{
    unsigned int i;
    for (i = 0; i < 256; i++)
    {
        if (!(LPC_GPIO4->PIN & (1UL << 31)))
            break;  /* from high to low once */
    }

    while (!(LPC_GPIO4->PIN & (1UL << 31)))
        ;  /* from low to high once */
    return;
}

/*********************************************************************//**
 * @brief       Initialize external NAND FLASH memory
 * @param[in]   None
 * @return      None
 **********************************************************************/
void NandFlash_Init( void )
{
    LPC_EMC_TypeDef *pEMC = LPC_EMC;
    EMC_STATIC_MEM_Config_Type config;

    /**************************************************************************
    * Initialize EMC for NAND FLASH
    **************************************************************************/
    EMC_Init();
    pEMC->Control = EMC_Control_E;

    /**************************************************************************
    * Initialize EMC for NAND FLASH
    **************************************************************************/
    config.CSn = 0;
    config.AddressMirror = 0;
    config.ByteLane = 1;
    config.DataWidth = 8;
    config.ExtendedWait = 2;
    config.PageMode = 0;
    config.WaitWEn = EMC_StaticWaitWen_WAITWEN(0x1f/*2*/);
    config.WaitOEn = EMC_StaticWaitOen_WAITOEN(0/*2*/);
    config.WaitWr =EMC_StaticWaitwr_WAITWR(6);
    config.WaitPage = EMC_StaticwaitPage_WAITPAGE(0x1f);
    config.WaitRd = EMC_StaticWaitwr_WAITWR(0x1f);
    config.WaitTurn = EMC_StaticWaitTurn_WAITTURN(0x1f);
    StaticMem_Init(&config);

    // Init GPIO pin
    // PINSEL_ConfigPin(2, 21, 0);
    // FIO2DIR &= ~(1 << 21);
    LPC_IOCON->P4_31&=~0x07;
    LPC_GPIO4->DIR&=~(0x01UL<<31);
}

/*********************************************************************//**
 * @brief       Issue Reset command to NAND FLASH memory
 * @param[in]   None
 * @return      None
 **********************************************************************/
void NandFlash_Reset( void )
{
    NAND_COMMAND = NAND_CMD_RESET;
    system_delay(10000);
}

/*********************************************************************//**
 * @brief       Read status from NAND FLASH memory
 * @param[in]   Cmd command for read operation, should be:
 *                  -  NAND_CMD_SEQIN
 *                  -  NAND_CMD_ERASE1
 *                  -  NAND_CMD_READ3
 * @return      Status, could be:
 *              - TRUE: pass
 *              - FALSE: Failure
 **********************************************************************/
Bool NandFlash_ReadStatus(uint32_t Cmd)
{
    unsigned char value;

    NAND_COMMAND = NAND_CMD_STATUS;

    /* Wait until bit 5 and 6 are 1, READY, bit 7 should be 1 too, not protected */
    /* if ready bit not set, it gets stuck here */
    while ((NAND_DATA & 0xE0) != 0xE0);

	value = NAND_DATA;

    switch (Cmd)
    {
    case NAND_CMD_SEQIN:
    case NAND_CMD_ERASE1:
        if (value & 0x01)			/* Erase/Program failure(1) or pass(0) */
            return (FALSE);
        else
            return (TRUE);

    case NAND_CMD_READ3:            /* bit 5 and 6, Read busy(0) or ready(1) */
        return (TRUE);

    default:
        break;
    }

    return (FALSE);
}

/*********************************************************************//**
 * @brief       Read ID from external NAND FLASH memory
 * @param[in]   None
 * @return      ID value
 **********************************************************************/
uint32_t NandFlash_ReadId( void )
{
    uint32_t id = 0;

    NAND_COMMAND = NAND_CMD_READID;
    NAND_ADDRESS = 0;

    id |= NAND_DATA; // 0xAD
    id <<= 8;
    id |= NAND_DATA; // 0xF1
    id <<= 8;
    id |= NAND_DATA; // 0x80
    id <<= 8;
    id |= NAND_DATA; // 0x1D

    return id;
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
    unsigned int blockPage;

    blockPage = blockNum*NANDFLASH_PAGE_PER_BLOCK;

    NAND_COMMAND = NAND_CMD_ERASE1;                        /* send erase command */
    NAND_ADDRESS = blockPage & 0xff;
    NAND_ADDRESS = (blockPage>>8)&0xff;
    NAND_COMMAND = NAND_CMD_ERASE2;                        /* start erase */

    NandFlash_WaitForReady();
    if (NandFlash_ReadStatus(NAND_CMD_ERASE1) == FALSE)
    {
        NandFlash_Reset();
        return FALSE;
    }

    return TRUE;
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
Bool NandFlash_ValidBlockCheck(void)
{
    uint32_t block, page;
    Bool retValue = TRUE;

    uint8_t data[16];

    for (block = 0; block < NANDFLASH_NUMOF_BLOCK; block++)
    {
        for (page = 0; page < 2; page++)
        {
            /* Check column address 2048 at first page and second page */
            NandFlash_PageReadFromAddr(block, page, NANDFLASH_RW_PAGE_SIZE, data, 16);

            if (data[0] != 0xFF)
            {
                // found invalid block number, mark block number in the invalid
                // block table
                InvalidBlockTable[block] = 1;

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
	unsigned int i;
    uint32_t curRow;
    uint16_t programSize = NANDFLASH_RW_PAGE_SIZE;

    curRow = blockNum*NANDFLASH_PAGE_PER_BLOCK + pageNum;
    if(bSpareProgram)
        programSize = NANDFLASH_PAGE_FSIZE;

    NAND_COMMAND = NAND_CMD_SEQIN;

    NAND_ADDRESS = 0 & 0xFF;
    NAND_ADDRESS = 0 >> 8;
    NAND_ADDRESS = curRow & 0xFF;
    NAND_ADDRESS = curRow >> 8;

	for (i = 0; i < programSize; i ++)
		NAND_DATA = bufPtr[i];

    NAND_COMMAND = NAND_CMD_PAGEPROG;

    NandFlash_WaitForReady();

    if(NandFlash_ReadStatus(NAND_CMD_SEQIN) == FALSE)
    {
        NandFlash_Reset();

        return FALSE;
    }

    return TRUE;
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
    uint32_t i, curColumm, curRow;

    i = 0;

    curColumm = addrInPage;
    curRow = blockNum*NANDFLASH_PAGE_PER_BLOCK + pageNum;

    NAND_COMMAND = NAND_CMD_READ0;

    NAND_ADDRESS = curColumm & 0xFF;
    NAND_ADDRESS = curColumm >> 8;

    NAND_ADDRESS = curRow & 0xFF;
    NAND_ADDRESS = curRow >> 8;

    NAND_COMMAND = NAND_CMD_READ3;

    NandFlash_WaitForReady();

    //Get data from the current address in the page til the end of the page
    for ( i = 0; i < (NANDFLASH_PAGE_FSIZE - curColumm); i++ )
    {
        *bufPtr = NAND_DATA;

        bufPtr++;

        if((i + 1) >= size)
            break;
    }

    // Ok, return
    return i;
}

#endif /*_EMC*/
#endif /* (_CUR_USING_NANDFLASH == _RUNNING_NANDFLASH_HY27UF081G2A) */
