/**********************************************************************
* $Id$      Sensor_mma7455.c            2012-03-22
*//**
* @file     Sensor_mma7455.c
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

#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _I2C
#include "sensor_mma7455.h"
#include "lpc_i2c.h"
#include "lpc_pinsel.h"

#ifndef abs
#define abs(x)  (x > 0) ? x:(-x)
#endif

/*********************************************************************//**
 * @brief       Read/Write data to MMA7455
 * @param[in]   txdata  point to buffer of data which will be sent.
 * @param[in]   txlen     the length of transmit buffer
 * @param[in]   rxdata point to receive buffer
 * @param[in]   rxlen     the length of receive buffer
 * @return      None
 **********************************************************************/
MMA7455_Status_t MMA7455_ReadWrite(uint8_t* txdata, uint32_t txlen,
                                          uint8_t* rxdata, uint32_t rxlen)
{
    I2C_M_SETUP_Type i2cData;
    
    i2cData.sl_addr7bit = MMA7455_ADDR;
    i2cData.tx_length = txlen;
    i2cData.tx_data = txdata;
    i2cData.rx_data = rxdata;
    i2cData.rx_length = rxlen;
    i2cData.retransmissions_max = 3;    
    
    if (I2C_MasterTransferData(I2C_0, &i2cData, I2C_TRANSFER_POLLING) == SUCCESS)
    {       
        return MMA7455_PASS;
    }

    return MMA7455_ERR;
}

/*********************************************************************//**
 * @brief       MMA7455 init
 * @param[in]   None
 * @return      MMA7455_Status_t
 **********************************************************************/
MMA7455_Status_t MMA7455_Init(void)
{
  unsigned char Data[2];
  MMA7455_Status_t ret;
  MMA7455_Data_t acc_data, offset, old_offset;
  uint8_t g_val;
  uint32_t cnt;

  //Init I2C module as master
  PINSEL_ConfigPin (0, 27, 1);
  PINSEL_ConfigPin (0, 28, 1);
  I2C_Init(I2C_0, MMA7455_SPEED);
  I2C_Cmd(I2C_0,I2C_MASTER_MODE, ENABLE);

  // Set Mode
  Data[0] = MMA7455_MODE_ADDR;
  Data[1] = MMA7455_MODE_SENS_64|MMA7455_MODE_MEASUREMENT;

  ret = MMA7455_ReadWrite(&Data[0], 2, NULL, 0);
  if(ret != MMA7455_PASS)
    return ret;
  
  Data[0] = MMA7455_CTR1_ADDR;
  Data[1] = MMA7455_CTR1_BANDWIDTH_125;
  ret = MMA7455_ReadWrite(&Data[0], 2, NULL, 0);
  if(ret != MMA7455_PASS)
    return ret;

  // Calibrate
  g_val = 63;
      
  for(cnt = 0; cnt < MMA7455_CALIB_N_TIMES; cnt++)
  {
      // Get current values of X, Y, Z
      ret = MMA7455_Get10bitData(&acc_data);
      if(ret != MMA7455_PASS)
        return ret;

       // [0,0,+1g] position
      if(acc_data.AccX == 0 &&
         acc_data.AccY == 0 &&
         acc_data.AccZ == g_val)
         break;

      // Get current offset
      ret = MMA7455_GetOffData(&old_offset);
      if(ret != MMA7455_PASS)
        return ret;

      
      offset.AccX =  acc_data.AccX * (-2);   // shift X. Ofs X has ½ LSB.  
      offset.AccY =  acc_data.AccY * (-2);   // Shift Y. Ofs Y has ½ LSB.
      if(acc_data.AccZ > 0)            
        offset.AccZ = acc_data.AccZ - g_val; // Shift Z
      else
        offset.AccZ = acc_data.AccZ + g_val;
      offset.AccZ *= -2;                    // . Ofs Z has ½ LSB.

      // compense the current offset
      offset.AccX += old_offset.AccX;
      offset.AccY += old_offset.AccY;
      offset.AccZ += old_offset.AccZ;

      ret = MMA7455_SetOffData(&offset);
      if(ret != MMA7455_PASS)
        return ret;
  }
  return ret;
}

/*********************************************************************//**
 * @brief       Get User Info
 * @param[in]   UserInfo address of the variable which is used to stored User Info.
 * @return      MMA7455_Status_t
 **********************************************************************/
 MMA7455_Status_t MMA7455_GetUserInfo (uint8_t *UserInfo)
{
  unsigned char buf[1] = {MMA7455_USER_INFO_ADDR};
  return MMA7455_ReadWrite(buf, 1, UserInfo, 1);
}


/*********************************************************************//**
 * @brief       MMA7455 get 8 bit output data
 * @param[in]   pData address of the variable which is used to stored data.
 * @return      MMA7455_Status_t
 **********************************************************************/
 MMA7455_Status_t MMA7455_GetData (pMMA7455_Data_t pData)
{
  uint8_t buf[2];
  uint32_t cnt = 0;

  pData->AccX = 0;
  pData->AccY = 0;
  pData->AccZ = 0;
  for(cnt = 0; cnt < MMA7455_GET_ACC_N_TIMES; cnt++)
  {
      // Get Status
      while(1)
      {
        buf[0] = MMA7455_STS_ADDR;
        MMA7455_ReadWrite(buf, 1,&buf[1], 1);
        if((buf[0] & MMA7455_STS_DRDY) == MMA7455_STS_DRDY)
            break;
      }
      buf[0] = MMA7455_OUTPUT_X_ADDR;
      MMA7455_ReadWrite(buf, 1,&buf[1], 1);
      if(buf[1] & 0x80)
       pData->AccX += buf[1] | 0xFF00;     // move sign bit to 15th bit
      else
        pData->AccX += buf[1];
    
       buf[0] = MMA7455_OUTPUT_Y_ADDR;
      MMA7455_ReadWrite(buf, 1,&buf[1], 1);
      if(buf[1] & 0x80)
       pData->AccY += buf[1] | 0xFF00;    // move sign bit to 15th bit
      else
       pData->AccY += buf[1];
    
      buf[0] = MMA7455_OUTPUT_Z_ADDR;
      MMA7455_ReadWrite(buf, 1,&buf[1], 1);
      if(buf[1] & 0x80)
       pData->AccZ += buf[1] | 0xFF00;       // move sign bit to 15th bits
      else
        pData->AccZ += buf[1];
  }
  pData->AccX /= MMA7455_GET_ACC_N_TIMES;
  pData->AccY /= MMA7455_GET_ACC_N_TIMES;
  pData->AccZ /= MMA7455_GET_ACC_N_TIMES;
  return MMA7455_PASS;
}
/*********************************************************************//**
 * @brief       MMA7455 get 10 bit output data
 * @param[in]   pData address of the variable which is used to stored data.
 * @return      MMA7455_Status_t
 **********************************************************************/
 MMA7455_Status_t MMA7455_Get10bitData (pMMA7455_Data_t pData)
{
  uint8_t buf[3];
  uint32_t cnt = 0;
  int16_t tmp;

  pData->AccX = 0;
  pData->AccY = 0;
  pData->AccZ = 0;
  for(cnt = 0; cnt < MMA7455_GET_ACC_N_TIMES; cnt++)
  {
      // Get Status
      while(1)
      {
        buf[0] = MMA7455_STS_ADDR;
        MMA7455_ReadWrite(buf, 1,&buf[1], 1);
        if((buf[0] & MMA7455_STS_DRDY) == MMA7455_STS_DRDY)
            break;
      }
      buf[0] = MMA7455_OUTPUT_X_LSB_ADDR;
      MMA7455_ReadWrite(buf, 1,&buf[1], 2);
      tmp = buf[1] | (buf[2]<<8);
      if(buf[2] & 0x02)
        tmp |=  0xFC00;       // move sign bit to 15th bit
      pData->AccX += tmp;
    
       buf[0] = MMA7455_OUTPUT_Y_LSB_ADDR;
      MMA7455_ReadWrite(buf, 1,&buf[1], 2);
      tmp = buf[1] | (buf[2]<<8);
      if(buf[2] & 0x02)
        tmp |=  0xFC00;    // move sign bit to 15th bit
      pData->AccY += tmp;
    
      buf[0] = MMA7455_OUTPUT_Z_LSB_ADDR;
      MMA7455_ReadWrite(buf, 1,&buf[1], 2);
      tmp = buf[1] | (buf[2]<<8);
      if(buf[2] & 0x02)
        tmp |=  0xFC00;    // move sign bit to 15th bit
      pData->AccZ += tmp;
  }
  pData->AccX /= MMA7455_GET_ACC_N_TIMES;
  pData->AccY /= MMA7455_GET_ACC_N_TIMES;
  pData->AccZ /= MMA7455_GET_ACC_N_TIMES;
  return MMA7455_PASS;
}
/*********************************************************************//**
 * @brief       MMA7455 get offset data
 * @param[in]   pData address of the variable which is used to stored data.
 * @return      MMA7455_Status_t
 **********************************************************************/
 MMA7455_Status_t MMA7455_GetOffData (pMMA7455_Data_t pData)
{
  uint8_t buf[2];
  int16_t tmp;

  buf[0] = MMA7455_OFS_X_LSB_ADDR;
  MMA7455_ReadWrite(buf, 1,&buf[1], 1);
  tmp = buf[1];

  buf[0] = MMA7455_OFS_X_MSB_ADDR;
  MMA7455_ReadWrite(buf, 1,&buf[1], 1);
  tmp |=  buf[1] << 8;
  if(buf[1] & 0x04)
    pData->AccX = tmp | 0xF800;      // move sign bit to 15th bit
  else
    pData->AccX = tmp;

  buf[0] = MMA7455_OFS_Y_LSB_ADDR;
  MMA7455_ReadWrite(buf, 1,&buf[1], 1);
  tmp = buf[1];

   buf[0] = MMA7455_OFS_Y_MSB_ADDR;
  MMA7455_ReadWrite(buf, 1,&buf[1], 1);
  tmp |=  buf[1] << 8;
  if(buf[1] & 0x04)
    pData->AccY = tmp | 0xF800;      // move sign bit to 15th bit
  else
    pData->AccY = tmp;

  buf[0] = MMA7455_OFS_Z_LSB_ADDR;
  MMA7455_ReadWrite(buf, 1,&buf[1], 1);
  tmp = buf[1];

  buf[0] = MMA7455_OFS_Z_MSB_ADDR;
  MMA7455_ReadWrite(buf, 1,&buf[1], 1);
  tmp |=  buf[1] << 8;
  if(buf[1] & 0x04)
    pData->AccZ = tmp | 0xF800;     // move sign bit to 15th bit
  else
    pData->AccZ = tmp;

  return MMA7455_PASS;
}
/*********************************************************************//**
 * @brief       MMA7455 set offset data
 * @param[in]   pData address of the variable which stores offset data.
 * @return      MMA7455_Status_t
 **********************************************************************/
MMA7455_Status_t MMA7455_SetOffData (pMMA7455_Data_t pData)
{
  uint8_t buf[2];

  buf[0] = MMA7455_OFS_X_LSB_ADDR;
  buf[1] = pData->AccX & 0xFF;
  MMA7455_ReadWrite(buf, 2, NULL, 0);

  buf[0] = MMA7455_OFS_X_MSB_ADDR;
  buf[1] = (pData->AccX >> 8) & 0x07;
  MMA7455_ReadWrite(buf, 2, NULL, 0);

  buf[0] = MMA7455_OFS_Y_LSB_ADDR;
  buf[1] = pData->AccY & 0xFF ;
  MMA7455_ReadWrite(buf, 2, NULL, 0);

  buf[0] = MMA7455_OFS_Y_MSB_ADDR;
  buf[1] = (pData->AccY >> 8) & 0x07 ;
  MMA7455_ReadWrite(buf, 2, NULL, 0);

  buf[0] = MMA7455_OFS_Z_LSB_ADDR;
  buf[1] = pData->AccZ & 0xFF;
  MMA7455_ReadWrite(buf, 2, NULL, 0);

  buf[0] = MMA7455_OFS_Z_MSB_ADDR;
  buf[1] = (pData->AccZ >> 8) & 0x07;
  MMA7455_ReadWrite(buf, 2, NULL, 0);

  return MMA7455_PASS;
}
/*********************************************************************//**
 * @brief       Get movement direction of the sensor
 * @param[in]   pData address of the variable which is used to store offset data.
 * @return      MMA7455_Status_t
 **********************************************************************/
MMA7455_Orientation_t MMA7455_GetOrientation(pMMA7455_Data_t pData)
{

  int8_t sin30, neg_sin30;
  int8_t x, y;
  MMA7455_Orientation_t ori;

  sin30 = 30;
  neg_sin30 = -30;

  x =  pData->AccX;
  y = pData->AccY;

   if ((x>sin30)&&((uint8_t)abs(y)<sin30)) ori = MMA7455_XUP;   
   else if ((x<neg_sin30)&&((uint8_t)abs(y)<sin30)) ori = MMA7455_XDOWN;   
   else if (((uint8_t)abs(x)<sin30)&&(y>sin30)) ori = MMA7455_YUP;   
   else if (((uint8_t)abs(x)<sin30)&&(y<neg_sin30)) ori = MMA7455_YDOWN;   
   else ori = MMA7455_FLAT; 
   
   return ori; 
}
#endif /*_I2C*/

