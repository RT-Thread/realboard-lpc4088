#include "drv_hy27uf081g.h"

#include <nftl.h>
#include "lpc_emc.h"
#include "lpc_clkpwr.h"
#include "lpc_pinsel.h"

#include <rtdevice.h>
#include <string.h>

/* nandflash confg */
#define PAGES_PER_BLOCK         64
#define PAGE_DATA_SIZE          2048
#define PAGE_OOB_SIZE           64

#define NAND_COMMAND            *((volatile rt_uint8_t *) 0x81000000)
#define NAND_ADDRESS            *((volatile rt_uint8_t *) 0x82000000)
#define NAND_DATA               *((volatile rt_uint8_t *) 0x80000000)

static void system_delay(unsigned int cnt)
{
	cnt = cnt * 100;
    while(cnt--);
}

static void wait_for_ready(void)
{
    rt_uint32_t i;
    for (i = 0; i < 256; i++)
    {
        if (!(LPC_GPIO4->PIN & (1UL << 31)))
            break;  /* from high to low once */
    }

    while (!(LPC_GPIO4->PIN & (1UL << 31)));  /* from low to high once */
    return;
}

static void nand_reset(void)
{
    NAND_COMMAND = NAND_CMD_RESET;
    system_delay(10000);

    return;
}

static rt_bool_t read_status(rt_uint8_t cmd)
{
    rt_uint8_t value;

    NAND_COMMAND = NAND_CMD_STATUS;

    /* Wait until bit 5 and 6 are 1, READY, bit 7 should be 1 too, not protected */
    /* if ready bit not set, it gets stuck here */
    while ((NAND_DATA & 0xE0) != 0xE0);

	value = NAND_DATA;

    switch (cmd)
    {
    case NAND_CMD_SEQIN:
    case NAND_CMD_ERASE1:
        if (value & 0x01)  			/* Erase/Program failure(1) or pass(0) */
            return (RT_FALSE);
        else
            return (RT_TRUE);

    case NAND_CMD_READ3:            /* bit 5 and 6, Read busy(0) or ready(1) */
        return (RT_TRUE);

    default:
        break;
    }

    return (RT_FALSE);
}

static rt_err_t nand_hy27uf_readid(struct rt_mtd_nand_device *device)
{
    rt_uint8_t a, b;

    NAND_COMMAND = NAND_CMD_READID;
    NAND_ADDRESS = 0;

    a = NAND_DATA;
    b = NAND_DATA;

	rt_kprintf("NAND ID: 0x%02X%02X\n", a, b);

    return RT_EOK;
}

static rt_err_t nand_hy27uf_readpage(struct rt_mtd_nand_device *device,
                                   rt_off_t                   page,
                                   rt_uint8_t                *data,
                                   rt_uint32_t                data_len,
                                   rt_uint8_t                *spare,
                                   rt_uint32_t                spare_len)
{
    rt_uint32_t i;
    rt_err_t result = RT_MTD_EOK;
	rt_uint8_t oob_buffer[PAGE_OOB_SIZE];

    page = page + device->block_start * device->pages_per_block;
    if (page/device->pages_per_block > device->block_end)
    {
        return -RT_MTD_EIO;
    }

    if (data != RT_NULL && data_len != 0)
    {
        NAND_COMMAND = NAND_CMD_READ0;
        NAND_ADDRESS = 0 & 0xFF;
        NAND_ADDRESS = 0 >> 8;
        NAND_ADDRESS = page & 0xFF;
        NAND_ADDRESS = page >> 8;
        NAND_COMMAND = NAND_CMD_READ3;

        wait_for_ready();
		for (i = 0; i < PAGE_DATA_SIZE; i ++)
			data[i] = NAND_DATA;
		for (i = 0; i < PAGE_OOB_SIZE; i ++)
			oob_buffer[i] = NAND_DATA;

		/* verify ECC */
		if (nftl_ecc_verify256(data, PAGE_DATA_SIZE, oob_buffer) != RT_MTD_EOK)
		{
			rt_kprintf("ECC error, block: %d, page: %d!\n", page/device->pages_per_block,
				page%device->pages_per_block);
			result = RT_MTD_EECC;
		}

		if (spare != RT_NULL && spare_len > 0)
		{
			memcpy(spare, oob_buffer, spare_len);
		}
    }
    else if (spare != RT_NULL && spare_len != RT_NULL)
    {
        NAND_COMMAND = NAND_CMD_READ0;
        NAND_ADDRESS = 2048 & 0xFF;
        NAND_ADDRESS = 2048 >> 8;
        NAND_ADDRESS = page & 0xFF;
        NAND_ADDRESS = page >> 8;
        NAND_COMMAND = NAND_CMD_READ3;

        wait_for_ready();
		for (i = 0; i < spare_len; i ++)
			spare[i] = NAND_DATA;
    }

    return result;
}

static rt_err_t nand_hy27uf_writepage(struct rt_mtd_nand_device *device,
                             rt_off_t                   page,
                             const rt_uint8_t          *data,
                             rt_uint32_t                data_len,
                             const rt_uint8_t          *spare,
                             rt_uint32_t                spare_len)
{
	rt_uint32_t i;
	rt_uint8_t oob_buffer[PAGE_OOB_SIZE];

    page = page + device->block_start * device->pages_per_block;
    if (page/device->pages_per_block > device->block_end)
    {
        return -RT_MTD_EIO;
    }
	if (data == RT_NULL) return -RT_MTD_EIO; /* we didn't support write data only */
	if (spare != RT_NULL && spare_len > 0)
	{
		memcpy(oob_buffer, spare, spare_len);
	}

	/* generate ECC */
	nftl_ecc_compute256(data, PAGE_DATA_SIZE, oob_buffer);

    NAND_COMMAND = NAND_CMD_SEQIN;

    NAND_ADDRESS = 0 & 0xFF;
    NAND_ADDRESS = 0 >> 8;
    NAND_ADDRESS = page & 0xFF;
    NAND_ADDRESS = page >> 8;

	for (i = 0; i < PAGE_DATA_SIZE; i ++)
		NAND_DATA = data[i];
	for (i = 0; i < PAGE_OOB_SIZE; i ++)
		NAND_DATA = oob_buffer[i];

    NAND_COMMAND = NAND_CMD_PAGEPROG;

    wait_for_ready();

    if(read_status(NAND_CMD_SEQIN) == RT_FALSE)
    {
        nand_reset();

        return -RT_MTD_EIO;
    }

    return RT_MTD_EOK;
}

static rt_err_t nand_hy27uf_eraseblock(struct rt_mtd_nand_device *device,
                                     rt_uint32_t block)
{
    unsigned int blockPage;

    /* add the start blocks */
    block = block + device->block_start;
    blockPage = (block<<6);

    NAND_COMMAND = NAND_CMD_ERASE1;                        /* send erase command */
    NAND_ADDRESS = blockPage & 0xff;
    NAND_ADDRESS = (blockPage>>8)&0xff;
    NAND_COMMAND = NAND_CMD_ERASE2;                        /* start erase */

    wait_for_ready();
    if(read_status(NAND_CMD_ERASE1) == RT_FALSE)
    {
        nand_reset();
        return -RT_MTD_EIO;
    }

    return RT_MTD_EOK;
}

static rt_err_t nand_hy27uf_pagecopy(struct rt_mtd_nand_device *device,
                                   rt_off_t                   src_page,
                                   rt_off_t                   dst_page)
{
    rt_err_t result = RT_MTD_EOK;

    src_page = src_page + device->block_start * device->pages_per_block;
    dst_page = dst_page + device->block_start * device->pages_per_block;

	/* read data */
    NAND_COMMAND = NAND_CMD_READ0;
	/* read start from the beginning of page */
	NAND_ADDRESS = 0;
	NAND_ADDRESS = 0;

	/* send the page address */
    NAND_ADDRESS =(src_page & 0xff);
    NAND_ADDRESS =((src_page>>8) & 0xff);
    NAND_COMMAND = NAND_CMD_READ_CB;

    wait_for_ready();

	/* copy back for program */
	NAND_COMMAND = NAND_CMD_CB_PROG;
	NAND_ADDRESS = 0;
	NAND_ADDRESS = 0;

	/* send the page address */
    NAND_ADDRESS = dst_page & 0xff;
    NAND_ADDRESS = (dst_page >> 8) & 0xff;

    NAND_COMMAND = NAND_CMD_PAGEPROG;                      /* start programming */

    wait_for_ready();

    if(read_status(NAND_CMD_SEQIN) == RT_FALSE)
    {
        nand_reset();
        return RT_MTD_EIO;
    }

    return result;
}

static const struct rt_mtd_nand_driver_ops ops =
{
    nand_hy27uf_readid,
    nand_hy27uf_readpage,
    nand_hy27uf_writepage,
    nand_hy27uf_pagecopy,
    nand_hy27uf_eraseblock,
    RT_NULL,
    RT_NULL,
};
static struct rt_mtd_nand_device _partition[1];

int nand_hy27uf_hw_init(void)
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

    /* register nand0 */
    _partition[0].page_size       = PAGE_DATA_SIZE;
    _partition[0].pages_per_block = PAGES_PER_BLOCK;
    _partition[0].plane_num       = 1;
    _partition[0].oob_size        = PAGE_OOB_SIZE;
    _partition[0].oob_free        = PAGE_OOB_SIZE - ((PAGE_DATA_SIZE) * 3 / 256);
    _partition[0].block_start     = 0;
    _partition[0].block_end       = 512;

    _partition[0].block_total     = _partition[0].block_end - _partition[0].block_start;
    _partition[0].ops             = &ops;

    rt_mtd_nand_register_device("nand0", &_partition[0]);
    nand_hy27uf_readid(&_partition[0]);

    return RT_EOK;
}
