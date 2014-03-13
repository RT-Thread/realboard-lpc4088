/*
 * File      : drv_nand.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2008 - 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author          Notes
 * 2013-05-22     Bernard         the first version
 */

#include "board.h"
#include <rtdevice.h>

#include "drv_gd5f1g.h"
#include <string.h>

#ifdef RT_USING_MTD_NAND

#ifndef RT_USING_LOGTRACE
#define log_trace(...)
#define log_trace_register_session(...)
#else
#include <log_trace.h>

#define NAND_MODULE "[NAND]"
static struct log_trace_session _nand_session = {{"NAND"}, LOG_TRACE_LEVEL_INFO};
#endif

#define NAND_SPI_ER_STATUS_P_FAIL       (1 << 3)
#define NAND_SPI_ER_STATUS_E_FAIL       (1 << 2)
#define NAND_SPI_ER_STATUS_WEL          (1 << 1)
#define NAND_SPI_ER_STATUS_OIP          (1 << 0)

struct gd_nand
{
    struct rt_spi_device *spi;       /* SPI device */
    struct rt_mutex lock;            /* mutex */
    rt_uint8_t  id[2];
};
static struct gd_nand _nand;

static int spi_nand_get_feature(struct rt_spi_device *spi, int reg, uint8_t *data)
{
    uint8_t txbuf[2];
    uint8_t rxbuf[1];

    txbuf[0] = SPI_NAND_GET_FEATURE_CMD;
    txbuf[1] = reg;
    rt_spi_send_then_recv(spi, txbuf, 2, rxbuf, 1);
    *data = rxbuf[0];

    return 0;
}

static int nandflash_busy_wait(struct rt_spi_device *spi, uint8_t *data)
{
    int i;

    for (i = 0; i < 4000; i++)
    {
        spi_nand_get_feature(spi, 0xC0, data);
        if (!(*data & NAND_SPI_ER_STATUS_OIP))
            break;
    }

    return 0;
}

/**
 * @brief Check for ID
 * @param device: Pointer to MTD_NAND device
 * @return RT_MTD_EOK - The nand flash is GD5F1G
 *         RT_MTD_EIO - can not support this flash
 */
static rt_err_t nandflash_readid(struct rt_mtd_nand_device *device)
{
    uint8_t txbuf[3];
    struct rt_mutex *lock;
    struct rt_spi_device *spi;

    lock = &_nand.lock;
    spi = _nand.spi;
    RT_ASSERT(spi != RT_NULL);

    rt_mutex_take(lock, RT_WAITING_FOREVER);
    txbuf[0] = SPI_NAND_READ_ID_CMD;
    txbuf[1] = 0x00;
    rt_spi_send_then_recv(spi, txbuf, 2, _nand.id, 2);
    rt_mutex_release(lock);

    rt_kprintf("NAND ID: 0x%02X%02X\n", _nand.id[0], _nand.id[1]);
    if (_nand.id[0] != 0xC8 && _nand.id[1] != 0xF1)
    {
        return -RT_MTD_EIO;
    }

    return RT_MTD_EOK;
}

static rt_err_t nandflash_unlock(struct rt_mtd_nand_device *device)
{
    uint8_t txbuf[3];
    struct rt_mutex *lock;
    struct rt_spi_device *spi;

    lock = &_nand.lock;
    spi = _nand.spi;
    RT_ASSERT(spi != RT_NULL);

    rt_mutex_take(lock, RT_WAITING_FOREVER);
    txbuf[0] = SPI_NAND_SET_FEATURE_CMD;
    txbuf[1] = 0xA0;
    txbuf[2] = 0x00;
    rt_spi_send(spi, txbuf, 3);
    rt_mutex_release(lock);

    return RT_MTD_EOK;
}

/**
 * @brief Read a page from nand flash
 * @param device: Pointer to MTD_NAND device
 * @param page: Relative page offset of a logical partition
 * @param data: Buffer of main data
 * @param data_len: The length you want to read
 * @param spare: Buffer of spare data
 * @param spare_len: The length you want to read
 * @return RT_MTD_EOK - No error occurs
 */
static rt_err_t nandflash_readpage(struct rt_mtd_nand_device *device,
                                   rt_off_t                   page,
                                   rt_uint8_t                *data,
                                   rt_uint32_t                data_len,
                                   rt_uint8_t                *spare,
                                   rt_uint32_t                spare_len)
{
    rt_err_t result;
    struct rt_mutex *lock;
    uint8_t txbuf[4], stat;
    struct rt_spi_device *spi;
    struct rt_spi_message message[3];

    result = RT_MTD_EOK;
    spi = _nand.spi;
    RT_ASSERT(spi != RT_NULL);
    lock = &(_nand.lock);

    page = page + device->block_start * device->pages_per_block;
    if (page / device->pages_per_block > device->block_end)
    {
        return -RT_MTD_EIO;
    }

    log_trace(LOG_TRACE_DEBUG NAND_MODULE "nand_read: block %d page %d, data len %d, spare len %d\n",
              page / device->pages_per_block, page % device->pages_per_block,
              data == RT_NULL ? 0 : data_len,
              spare == RT_NULL ? 0 : spare_len);
    rt_mutex_take(lock, RT_WAITING_FOREVER);

    /* Load the appropriate page */
    txbuf[0] = SPI_NAND_READ_TO_CACHE_CMD;
    txbuf[1] = 0x00;
    txbuf[2] = page >> 8;
    txbuf[3] = page & 0xFF;

    message[0].send_buf   = txbuf;
    message[0].recv_buf   = RT_NULL;
    message[0].length     = 4;
    message[0].cs_take    = 1;
    message[0].cs_release = 1;
    message[0].next       = RT_NULL;
    rt_spi_transfer_message(spi, &message[0]);

    /* Wait */
    nandflash_busy_wait(spi, &stat);
    if ((stat & NAND_SPI_ER_STATUS_OIP))
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"chip is in busy or nonresponsive stat=%02x\n", stat);
        /* Chip is stuck? */
        rt_mutex_release(lock);
        return -RT_MTD_EIO;
    }

    if (data)
    {
        /* Check the ECC bits */
        stat >>= 4;
        if ((stat & 0x3 == 1) || (stat & 0x3 == 3))
        {
            /* ECC recovered, it's OK */
            result = RT_MTD_EOK;
        }
        if (stat == 2)
        {
            log_trace(LOG_TRACE_ERROR NAND_MODULE"ECC failed when reading, page=%x, %d:%d\n",
                      page,
                      page % device->pages_per_block,
                      page / device->pages_per_block);
            rt_mutex_release(lock);
            return -RT_MTD_EIO;
        }

        txbuf[0] = SPI_NAND_READ_FROM_CACHE1_CMD;
        txbuf[1] = 0 >> 8;
        txbuf[2] = 0 & 0xFF;
        txbuf[3] = 0;

        message[0].send_buf   = txbuf;
        message[0].recv_buf   = RT_NULL;
        message[0].length     = 4;
        message[0].cs_take    = 1;
        message[0].cs_release = 0;
        message[0].next       = &message[1];

        message[1].send_buf   = RT_NULL;
        message[1].recv_buf   = data;
        message[1].length     = data_len;
        message[1].cs_take    = 0;
        message[1].cs_release = 0;
        if (spare != RT_NULL)
        {
            message[1].next   = &message[2];

            message[2].send_buf   = RT_NULL;
            message[2].recv_buf   = spare;
            message[2].length     = spare_len;
            message[2].cs_take    = 0;
            message[2].cs_release = 1;
            message[2].next       = RT_NULL;
        }
        else
        {
            message[1].cs_release = 1;
            message[1].next   = RT_NULL;
        }
        /* transfer message */
        rt_spi_transfer_message(spi, &message[0]);
    }
    else if (spare != RT_NULL)
    {
        txbuf[0] = SPI_NAND_READ_FROM_CACHE1_CMD;
        txbuf[1] = 0x8 | ((2048) >> 8);
        txbuf[2] = (2048) & 0xFF;
        txbuf[3] = 0;

        message[0].send_buf   = txbuf;
        message[0].recv_buf   = RT_NULL;
        message[0].length     = 4;
        message[0].cs_take    = 1;
        message[0].cs_release = 0;
        message[0].next       = &message[1];

        message[1].send_buf   = RT_NULL;
        message[1].recv_buf   = spare;
        message[1].length     = spare_len;
        message[1].cs_take    = 0;
        message[1].cs_release = 1;
        message[1].next       = RT_NULL;

        /* transfer message */
        rt_spi_transfer_message(spi, &message[0]);
    }

    rt_mutex_release(lock);
    return (result);
}

/**
 * @brief Write a page to nand flash
 * @param device: Pointer to MTD_NAND device
 * @param page: Relative page offset of a logical partition
 * @param data: Buffer of main data
 * @param data_len: The length you want to write
 * @param spare: Buffer of spare data
 * @param spare_len: The length you want to write
 * @return RT_MTD_EOK - No error occurs
 *         -RT_MTD_EIO - Programming fail
 */
rt_err_t nandflash_writepage(struct rt_mtd_nand_device *device,
                             rt_off_t                   page,
                             const rt_uint8_t          *data,
                             rt_uint32_t                data_len,
                             const rt_uint8_t          *spare,
                             rt_uint32_t                spare_len)
{
    rt_err_t result;
    uint8_t txbuf[4], stat;
    uint8_t oob_buffer[64];
    struct rt_spi_device *spi;
    struct rt_mutex *lock;
    struct rt_spi_message message[3];

    result = RT_MTD_EOK;
    lock = &(_nand.lock);
    spi = _nand.spi;
    RT_ASSERT(spi != RT_NULL);

    RT_ASSERT(data_len <= PAGE_DATA_SIZE);
    RT_ASSERT(spare_len <= PAGE_OOB_SIZE);

    if (data == RT_NULL && spare == RT_NULL) return -RT_MTD_EIO;

    page = page + device->block_start * device->pages_per_block;
    if (page / device->pages_per_block > device->block_end)
    {
        return -RT_MTD_EIO;
    }

    log_trace(LOG_TRACE_DEBUG NAND_MODULE"nand_write: block %d page %d, data len %d, spare len %d\n",
              page / device->pages_per_block, page % device->pages_per_block,
              data == RT_NULL ? 0 : data_len,
              spare == RT_NULL ? 0 : spare_len);

    rt_mutex_take(lock, RT_WAITING_FOREVER);

    /* set oob buffer */
    memset(oob_buffer, 0xff, sizeof(oob_buffer));
    if (spare != RT_NULL) memcpy(oob_buffer, spare, spare_len);

    txbuf[0] = SPI_NAND_PROGRAM_LOAD_CMD;
    txbuf[1] = 0 >> 8;
    txbuf[2] = 0 & 0xFF;

    message[0].send_buf   = txbuf;
    message[0].recv_buf   = RT_NULL;
    message[0].length     = 3;
    message[0].cs_take    = 1;
    message[0].cs_release = 0;
    message[0].next       = &message[1];

    message[1].send_buf   = data;
    message[1].recv_buf   = RT_NULL;
    message[1].length     = 2048;
    message[1].cs_take    = 0;
    message[1].cs_release = 0;
    message[1].next       = &message[2];

    message[2].send_buf   = oob_buffer;
    message[2].recv_buf   = RT_NULL;
    message[2].length     = 64;
    message[2].cs_take    = 0;
    message[2].cs_release = 1;
    message[2].next       = RT_NULL;

    /* transfer message */
    rt_spi_transfer_message(spi, &message[0]);

    /* Write enable */
    txbuf[0] = SPI_NAND_WRITE_ENABLE_CMD;
    rt_spi_send(spi, txbuf, 1);

    /* Program execute */
    txbuf[0] = SPI_NAND_PROGRAM_EXEC_CMD;
    txbuf[1] = 0x00;
    txbuf[2] = page >> 8;
    txbuf[3] = page & 0xFF;
    rt_spi_send(spi, txbuf, 4);

    /* Wait */
    nandflash_busy_wait(spi, &stat);
    if ((stat & NAND_SPI_ER_STATUS_OIP))
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"chip is busy or nonresponsive stat=%02x\n", stat);
        result = -RT_MTD_EBUSY;
    }

    if (stat & (2 << 3))
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"EIO error in write.\n");
        result = -RT_MTD_EIO;
    }

    rt_mutex_release(lock);

    return result;
}

/**
 * @brief Erase a block
 * @param device: Pointer to MTD_NAND device
 * @param block: Relative block offset of a logical partition
 * @return RT_MTD_EOK - Erase successfully
 *         -RT_MTD_EIO - Erase fail
 */
static rt_err_t nandflash_eraseblock(struct rt_mtd_nand_device *device,
                                     rt_uint32_t                block)
{
    rt_err_t result;
    uint8_t txbuf[4];
    uint8_t stat;
    rt_uint32_t page;
    struct rt_spi_device *spi;
    struct rt_mutex *lock;

    result = RT_MTD_EOK;
    lock = &(_nand.lock);
    spi = _nand.spi;
    RT_ASSERT(spi != RT_NULL);

    /* add the start blocks and get page */
    block = block + device->block_start;
    page = block * PAGES_PER_BLOCK;

    rt_mutex_take(lock, RT_WAITING_FOREVER);

    /* Write enable */
    txbuf[0] = SPI_NAND_WRITE_ENABLE_CMD;
    rt_spi_send(spi, txbuf, 1);

    /* Block erase */
    txbuf[0] = SPI_NAND_ERASE_BLOCK_CMD;
    txbuf[1] = 0x00;
    txbuf[2] = page >> 8;
    txbuf[3] = page & 0xFF;
    rt_spi_send(spi, txbuf, 4);

    /* Wait */
    nandflash_busy_wait(spi, &stat);
    /* Check status */
    if (stat & NAND_SPI_ER_STATUS_OIP)
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"block %d:chip is busy or nonresponsive stat=%02x\n",
                  block, stat);
        result = -RT_MTD_EBUSY;
    }
    if (stat & NAND_SPI_ER_STATUS_E_FAIL)
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"block %d: erase failed.\n", block);
        result = -RT_MTD_EIO;
    }

    rt_mutex_release(lock);

    return (result);
}

static rt_err_t nandflash_pagecopy(struct rt_mtd_nand_device *device,
                                   rt_off_t                   src_page,
                                   rt_off_t                   dst_page)
{
    rt_err_t result;
    uint8_t txbuf[4];
    uint8_t stat;
    struct rt_spi_device *spi;
    struct rt_mutex *lock;

    result = RT_MTD_EOK;
    lock = &(_nand.lock);
    spi = _nand.spi;
    RT_ASSERT(spi != RT_NULL);

    src_page = src_page + device->block_start * device->pages_per_block;
    dst_page = dst_page + device->block_start * device->pages_per_block;

    rt_mutex_take(lock, RT_WAITING_FOREVER);

    /* Load the appropriate page */
    txbuf[0] = SPI_NAND_READ_TO_CACHE_CMD;
    txbuf[1] = 0x00;
    txbuf[2] = src_page >> 8;
    txbuf[3] = src_page & 0xFF;
    rt_spi_send(spi, txbuf, 4);

    /* Wait */
    nandflash_busy_wait(spi, &stat);
    if ((stat & NAND_SPI_ER_STATUS_OIP))
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"chip is busy or nonresponsive stat=%02x\n", stat);

        /*
         * Chip is stuck?
         */
        rt_mutex_release(lock);
        return -RT_MTD_EBUSY;
    }

    /* Check the ECC bits */
    stat >>= 4;
    if (stat == 1)
    {
        /* ECC recovered, it's OK */
        result = RT_MTD_EOK;
    }
    if (stat == 2)
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"failed ECC, page=%x\n", src_page);
        rt_mutex_release(lock);
        return -RT_MTD_EIO;
    }

    /* Write enable */
    txbuf[0] = SPI_NAND_WRITE_ENABLE_CMD;
    rt_spi_send(spi, txbuf, 1);

    /* Program execute */
    txbuf[0] = SPI_NAND_PROGRAM_EXEC_CMD;
    txbuf[1] = 0x00;
    txbuf[2] = dst_page >> 8;
    txbuf[3] = dst_page & 0xFF;
    rt_spi_send(spi, txbuf, 4);

    /* Wait */
    nandflash_busy_wait(spi, &stat);
    if ((stat & NAND_SPI_ER_STATUS_OIP))
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"chip is busy or nonresponsive stat=%02x\n", stat);
        result = RT_MTD_EBUSY;
    }

    if (stat & (1 << 3))
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"EIO error on page copy.\n");
        result = -RT_MTD_EIO;
    }

    rt_mutex_release(lock);
    return (result);
}

static const struct rt_mtd_nand_driver_ops ops =
{
    nandflash_readid,
    nandflash_readpage,
    nandflash_writepage,
    nandflash_pagecopy,
    nandflash_eraseblock,
    RT_NULL,
    RT_NULL,
};
struct rt_mtd_nand_device _partition[1];

int gd5f1g_init(void)
{
    struct rt_spi_device *spi;
    struct rt_spi_configuration cfg;

    spi = (struct rt_spi_device *)rt_device_find("spi00");
    if (spi == RT_NULL)
    {
        log_trace(LOG_TRACE_ERROR NAND_MODULE"spi device %s not found!\n", "spi00");
        return -RT_ERROR;
    }

    log_trace_register_session(&_nand_session);

    /* config spi */
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MODE_0 | RT_SPI_MSB; /* SPI Compatible: Mode 0 and Mode 3 */
    cfg.max_hz = 18000000; /* 20M for test. */
    rt_spi_configure(spi, &cfg);

    _nand.spi = spi;
    rt_mutex_init(&_nand.lock, "nand", RT_IPC_FLAG_FIFO);

    if (nandflash_readid(&_partition[0]) == RT_MTD_EOK)
    {
        /* register nand0 */
        _partition[0].page_size       = PAGE_DATA_SIZE;
        _partition[0].pages_per_block = PAGES_PER_BLOCK;
        _partition[0].plane_num       = 1;
        _partition[0].oob_size        = PAGE_OOB_SIZE;
        _partition[0].oob_free        = PAGE_OOB_SIZE - 16;
        _partition[0].block_start     = 0;
        _partition[0].block_end       = 1024;

        _partition[0].block_total     = _partition[0].block_end - _partition[0].block_start;
        _partition[0].ops             = &ops;

        rt_mtd_nand_register_device("nand1", &_partition[0]);
        /* unlock chip */
        nandflash_unlock(&_partition[0]);

        return RT_EOK;
    }

    return -RT_EIO;
}

#endif /* RT_USING_MTD_NAND */
