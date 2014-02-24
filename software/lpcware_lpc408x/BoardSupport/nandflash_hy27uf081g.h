#ifndef __DRV_HY27UF081G_H__
#define __DRV_HY27UF081G_H__

#include "lpc_types.h"

// total 1024 blocks in a device
#define NANDFLASH_NUMOF_BLOCK       1024

// total pages in a block
#define NANDFLASH_PAGE_PER_BLOCK    64

#define NANDFLASH_RW_PAGE_SIZE      2048        // 2048 bytes/page

#define NANDFLASH_SPARE_SIZE        64          //bytes/page

#define NANDFLASH_PAGE_FSIZE        (NANDFLASH_RW_PAGE_SIZE + NANDFLASH_SPARE_SIZE)

#define NANDFLASH_BLOCK_RWSIZE  (NANDFLASH_RW_PAGE_SIZE * NANDFLASH_PAGE_PER_BLOCK)
#define NANDFLASH_BLOCK_FSIZE   (NANDFLASH_PAGE_FSIZE * NANDFLASH_PAGE_PER_BLOCK)

#define NANDFLASH_ADDR_COLUMM_POS       0
#define NANDFLASH_ADDR_ROW_POS          16

#define HY271G_ID                    0xADF10000  /* Byte 3 and 2 only */

/* NAND command */
#define NAND_CMD_READ0     0x00
#define NAND_CMD_READ1     0x01
#define NAND_CMD_PAGEPROG  0x10
#define NAND_CMD_READ3     0x30
#define NAND_CMD_READ_CB   0x35
#define NAND_CMD_READOOB   0x50
#define NAND_CMD_ERASE1    0x60
#define NAND_CMD_STATUS    0x70
#define NAND_CMD_SEQIN     0x80
#define NAND_CMD_CB_PROG   0x85
#define NAND_CMD_READID    0x90
#define NAND_CMD_READID1   0x91
#define NAND_CMD_ERASE2    0xd0
#define NAND_CMD_RESET     0xff

extern void NandFlash_Init( void );
extern void NandFlash_Reset( void );
extern void NandFlash_WaitForReady( void ); /* same as CheckBusy, no time out */
extern uint32_t NandFlash_ReadId( void );
extern Bool NandFlash_ReadStatus( uint32_t Cmd );
extern Bool NandFlash_BlockErase( uint32_t blockNum );
extern Bool NandFlash_ValidBlockCheck( void );
extern Bool NandFlash_PageProgram( uint32_t blockNum, uint32_t pageNum, uint8_t *bufPtr , Bool bSpareProgram);
extern Bool NandFlash_PageRead( uint32_t blockNum, uint32_t pageNum, uint8_t *bufPtr );

extern int NandFlash_PageReadFromBeginning(uint32_t blockNum, uint32_t pageNum, uint8_t* bufPtr);
extern int NandFlash_PageReadFromAddr(uint32_t blockNum, uint32_t pageNum,
                                                    uint32_t addrInPage, uint8_t* bufPtr, uint32_t size);
#endif
