/*
 * File      : drv_nand.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2012 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author          Notes
 * 2013-05-20     Bernard         the first version
 */

#ifndef __DRV_NANDFLASH_H__
#define __DRV_NANDFLASH_H__

#include <rtdevice.h>

/* SPI NAND Flash Command */
#define SPI_NAND_WRITE_ENABLE_CMD           0x06
#define SPI_NAND_WRITE_DISABLE_CMD          0x04
#define SPI_NAND_GET_FEATURE_CMD            0x0f
#define SPI_NAND_SET_FEATURE_CMD            0x1f
#define SPI_NAND_READ_TO_CACHE_CMD          0x13
#define SPI_NAND_READ_FROM_CACHE1_CMD       0x03
#define SPI_NAND_READ_FROM_CACHE2_CMD       0x0b
#define SPI_NAND_READ_ID_CMD                0x9f
#define SPI_NAND_PROGRAM_LOAD_CMD           0x02
#define SPI_NAND_PROGRAM_EXEC_CMD           0x10
#define SPI_NAND_PROGRAM_LOAD_RANDOM_CMD    0x84
#define SPI_NAND_ERASE_BLOCK_CMD            0xd8
#define SPI_NAND_RESET_CMD                  0xff

#define NAND_ID_GD5F1G				        0x00C8

/* nandflash confg */
#define PAGES_PER_BLOCK         64
#define PAGE_DATA_SIZE          2048
#define PAGE_OOB_SIZE           64
#define NAND_MARK_SPARE_OFFSET  4

int gd5f1g_init(void);

#endif /* __DRV_NANDFLASH_H__ */
