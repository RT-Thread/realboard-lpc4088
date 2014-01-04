/**********************************************************************
* $Id$      Sensor_smb380.c         2012-01-12
*//**
* @file     Sensor_smb380.c
* @brief    SMB380 acceleration sensor driver (I2C data mode)
* @version  1.0
* @date     12. January. 2012
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

#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _I2C
#include "sensor_smb380.h"
#include "lpc_i2c.h"
#include "lpc_pinsel.h"

/*********************************************************************//**
 * @brief       Read/Write data to SMB380
 * @param[in]   txdata  point to buffer of data which will be sent.
 * @param[in]   txlen     the length of transmit buffer
 * @param[in]   rxdata point to receive buffer
 * @param[in]   rxlen     the length of receive buffer
 * @return      None
 **********************************************************************/
SMB380_Status_t SMB380_ReadWrite(uint8_t* txdata, uint32_t txlen,
                                          uint8_t* rxdata, uint32_t rxlen)
{
    I2C_M_SETUP_Type i2cData;
    
    i2cData.sl_addr7bit = SMB380_ADDR;
    i2cData.tx_length = txlen;
    i2cData.tx_data = txdata;
    i2cData.rx_data = rxdata;
    i2cData.rx_length = rxlen;
    i2cData.retransmissions_max = 3;    
    
    if (I2C_MasterTransferData(I2C_1, &i2cData, I2C_TRANSFER_POLLING) == SUCCESS)
    {       
        return SMB380_PASS;
    }

    return SMB380_ERR;
}


/*********************************************************************//**
 * @brief       SMB380 init
 * @param[in]   None
 * @return      SMB380_Status_t
 **********************************************************************/

SMB380_Status_t SMB380_Init(void)
{
  unsigned char Data[2];

  //Init I2C module as master
  PINSEL_ConfigPin (2, 14, 2);
  PINSEL_ConfigPin (2, 15, 2);
  I2C_Init(I2C_1, SMB380_SPEED);
  I2C_Cmd(I2C_1,I2C_MASTER_MODE, ENABLE);

  Data[0] = 0x14;
  SMB380_ReadWrite(&Data[0], 1, NULL, 0);
  SMB380_ReadWrite(NULL, 0, &Data[1], 1);

  Data[1] &= ~(0x1F<<0);
  Data[1] |= (0x08<<0);
  //I2C_MasterWrite(SMB380_ADDR, &Data[0], 2);
  SMB380_ReadWrite(&Data[0], 2, NULL, 0);
  

  Data[0] = 0x15;
  SMB380_ReadWrite(&Data[0], 1, NULL, 0);
  SMB380_ReadWrite(NULL, 0, &Data[1], 1);

  Data[1] &= ~(3<<1);
  Data[1] |= ((1<<5) | (1<<0));
  SMB380_ReadWrite(&Data[0], 2, NULL, 0);

  return SMB380_PASS;
}
/*********************************************************************//**
 * @brief       Get Chip ID and revision
 * @param[in]   pChipId pointer to Chip ID storage
 * @param[in]   pRevision   pointer to revision storage
 * @return      SMB380_Status_t
 **********************************************************************/

SMB380_Status_t SMB380_GetID (uint8_t *pChipId, uint8_t *pRevision)
{
unsigned char buf[2] = {SMB380_CHIP_ID};
  //Write the address of Chip ID register
  SMB380_ReadWrite(buf, 1, NULL, 0);
  SMB380_ReadWrite(NULL, 0, buf, 1);
  *pChipId = buf[0];
  *pRevision = buf[1];

  return SMB380_PASS;
}

/*********************************************************************//**
 * @brief       SMB380 get data
 * @param[in]   pData address of the variable which is used to stored data.
 * @return      SMB380_Status_t
 **********************************************************************/
SMB380_Status_t SMB380_GetData (pSMB380_Data_t pData)
{
  unsigned char regaddr = SMB380_ACCX_ADDR;

  SMB380_ReadWrite(&regaddr, 1, NULL, 0);
  SMB380_ReadWrite(NULL, 0,(unsigned char *)pData, sizeof(SMB380_Data_t));

  return SMB380_PASS;
}

#endif /*_I2C*/

