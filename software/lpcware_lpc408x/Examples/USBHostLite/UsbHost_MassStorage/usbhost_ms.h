/**********************************************************************
* $Id$		Usbhost_ms.h			2011-09-05
*//**
* @file		Usbhost_ms.h
* @brief		Implementation of USB Mass Storage Class.
* @version	1.0
* @date		05. September. 2011
* @author	NXP MCU SW Application Team
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


#ifndef  USBHOST_MS_H
#define  USBHOST_MS_H

/*
**************************************************************************************************************
*                                       INCLUDE HEADER FILES
**************************************************************************************************************
*/

#include  "usbhost_inc.h"

/** @addtogroup USBHost_Ms
 * @{
 */

/* Public macros ------------------------------------------------------------- */
/** @defgroup MS_Macros MS Macros
 * @{
 */

/*********************************************************************//**
 *  Mass Storage Specific Definitions
 **********************************************************************/
#define    MS_GET_MAX_LUN_REQ            0xFE
#define    MASS_STORAGE_CLASS            0x08
#define    MASS_STORAGE_SUBCLASS_SCSI    0x06
#define    MASS_STORAGE_PROTOCOL_BO      0x50
#define    INQUIRY_LENGTH                36

/*********************************************************************//**
 *  SCSI Specific Definitions
 **********************************************************************/
#define  CBW_SIGNATURE               0x43425355
#define  CSW_SIGNATURE               0x53425355
#define  CBW_SIZE                      31
#define  CSW_SIZE                      13
#define  CSW_CMD_PASSED              0x00
#define  SCSI_CMD_REQUEST_SENSE      0x03
#define  SCSI_CMD_TEST_UNIT_READY    0x00
#define  SCSI_CMD_INQUIRY            0x12
#define  SCSI_CMD_READ_10            0x28
#define  SCSI_CMD_READ_CAPACITY      0x25
#define  SCSI_CMD_WRITE_10           0x2A

/*********************************************************************//**
 *   MASS STORAGE SPECIFIC ERROR CODES
 **********************************************************************/
#define  MS_FUNC_OK              0
#define  ERR_NO_MS_INTERFACE     -12
#define  ERR_MS_CMD_FAILED       -10

/**
 * @}
 */



/* Public Types --------------------------------------------------------------- */
/** @defgroup  MS_Public_Types MS Public Types
 * @{
 */

/**
 * @brief MS Data Direction Enumeration*/

typedef enum  ms_data_dir {

    MS_DATA_DIR_IN     = 0x80,
    MS_DATA_DIR_OUT    = 0x00,
    MS_DATA_DIR_NONE   = 0x01

} MS_DATA_DIR;

/**
 * @}
 */

/* Public Functions ----------------------------------------------------------- */
/** @defgroup MS_Public_Functions MS Public Functions
 * @{
 */

int32_t  MS_BulkRecv          (          uint32_t    block_number,
                                            uint16_t    num_blocks,
                                  volatile  uint8_t   *user_buffer);

int32_t  MS_BulkSend          (          uint32_t    block_number,
                                            uint16_t    num_blocks,
                                  volatile  uint8_t   *user_buffer);
int32_t  MS_ParseConfiguration(void);
int32_t  MS_TestUnitReady     (void);
int32_t  MS_ReadCapacity (uint32_t *numBlks, uint32_t *blkSize);
int32_t  MS_GetMaxLUN         (void);
int32_t  MS_GetSenseInfo      (void);
int32_t  MS_Init (uint32_t *blkSize, uint32_t *numBlks, uint8_t *inquiryResult);
int32_t  MS_Inquire (uint8_t *response);

void        Fill_MSCommand       (          uint32_t    block_number,
                                            uint32_t    block_size,
                                            uint16_t    num_blocks,
                                            MS_DATA_DIR direction,
                                            uint8_t     scsi_cmd,
                                            uint8_t     scsi_cmd_len);
/**
 * @}
 */
 /**
 * @}
 */

#endif
