/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2007
 *
 *    File name   : smb380_drv.c
 *    Description : SMB380 acceleration sensor driver include file
 *
 *    History :
 *    1. Date        : 13, February 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *
 *    $Revision: 24636 $
 *
 *    @Modify: NXP MCU Application Team - NguyenCao
 *    @Date: 04. March. 2011
 **************************************************************************/

#ifndef __SMB380_DRV_H
#define __SMB380_DRV_H

#include "lpc_types.h"

#define SMB380_SPEED  400000
#define SMB380_ADDR   0x38
#define SMB380_I2C    (I2C_1)

/*#define SMD380_READ   0x71
#define SMD380_WRITE  0x70*/

#define SMB380_CHIP_ID    0x00
#define SMB380_ACCX_ADDR  0x02

typedef enum _SMB380_Status_t
{
  SMB380_PASS = 0,
  SMB380_ERR = -1,
} SMB380_Status_t;

#pragma pack(1)
typedef struct _SMB380_Data_t
{
  int16_t AccX;
  int16_t AccY;
  int16_t AccZ;
  int8_t Temp;
} SMB380_Data_t, *pSMB380_Data_t;
#pragma pack()

typedef enum _SMB380_Range_t
{
  SMB380_2G = 0, SMB380_4G, SMB380_8G
} SMB380_Range_t;

typedef enum _SMB380_Bandwidth_t
{
  SMB380_25HZ = 0, SMB380_50HZ, SMB380_100HZ, SMB380_190HZ,
  SMB380_375HZ, SMB380_750HZ, SMB380_1500HZ
} SMB380_Bandwidth_t;

/*************************************************************************
 * Function Name: SMB380_Init
 * Parameters: none
 *
 * Return: SMB380_Status_t
 *
 * Description: SMB380 init
 *
 *************************************************************************/
SMB380_Status_t SMB380_Init(void);

SMB380_Status_t SMB380_ReadWrite(uint8_t* in_data, uint32_t txlen, 
                                          uint8_t* out_data, uint32_t rxlen);
                                          

/*************************************************************************
 * Function Name: SMB380_GetID
 * Parameters: none
 *
 * Return: SMB380_Status_t
 *
 * Description: SMB380 get chip ID and revision
 *
 *************************************************************************/
SMB380_Status_t SMB380_GetID (uint8_t *pChipId, uint8_t *pRevision);

/*************************************************************************
 * Function Name: SMB380_GetData
 * Parameters: none
 *
 * Return: SMB380_Status_t
 *
 * Description:
 *
 *************************************************************************/
SMB380_Status_t SMB380_GetData (pSMB380_Data_t pData);
//SMB380_Status_t SMB380_IntClear (void);

#endif // __SMB380_DRV_H
