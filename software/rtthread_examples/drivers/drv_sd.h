/*
 * File      : drv_sd.h
 * SPI mode SD Card Driver
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2012, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-11-06     xiaonong      first version.
 */

#ifndef __DRV_SD_H
#define __DRV_SD_H

#include <stdint.h>
#include <rtthread.h>
#include <lpc407x_8x_177x_8x.h>
#include "lpc_gpdma.h"


#define MCI_DMA_ENABLED          (1)

#define HIGH_LVL                 (1)
#define LOW_LVL                  (0)

/* SD/MMC Command list, per MMC spec. SD Memory Card Spec. Simplified version */
/* GO_IDLE_STATE(MMC) or RESET(SD) */
#define CMD0_GO_IDLE_STATE       (0)
/* SEND_OP_COND(MMC) or ACMD41(SD) */
#define CMD1_SEND_OP_COND        (1)
/* ALL_SEND_CID */
#define CMD2_ALL_SEND_CID        (2)
/* SET_RELATE_ADDR */
#define CMD3_SET_RELATIVE_ADDR   (3)
/* Set Bus-Width 1 bit or 4 bits */
#define ACMD6_SET_BUS_WIDTH      (6)
/* SELECT/DESELECT_CARD */
#define CMD7_SELECT_CARD         (7)
/**Sending interface condition cmd */
#define CMD8_SEND_IF_COND        (8)
/* SEND_CSD */
#define CMD9_SEND_CSD            (9)
/* Stop either READ or WRITE operation */
#define CMD12_STOP_TRANSMISSION  12
/* SEND_STATUS */
#define CMD13_SEND_STATUS        (13)
/* SET_BLOCK_LEN */
#define CMD16_SET_BLOCK_LEN      (16)
/* READ_SINGLE_BLOCK */
#define CMD17_READ_SINGLE_BLOCK  (17)
/* READ_MULTIPLE_BLOCK */
#define CMD18_READ_MULTIPLE_BLOCK    (18)
/* WRITE_BLOCK */
#define CMD24_WRITE_BLOCK            (24)
/* WRITE_MULTIPLE_BLOCK */
#define CMD25_WRITE_MULTIPLE_BLOCK   (25)
/* Start erase block number*/
#define CMD32_ERASE_WR_BLK_START     (32)
/* End erase block number*/
#define CMD33_ERASE_WR_BLK_END       (33)
/* Start erase*/
#define CMD38_ERASE                  (38)

/* ACMD41 for SD card */
#define ACMD41_SEND_APP_OP_COND      (41)

/* APP_CMD, the following will a ACMD */
#define CMD55_APP_CMD                (55)

#define OCR_INDEX                    (0x00FF8000)
#define RCA_ARGUMENT_POS             (16)
#define RCA_ARGUMENT_MASK            (0xFFFF)


/* Card Status (coded in 32 bits) in R1 & R1b Response */
#define CARD_STATUS_OUT_OF_RANGE    ( 1 << 31)
#define CARD_STATUS_ADDRESS_ERROR   ( 1 << 30)
#define CARD_STATUS_BLOCK_LEN_ERROR ( 1 << 29)
#define CARD_STATUS_ERASE_SEQ_ERROR ( 1 << 28)
#define CARD_STATUS_ERASE_PARAM_ERROR ( 1 << 27)
#define CARD_STATUS_WP_VIOLATION    ( 1 << 26)
#define CARD_STATUS_CARD_IS_LOCKED  ( 1 << 25)
#define CARD_STATUS_COM_CRC_ERROR   ( 1 << 23)
#define CARD_STATUS_ILLEGAL_COMMAND ( 1 << 22)
#define CARD_STATUS_CARD_ECC_FAILED ( 1 << 21)
#define CARD_STATUS_CC_ERROR        ( 1 << 20)
#define CARD_STATUS_GEN_ERROR       ( 1 << 19)
#define CARD_STATUS_CSD_OVERWRITE   ( 1 << 16)
#define CARD_STATUS_WP_ERASE_SKIP   ( 1 << 15)
#define CARD_STATUS_CARD_ECC_DISABLED   ( 1 << 14)
#define CARD_STATUS_ERASE_RESET     ( 1 << 13)
#define CARD_STATUS_READY_FOR_DATA  ( 1 << 8)
#define CARD_STATUS_ACMD_ENABLE     ( 1 << 5)
#define CARD_STATUS_ERR_MASK         (0xFDF88008)
#define CARDSTATEOF(x)              ((x>>9) & 0x0F)
#define CARD_STATE_IDLE             (0)
#define CARD_STATE_READY            (1)
#define CARD_STATE_IDENT            (2)
#define CARD_STATE_STBY             (3)
#define CARD_STATE_TRAN             (4)
#define CARD_STATE_DATA             (5)
#define CARD_STATE_RCV              (6)
#define CARD_STATE_PRG              (7)
#define CARD_STATE_DIS              (8)


/* CID  in R2 reponse (Code length is 136 bits) */
#define MCI_CID_MANUFACTURER_ID_WPOS           (24)    //pos in word 0
#define MCI_CID_MANUFACTURER_ID_WBMASK         (0xFF)

#define MCI_CID_OEMAPPLICATION_ID_WPOS         (8)        //pos in word 0
#define MCI_CID_OEMAPPLICATION_ID_WBMASK       (0xFFFF)

#define MCI_CID_PRODUCTNAME_ID_H_WPOS          (0)        //pos in word 0
#define MCI_CID_PRODUCTNAME_ID_H_WBMASK        (0xFF)

#define MCI_CID_PRODUCTNAME_ID_L_WPOS          (0)        //pos in word 1
#define MCI_CID_PRODUCTNAME_ID_L_WBMASK        (0xFFFFFFFF)

#define MCI_CID_PRODUCTREVISION_ID_WPOS        (24)    //pos in word 2
#define MCI_CID_PRODUCTREVISION_ID_WBMASK      (0xFF)

#define MCI_CID_PRODUCTSERIALNUM_ID_H_WPOS     (0)    //pos in word 2
#define MCI_CID_PRODUCTSERIALNUM_ID_H_WBMASK   (0x00FFFFFF)
#define MCI_CID_PRODUCTSERIALNUM_ID_L_WPOS     (24)    //pos in word 3
#define MCI_CID_PRODUCTSERIALNUM_ID_L_WBMASK   (0xFF)
#define MCI_CID_PRODUCTSERIALNUM_ID_WBMASK     (0xFFFFFFFF)

#define MCI_CID_RESERVED_ID_WPOS               (20)    //pos in word 3
#define MCI_CID_RESERVED_ID_WBMASK             (0x1F)

#define MCI_CID_MANUFACTURINGDATE_ID_WPOS      (8)    //in word 3
#define MCI_CID_MANUFACTURINGDATE_ID_WBMASK    (0x0FFF)

#define MCI_CID_CHECKSUM_ID_WPOS               (1)    //in word 3
#define MCI_CID_CHECKSUM_ID_WBMASK             (0x7F)

#define MCI_CID_UNUSED_ID_WPOS                 (0)    //in word 3
#define MCI_CID_UNUSED_ID_WBMASK               (0x01)

/* R6 (Published RCA response) */
#define RCA_RES_CARD_STATUS_POS                (0)
#define RCA_RES_CARD_STATUS_MASK               (0xFFFF)

#define RCA_RES_NEW_PUBLISHED_RCA_POS           (16)
#define RCA_RES_NEW_PUBLISHED_RCA_MASK          (0xFFFF)

/* R7 (Card interface condition) */
#define MCI_CMD8_VOLTAGESUPPLIED_POS           (8)
#define MCI_CMD8_VOLTAGESUPPLIED_BMASK         (0x0F)
#define MCI_CMD8_VOLATAGESUPPLIED_NOT_DEFINED  (0)
#define MCI_CMD8_VOLATAGESUPPLIED_27_36        (1)  /*2.7 - 3.6V*/

#define MCI_CMD8_CHECKPATTERN_POS              (0)
#define MCI_CMD8_CHECKPATTERN_BMASK            (0xFF)


#define MCI_SLOW_RATE                (400000)    /* 400KHz */
#define MCI_NORMAL_RATE              (20000000)  /* 20MHz */

#define SD_1_BIT                     (0)
#define SD_4_BIT                     (1)

#define DATA_TIMER_VALUE_R           (MCI_NORMAL_RATE/4)    // 250ms
#define DATA_TIMER_VALUE_W           (MCI_NORMAL_RATE)    // 1000ms

#define DATA_RW_MAX_LEN              (0xFFFF)

#define EXPECT_NO_RESP               (0)
#define EXPECT_SHORT_RESP            (1)
#define EXPECT_LONG_RESP             (2)

#define MCI_OUTPUT_MODE_PUSHPULL     (0)
#define MCI_OUTPUT_MODE_OPENDRAIN    (1)

#define NOT_ALLOW_CMD_TIMER          (0)
#define ALLOW_CMD_TIMER              (1)

#define MCI_DISABLE_CMD_TIMER        (1<<8)

/* For the SD card I tested, the minimum block length is 512 */
/* For MMC, the restriction is loose, due to the variety of SD and MMC
card support, ideally, the driver should read CSD register to find the
speed and block length for the card, and set them accordingly. */
/* In this driver example, it will support both MMC and SD cards, it
does read the information by send SEND_CSD to poll the card status,
but, it doesn't configure them accordingly. this is not intended to
support all the SD and MMC card. */

/* DATA_BLOCK_LEN table
    DATA_BLOCK_LEN           Actual Size( BLOCK_LENGTH )
    11                       2048
    10                       1024
    9                        512
    8                        256
    7                        128
    6                        64
    5                        32
    4                        16
    3                        8
    2                        4
    1                        2
*/
/* This is the size of the buffer of origin data */
#define MCI_DMA_SIZE            (1000UL)
/* This is the area original data is stored or data to be written to the SD/MMC card. */
#define MCI_DMA_SRC_ADDR        LPC_PERI_RAM_BASE
/* This is the area, after reading from the SD/MMC*/
#define MCI_DMA_DST_ADDR        (MCI_DMA_SRC_ADDR + MCI_DMA_SIZE)


/* To simplify the programming, please note that, BLOCK_LENGTH is a multiple
of FIFO_SIZE */
#define DATA_BLOCK_LEN       (9)    /* Block size field in DATA_CTRL */
#define BLOCK_LENGTH         (1 << DATA_BLOCK_LEN)
/* for SD card, 128, the size of the flash */
/* card is 512 * 128 = 64K */
#define BLOCK_NUM            0x80
#define FIFO_SIZE            16

#define BUS_WIDTH_1BIT       0
#define BUS_WIDTH_4BITS      10

/* MCI Status register bit information */
#define MCI_CMD_CRC_FAIL     (1 << 0)
#define MCI_DATA_CRC_FAIL    (1 << 1)
#define MCI_CMD_TIMEOUT      (1 << 2)
#define MCI_DATA_TIMEOUT     (1 << 3)
#define MCI_TX_UNDERRUN      (1 << 4)
#define MCI_RX_OVERRUN       (1 << 5)
#define MCI_CMD_RESP_END     (1 << 6)
#define MCI_CMD_SENT         (1 << 7)
#define MCI_DATA_END         (1 << 8)
#define MCI_START_BIT_ERR    (1 << 9)
#define MCI_DATA_BLK_END     (1 << 10)
#define MCI_CMD_ACTIVE       (1 << 11)
#define MCI_TX_ACTIVE        (1 << 12)
#define MCI_RX_ACTIVE        (1 << 13)
#define MCI_TX_HALF_EMPTY    (1 << 14)
#define MCI_RX_HALF_FULL     (1 << 15)
#define MCI_TX_FIFO_FULL     (1 << 16)
#define MCI_RX_FIFO_FULL     (1 << 17)
#define MCI_TX_FIFO_EMPTY    (1 << 18)
#define MCI_RX_FIFO_EMPTY    (1 << 19)
#define MCI_TX_DATA_AVAIL    (1 << 20)
#define MCI_RX_DATA_AVAIL    (1 << 21)


/***********************************************************************
 * MCI Data control register definitions
 **********************************************************************/
/** Data transfer enable */
#define MCI_DATACTRL_ENABLE_POS         (0)
#define MCI_DATACTRL_ENABLE_MASK        (0x01)
#define MCI_DATACTRL_ENABLE             (1 << MCI_DATACTRL_ENABLE_POS)
#define MCI_DATACTRL_DISABLE            (0 << MCI_DATACTRL_ENABLE_POS)

/** Data transfer direction */
#define MCI_DATACTRL_DIR_POS            (1)
#define MCI_DATACTRL_DIR_MASK           (0x01)
#define MCI_DATACTRL_DIR_FROM_CARD      (1 << MCI_DATACTRL_DIR_POS)
#define MCI_DATACTRL_DIR_TO_CARD        (0 << MCI_DATACTRL_DIR_POS)


/** Data transfer mode */
#define MCI_DATACTRL_XFER_MODE_POS      (2)
#define MCI_DATACTRL_XFER_MODE_MASK     (0x01)
#define MCI_DATACTRL_XFER_MODE_STREAM   (1 << MCI_DATACTRL_XFER_MODE_POS)
#define MCI_DATACTRL_XFER_MODE_BLOCK    (0 << MCI_DATACTRL_XFER_MODE_POS)

/** Enable DMA */
#define MCI_DATACTRL_DMA_ENABLE_POS     (3)
#define MCI_DATACTRL_DMA_ENABLE_MASK    (0x01)
#define MCI_DATACTRL_DMA_ENABLE         (1 << MCI_DATACTRL_DMA_ENABLE_POS)
#define MCI_DATACTRL_DMA_DISABLE        (0 << MCI_DATACTRL_DMA_ENABLE_POS)

/** Data block length macro */
#define MCI_DTATCTRL_BLOCKSIZE(n)    ((n & 0xF)<<4)


#define CMD_INT_MASK      (MCI_CMD_CRC_FAIL | MCI_CMD_TIMEOUT | MCI_CMD_RESP_END \
                         | MCI_CMD_SENT     | MCI_CMD_ACTIVE)

#define DATA_ERR_INT_MASK    (MCI_DATA_CRC_FAIL | MCI_DATA_TIMEOUT | MCI_TX_UNDERRUN \
                           | MCI_RX_OVERRUN | MCI_START_BIT_ERR)

#define ACTIVE_INT_MASK ( MCI_TX_ACTIVE | MCI_RX_ACTIVE)

#define FIFO_INT_MASK        (MCI_TX_HALF_EMPTY | MCI_RX_HALF_FULL \
                           | MCI_TX_FIFO_FULL  | MCI_RX_FIFO_FULL \
                           | MCI_TX_FIFO_EMPTY | MCI_RX_FIFO_EMPTY \
                           | MCI_DATA_BLK_END )

#define    FIFO_TX_INT_MASK (MCI_TX_HALF_EMPTY )
#define    FIFO_RX_INT_MASK (MCI_RX_HALF_FULL  )

#define DATA_END_INT_MASK    (MCI_DATA_END | MCI_DATA_BLK_END)

#define ERR_TX_INT_MASK (MCI_DATA_CRC_FAIL | MCI_DATA_TIMEOUT | MCI_TX_UNDERRUN | MCI_START_BIT_ERR)
#define ERR_RX_INT_MASK (MCI_DATA_CRC_FAIL | MCI_DATA_TIMEOUT | MCI_RX_OVERRUN  | MCI_START_BIT_ERR)

/* Error code on the command response. */
#define INVALID_RESPONSE    0xFFFFFFFF


typedef enum
{
    MCI_CARDSTATE_IDLE = 0,
    MCI_CARDSTATE_READY,
    MCI_CARDSTATE_IDENDTIFIED,
    MCI_CARDSTATE_STBY,
    MCI_CARDSTATE_TRAN,
    MCI_CARDSTATE_DATA,
    MCI_CARDSTATE_RCV,
    MCI_CARDSTATE_PRG,
    MCI_CARDSTATE_DIS,
} mci_card_state;

/**
 * SD/MMC card type
 */
typedef enum
{
    MCI_SDHC_SDXC_CARD = 3,                     /**< later Extended Capacity SD Memory Card */
    MCI_SDSC_V2_CARD = 2,                       /**< Ver 2.00 or later Standard Capacity SD Memory Card */
    MCI_MMC_CARD = 1,                           /**< MultiMedia Card */
    MCI_SDSC_V1_CARD = 0,                       /**< Ver 1.X  Standard Capacity SD Memory Card */
    MCI_CARD_UNKNOWN = -1,                      /**< unknown */
} mci_card_type;

typedef struct
{
    /** Manufacturer ID */
    rt_uint8_t MID;
    /** OEM/Application ID */
    rt_uint16_t OID;
    /** Product name 8-bits higher */
    rt_uint8_t PNM_H;
    /** Product name 32-bits Lower */
    rt_uint32_t PNM_L;
    /** Product revision */
    rt_uint8_t PRV;
    /** Product serial number */
    rt_uint32_t PSN;
    /** reserved: 4 bit */
    rt_uint8_t reserved;
    /** Manufacturing date: 12 bit */
    rt_uint16_t MDT;
    /** CRC7 checksum: 7 bit */
    rt_uint8_t CRC;
    /** not used, always: 1 bit always 1 */
    rt_uint8_t unused;
} mci_cid_t;
typedef struct
{
    /** Command ID*/
    rt_uint32_t command;
    /** Command Argument*/
    rt_uint32_t argument;
    /** Expected response: no response, short response or long response */
    rt_uint32_t expect_resp;
    /** Allow timeout */
    rt_uint32_t allow_timeout;
    /** Command Response Info */
    rt_uint32_t *cmd_resp;
} mci_cmd_t;
struct mci_device
{
    struct rt_device                parent;      /**< RT-Thread device struct */
    struct rt_device_blk_geometry   geometry;    /**< sector size, sector count */
    mci_card_type                   card_type;   /**< card type: MMC SD1.x SD2.0 SDHC SDXC */
    rt_event_t finish_event;                      /**< data send finish event*/
    rt_bool_t data_error;                          /**< data send error*/
    struct rt_mutex lock;
};
extern rt_err_t mci_hw_init(const char *device_name);

#endif // __DRV_SD_H
