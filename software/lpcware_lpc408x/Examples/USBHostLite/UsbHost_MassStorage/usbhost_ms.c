/**********************************************************************
* $Id$      usbhost_ms.c            2011-09-05
*//**
* @file     usbhost_ms.c
* @brief        Implementation of B Mass Storage class.
* @version  1.0
* @date     05. September. 2011
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


/*
**************************************************************************************************************
*                                       INCLUDE HEADER FILES
**************************************************************************************************************
*/

#include  "usbhost_ms.h"

/** @addtogroup USBHost_Ms
 * @{
 */

/*
**************************************************************************************************************
*                                         GLOBAL VARIABLES
**************************************************************************************************************
*/
static uint32_t  MS_BlkSize;


/*********************************************************************//**
 * @brief           initializes the mass storage interface.
 * @param[in]       None.
 * @return      MS_FUNC_OK                    if Success
 *                         ERR_INVALID_BOOTSIG    if Failed
 **********************************************************************/
int32_t MS_Init (uint32_t *blkSize, uint32_t *numBlks, uint8_t *inquiryResult)
{
    uint8_t  retry;
    int32_t  rc;

    MS_GetMaxLUN();                                                    /* Get maximum logical unit number   */
    retry  = 80;
    while(retry) {
        rc = MS_TestUnitReady();                                       /* Test whether the unit is ready    */
        if (rc == MS_FUNC_OK) {
            break;
        }
        MS_GetSenseInfo();                                             /* Get sense information             */
        retry--;
    }
    if (rc != MS_FUNC_OK) {
        PRINT_Err(rc);
        return (rc);
    }
    rc = MS_ReadCapacity(numBlks, blkSize);                         /* Read capacity of the disk         */
    MS_BlkSize = *blkSize;                      // Set global
    rc = MS_Inquire (inquiryResult);
    return (rc);
}


/*********************************************************************//**
 * @brief           get  the maximum logical unit from the device.
 * @param[in]       None.
 * @return      The maximum logical uint
 **********************************************************************/
int32_t  MS_GetMaxLUN (void)
{
    int32_t  rc;


    rc = Host_CtrlRecv(USB_DEVICE_TO_HOST | USB_REQUEST_TYPE_CLASS | USB_RECIPIENT_INTERFACE,
                       MS_GET_MAX_LUN_REQ,
                       0,
                       0,
                       1,
                       TDBuffer);
    return (rc); 
}


/*********************************************************************//**
 * @brief           get sense information from the device.
 * @param[in]       None.
 * @return      MS_FUNC_OK       if Success
 *              ERR_MS_CMD_FAILED if failed
 **********************************************************************/
 int32_t  MS_GetSenseInfo (void)
{
    int32_t  rc;


    Fill_MSCommand(0, 0, 0, MS_DATA_DIR_IN, SCSI_CMD_REQUEST_SENSE, 6);
    rc = Host_ProcessTD(EDBulkOut, TD_OUT, TDBuffer, CBW_SIZE);
    if (rc == MS_FUNC_OK) {
        rc = Host_ProcessTD(EDBulkIn, TD_IN, TDBuffer, 18);
        if (rc == MS_FUNC_OK) {
            rc = Host_ProcessTD(EDBulkIn, TD_IN, TDBuffer, CSW_SIZE);
            if (rc == MS_FUNC_OK) {
                if (TDBuffer[12] != 0) {
                    rc = ERR_MS_CMD_FAILED;
                }
            }
        }
    }
    return (rc);
}

/*********************************************************************//**
 * @brief           test whether the unit is ready or not.
 * @param[in]       None.
 * @return      MS_FUNC_OK       if Success
 *              ERR_MS_CMD_FAILED if failed
 **********************************************************************/
 int32_t  MS_TestUnitReady (void)
{
    int32_t  rc;


    Fill_MSCommand(0, 0, 0, MS_DATA_DIR_NONE, SCSI_CMD_TEST_UNIT_READY, 6);
    rc = Host_ProcessTD(EDBulkOut, TD_OUT, TDBuffer, CBW_SIZE);
    if (rc == MS_FUNC_OK) {
        rc = Host_ProcessTD(EDBulkIn, TD_IN, TDBuffer, CSW_SIZE);
        if (rc == MS_FUNC_OK) {        
            if (TDBuffer[12] != 0) {
                rc = ERR_MS_CMD_FAILED;
            }
        }
    }
    return (rc);
}

/*********************************************************************//**
 * @brief           read the capacity of the mass storage device.
 * @param[in]       None.
 * @return      MS_FUNC_OK       if Success
 *              ERR_MS_CMD_FAILED if failed
 **********************************************************************/
int32_t MS_ReadCapacity (uint32_t *numBlks, uint32_t *blkSize)
{
    int32_t  rc;


    Fill_MSCommand(0, 0, 0, MS_DATA_DIR_IN, SCSI_CMD_READ_CAPACITY, 10);
    rc = Host_ProcessTD(EDBulkOut, TD_OUT, TDBuffer, CBW_SIZE);
    if (rc == MS_FUNC_OK) {
        rc = Host_ProcessTD(EDBulkIn, TD_IN, TDBuffer, 8);
        if (rc == MS_FUNC_OK) {
            if (numBlks)
                *numBlks = ReadBE32U(&TDBuffer[0]);
            if (blkSize)
                *blkSize = ReadBE32U(&TDBuffer[4]);
            rc = Host_ProcessTD(EDBulkIn, TD_IN, TDBuffer, CSW_SIZE);
            if (rc == MS_FUNC_OK) {
                if (TDBuffer[12] != 0) {
                    rc = ERR_MS_CMD_FAILED;
                }
            }
        }
    }
    return (rc);
}



/*********************************************************************//**
 * @brief           Inquiry the mass storage device.
 * @param[in]       None.
 * @return      MS_FUNC_OK       if Success
 *              ERR_MS_CMD_FAILED if failed
 **********************************************************************/
int32_t MS_Inquire (uint8_t *response)
{
    int32_t rc;
    uint32_t i;

    Fill_MSCommand(0, 0, 0, MS_DATA_DIR_IN, SCSI_CMD_INQUIRY, 6);
    rc = Host_ProcessTD(EDBulkOut, TD_OUT, TDBuffer, CBW_SIZE);
    if (rc == MS_FUNC_OK) {
        rc = Host_ProcessTD(EDBulkIn, TD_IN, TDBuffer, INQUIRY_LENGTH);
        if (rc == MS_FUNC_OK) {
            if (response) {
        for ( i = 0; i < INQUIRY_LENGTH; i++ )
            *response++ = *TDBuffer++;
#if 0
                MemCpy (response, TDBuffer, INQUIRY_LENGTH);
                StrNullTrailingSpace (response->vendorID, SCSI_INQUIRY_VENDORCHARS);
                StrNullTrailingSpace (response->productID, SCSI_INQUIRY_PRODUCTCHARS);
                StrNullTrailingSpace (response->productRev, SCSI_INQUIRY_REVCHARS);
#endif
            }
            rc = Host_ProcessTD(EDBulkIn, TD_IN, TDBuffer, CSW_SIZE);
            if (rc == MS_FUNC_OK) {
                if (TDBuffer[12] != 0) {    // bCSWStatus byte
                    rc = ERR_MS_CMD_FAILED;
                }
            }
        }
    }
    return (rc);
}
    
/*********************************************************************//**
 * @brief           receive the bulk data.
 * @param[in]       None.
 * @return      MS_FUNC_OK       if Success
 *              ERR_MS_CMD_FAILED if failed
 **********************************************************************/
 int32_t  MS_BulkRecv (          uint32_t   block_number,
                                   uint16_t   num_blocks,
                         volatile  uint8_t  *user_buffer)
{
    int32_t  rc;
    int i;
    volatile uint8_t *c = user_buffer;
    for (i=0;i<MS_BlkSize*num_blocks;i++)
        *c++ = 0;


    Fill_MSCommand(block_number, MS_BlkSize, num_blocks, MS_DATA_DIR_IN, SCSI_CMD_READ_10, 10);

    rc = Host_ProcessTD(EDBulkOut, TD_OUT, TDBuffer, CBW_SIZE);
    if (rc == MS_FUNC_OK) {
        rc = Host_ProcessTD(EDBulkIn, TD_IN, user_buffer, MS_BlkSize * num_blocks);
        if (rc == MS_FUNC_OK) {
            rc = Host_ProcessTD(EDBulkIn, TD_IN, TDBuffer, CSW_SIZE);
            if (rc == MS_FUNC_OK) {
                if (TDBuffer[12] != 0) {
                    rc = ERR_MS_CMD_FAILED;
                }
            }
        }
    }
    return (rc);
}

/*********************************************************************//**
 * @brief           send the bulk data.
 * @param[in]       None.
 * @return      MS_FUNC_OK       if Success
 *              ERR_MS_CMD_FAILED if failed
 **********************************************************************/
int32_t  MS_BulkSend (          uint32_t   block_number,
                                   uint16_t   num_blocks,
                         volatile  uint8_t  *user_buffer)
{
    int32_t  rc;


    Fill_MSCommand(block_number, MS_BlkSize, num_blocks, MS_DATA_DIR_OUT, SCSI_CMD_WRITE_10, 10);

    rc = Host_ProcessTD(EDBulkOut, TD_OUT, TDBuffer, CBW_SIZE);
    if (rc == MS_FUNC_OK) {
        rc = Host_ProcessTD(EDBulkOut, TD_OUT, user_buffer, MS_BlkSize * num_blocks);
        if (rc == MS_FUNC_OK) {
            rc = Host_ProcessTD(EDBulkIn, TD_IN, TDBuffer, CSW_SIZE);
            if (rc == MS_FUNC_OK) {
                if (TDBuffer[12] != 0) {
                    rc = ERR_MS_CMD_FAILED;
                }
            }
        }
    }
    return (rc);
}

/*********************************************************************//**
 * @brief            fill the mass storage command.
 * @param[in]       block_number     The block number.
 * @param[in]       block_size        The block size
 * @param[in]          num_blocks      The number of blocks
 * @param[in]          direction           The flow direction
 * @param[in]          scsi_cmd          The command
 * @param[in]          scsi_cmd_len    The command length
 * @return      MS_FUNC_OK       if Success
 *              ERR_MS_CMD_FAILED if failed
 **********************************************************************/
void  Fill_MSCommand (uint32_t   block_number,
                      uint32_t   block_size,
                      uint16_t   num_blocks,
                      MS_DATA_DIR  direction,
                      uint8_t   scsi_cmd,
                      uint8_t   scsi_cmd_len)
{
            uint32_t  data_len;
    static  uint32_t  tag_cnt = 0;
            uint32_t  cnt;


    for (cnt = 0; cnt < CBW_SIZE; cnt++) {
         TDBuffer[cnt] = 0;
    }
    switch(scsi_cmd) {

        case SCSI_CMD_TEST_UNIT_READY:
             data_len = 0;
             break;
        case SCSI_CMD_READ_CAPACITY:
             data_len = 8;
             break;
        case SCSI_CMD_REQUEST_SENSE:
             data_len = 18;
             break;
        case SCSI_CMD_INQUIRY:
             data_len = 36;
             break;
        default:
             data_len = block_size * num_blocks;
             break;
    }
    WriteLE32U(TDBuffer, CBW_SIGNATURE);
    WriteLE32U(&TDBuffer[4], tag_cnt);
    WriteLE32U(&TDBuffer[8], data_len);
    TDBuffer[12]     = (direction == MS_DATA_DIR_NONE) ? 0 : direction;
    TDBuffer[14]     = scsi_cmd_len;                                   /* Length of the CBW                 */
    TDBuffer[15]     = scsi_cmd;
    if ((scsi_cmd     == SCSI_CMD_REQUEST_SENSE)
     || (scsi_cmd     == SCSI_CMD_INQUIRY)) {
        TDBuffer[19] = (uint8_t)data_len;
    } else {
        WriteBE32U(&TDBuffer[17], block_number);
    }
    WriteBE16U(&TDBuffer[22], num_blocks);
}
/**
 * @}
 */

