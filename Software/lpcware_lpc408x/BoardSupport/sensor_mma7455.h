/**********************************************************************
* $Id$      Sensor_mma7455.h            2012-03-22
*//**
* @file     Sensor_mma7455.h
* @brief    MMA7455 acceleration sensor driver (I2C data mode)
* @version  1.0
* @date     22. March. 2012
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

#ifndef __MMA7455_DRV_H
#define __MMA7455_DRV_H

#include "lpc_types.h"

/** @defgroup  Sensor_MMA7455   I2C Sensor MMA7455 
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */

#define MMA7455_SPEED  400000
#define MMA7455_ADDR   0x1D
#define MMA7455_I2C    (I2C_1)


#define MMA7455_OUTPUT_X_LSB_ADDR  0x00
#define MMA7455_OUTPUT_X_MSB_ADDR  0x01
#define MMA7455_OUTPUT_Y_LSB_ADDR  0x02
#define MMA7455_OUTPUT_Y_MSB_ADDR  0x03
#define MMA7455_OUTPUT_Z_LSB_ADDR  0x04
#define MMA7455_OUTPUT_Z_MSB_ADDR  0x05

#define MMA7455_OUTPUT_X_ADDR  0x06
#define MMA7455_OUTPUT_Y_ADDR  0x07
#define MMA7455_OUTPUT_Z_ADDR  0x08

#define MMA7455_STS_ADDR   0x09
#define MMA7455_STS_DRDY   0x01      // Data is ready
#define MMA7455_STS_DOVR   0x02      // Data is over written
#define MMA7455_STS_PERR   0x04      // Parity error

#define MMA7455_DETECT_SOURCE_ADDR   0x0A
#define MMA7455_DETECT_SOURCE_LEVEL_X   (0x01<<7)
#define MMA7455_DETECT_SOURCE_LEVEL_Y   (0x01<<6)
#define MMA7455_DETECT_SOURCE_LEVEL_Z   (0x01<<5)
#define MMA7455_DETECT_SOURCE_PULSE_X   (0x01<<4)
#define MMA7455_DETECT_SOURCE_PULSE_Y   (0x01<<3)
#define MMA7455_DETECT_SOURCE_PULSE_Z   (0x01<<2)

#define MMA7455_I2C_ADDR   0x0D
#define MMA7455_USER_INFO_ADDR 0x0E
#define MMA7455_WHOAMI_ADDR   0x0F

#define MMA7455_OFS_X_LSB_ADDR  0x10
#define MMA7455_OFS_X_MSB_ADDR  0x11
#define MMA7455_OFS_Y_LSB_ADDR  0x12
#define MMA7455_OFS_Y_MSB_ADDR  0x13
#define MMA7455_OFS_Z_LSB_ADDR  0x14
#define MMA7455_OFS_Z_MSB_ADDR  0x15

#define MMA7455_MODE_ADDR   0x16
#define MMA7455_MODE_STANDBY         (0x00)
#define MMA7455_MODE_MEASUREMENT     (0x01)
#define MMA7455_MODE_LEVEL_DETECT    (0x02)
#define MMA7455_MODE_PULSE_DETECT    (0x03)
#define MMA7455_MODE_SENS_16    (0x00<<2)    //8g
#define MMA7455_MODE_SENS_64    (0x01<<2)    //2g
#define MMA7455_MODE_SENS_32    (0x10<<2)    //4g
#define MMA7455_MODE_SELF_TEST   (0x01<<4)
#define MMA7455_MODE_DRPD       (0x01<<6)

#define MMA7455_CTR1_ADDR      0x18
#define MMA7455_CTR1_XDA_DISABLE    (0x01<<3)
#define MMA7455_CTR1_YDA_DISABLE    (0x01<<4)
#define MMA7455_CTR1_ZDA_DISABLE    (0x01<<5)
#define MMA7455_CTR1_BANDWIDTH_125  (0x01<<7) // Default 62.5Hz
#define MMA7455_CTR1_BANDWIDTH_65   (0x00<<7) // Default 62.5Hz

#define MMA7455_CTRL2_ADDR     0x19

#define MMA7455_GET_ACC_N_TIMES     8
#define MMA7455_CALIB_N_TIMES       8

typedef int8_t MMA7455_Status_t;
#define MMA7455_PASS        0
#define MMA7455_ERR         (-1)

typedef uint32_t MMA7455_Orientation_t;
#define   MMA7455_FLAT          0x00
#define   MMA7455_XUP           0x01
#define   MMA7455_XDOWN         0x02
#define   MMA7455_YUP           0x04
#define   MMA7455_YDOWN         0x08

#pragma pack(1)
typedef struct _MMA7455_Data_t
{
  int16_t AccX;
  int16_t AccY;
  int16_t AccZ;
} MMA7455_Data_t, *pMMA7455_Data_t;

#pragma pack()

/* Initialize MMA7455 */
MMA7455_Status_t MMA7455_Init(void);
/* Read/Write data on MMA7455 */
MMA7455_Status_t MMA7455_ReadWrite(uint8_t* in_data, uint32_t txlen, 
                                          uint8_t* out_data, uint32_t rxlen);
/* Get User Info from MMA7455 */
MMA7455_Status_t MMA7455_GetUserInfo (uint8_t *UserInfo);
/* Get output X,Y,Z (8 bit)*/
MMA7455_Status_t MMA7455_GetData (pMMA7455_Data_t pData);
/* Get output X,Y,Z (10 bit)*/
MMA7455_Status_t MMA7455_Get10bitData (pMMA7455_Data_t pData);
/* Set offset data*/
MMA7455_Status_t MMA7455_SetOffData (pMMA7455_Data_t pData);
/* Get Offset data */
MMA7455_Status_t MMA7455_GetOffData (pMMA7455_Data_t pData);
/* Get orientation */
MMA7455_Orientation_t MMA7455_GetOrientation(pMMA7455_Data_t pData);
/**
 * @}
 */

#endif // __MMA7455_DRV_H
