/**********************************************************************
* $Id$      tsc2046.c           2012-03-13
*//**
* @file     tsc2046.c
* @brief    Contains all functions to control TSC2046 using SPI
* @version  1.0
* @date     13. March. 2012
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
#include "bsp.h"
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */

#if ((_CUR_USING_LCD == _RUNNING_LCD_QVGA_TFT) ||(_CUR_USING_LCD == _RUNNING_LCD_EA_REV_PB1)||(_CUR_USING_LCD == _RUNNING_LCD_MDM4301))
#ifdef _SSP
#include "LPC407x_8x_177x_8x.h"
#include "lpc_ssp.h"
#include "lpc_pinsel.h"
#include "lpc_gpio.h"
#include "tsc2046.h"
#include "bsp.h"
#if (TSC2046_CONVERSION_BITS == 12)
#define TSC2046_X_COORD_MAX             (0xFFF)
#define TSC2046_Y_COORD_MAX             (0xFFF)
#define TSC2046_Z1_COORD_MAX            (0xFFF)
#define TSC2046_Z2_COORD_MAX            (0xFFF)
#define TSC2046_DELTA_X_VARIANCE        (0x50)
#define TSC2046_DELTA_Y_VARIANCE        (0x50)
#define TSC2046_DELTA_Z1_VARIANCE       (0x50)
#define TSC2046_DELTA_Z2_VARIANCE       (0x50)
#else
#define TSC2046_X_COORD_MAX             (0xFF)
#define TSC2046_Y_COORD_MAX             (0xFF)
#define TSC2046_Z1_COORD_MAX            (0xFF)
#define TSC2046_Z2_COORD_MAX            (0xFF)
#define TSC2046_DELTA_X_VARIANCE        (0x05)
#define TSC2046_DELTA_Y_VARIANCE        (0x05)
#define TSC2046_DELTA_Z1_VARIANCE       (0x05)
#define TSC2046_DELTA_Z2_VARIANCE       (0x05)
#endif
#define COORD_GET_NUM                   (3)

/** SSP Configuration */
#define TSC2046_SSP_PORT                (LCD_TS_SSP_CTRL)
#define TSC2046_CS_PORT_NUM             (LCD_CS_PORT_NUM)
#define TSC2046_CS_PIN_NUM              (LCD_CS_PIN_NUM)

#define CS_ON    (GPIO_ClearValue(TSC2046_CS_PORT_NUM, (1<<TSC2046_CS_PIN_NUM))) 
#define CS_OFF   (GPIO_SetValue(TSC2046_CS_PORT_NUM, (1<<TSC2046_CS_PIN_NUM)))

/** Local variables */
static uint16_t X_Points[COORD_GET_NUM];
static uint16_t Y_Points[COORD_GET_NUM];
static uint16_t Z1_Points[COORD_GET_NUM];
static uint16_t Z2_Points[COORD_GET_NUM];
static TSC2046_Init_Type TSC_Config;


/*********************************************************************//**
 * @brief       Enable Touch Screen Controller.
 * @param[in]   None
 * @return      None
 **********************************************************************/
static void EnableTS(void)
{
    uint8_t cmd;
    SSP_DATA_SETUP_Type sspCfg;

#if (TSC2046_CONVERSION_BITS == 8)     
    cmd = START_BIT | CHANNEL_SELECT(Y_MEASURE)|CONVERT_MODE_8_BITS|DFR_MODE|PD_ENABLED;
#else
    cmd = START_BIT | CHANNEL_SELECT(Y_MEASURE)|CONVERT_MODE_12_BITS|DFR_MODE|PD_ENABLED;
#endif    
    sspCfg.tx_data = &cmd;
    sspCfg.rx_data = NULL;
    sspCfg.length  = 1; 
    CS_ON;
    SSP_ReadWrite (TSC2046_SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);
    CS_OFF;
}

/*********************************************************************//**
 * @brief       Initialize TSC2046.
 * @param[in]   pConfig  TSC Configuration
 * @return      None
 **********************************************************************/
void InitTSC2046(TSC2046_Init_Type *pConfig)
{
  SSP_CFG_Type SSP_ConfigStruct;
    
  // PIN config SSP
#ifdef CORE_M4
#if (_CURR_USING_BRD == _RB4088_BOARD)
  PINSEL_ConfigPin(4, 20, 3);
  //PINSEL_ConfigPin(4, 21, 3);
  PINSEL_ConfigPin(4, 22, 3);
  PINSEL_ConfigPin(4, 23, 3);
#else
  //PINSEL_ConfigPin(TSC2046_CS_PORT_NUM, TSC2046_CS_PIN_NUM, 0);
  PINSEL_ConfigPin(5, 2, 2);
  //PINSEL_ConfigPin(5, 3, 2);
  PINSEL_ConfigPin(5, 1, 2);
  PINSEL_ConfigPin(5, 0, 2);
#endif
#else
  PINSEL_ConfigPin(0, 15, 2);
  //PINSEL_ConfigPin(0, 16, 2);   // Use another pin for CS
  PINSEL_ConfigPin(0, 17, 2);
  PINSEL_ConfigPin(0, 18, 2);
#endif
  PINSEL_ConfigPin(TSC2046_CS_PORT_NUM, TSC2046_CS_PIN_NUM, 0);
  GPIO_SetDir(TSC2046_CS_PORT_NUM,(1<<TSC2046_CS_PIN_NUM),1);
  
  
  // initialize SSP configuration structure to default
  SSP_ConfigStructInit(&SSP_ConfigStruct);

  // set clock rate
  SSP_ConfigStruct.ClockRate = TSC2046_SSP_CLOCK;
  SSP_ConfigStruct.CPHA = SSP_CPHA_SECOND;
  SSP_ConfigStruct.CPOL = SSP_CPOL_LO;
    
  // Initialize SSP peripheral with parameter given in structure above
  SSP_Init(TSC2046_SSP_PORT, &SSP_ConfigStruct);

  // Enable SSP peripheral
  SSP_Cmd(TSC2046_SSP_PORT, ENABLE);

  // Enable Touch Screen Controller
  EnableTS();

  TSC_Config = *pConfig;

}
/*********************************************************************//**
 * @brief       Send/Receive data to/from TSC2046.
 * @param[in]   channel     It should be
 *                          X_MEASURE
 *                          Y_MEASURE
 *                          Z1_MEASURE
 *                          Z2_MEASURE
 * @param[out]  data       Received data
 * @return      None
 **********************************************************************/
static void ReadWriteTSC2046(uint8_t channel, uint16_t* data)
{
    uint8_t cmd;
    //volatile uint32_t tmp;
    SSP_DATA_SETUP_Type sspCfg;
    uint8_t rx[2];
    
    CS_ON;

    /* Send command */
#if (TSC2046_CONVERSION_BITS == 8)      
    cmd = START_BIT | CHANNEL_SELECT(channel)|CONVERT_MODE_8_BITS|DFR_MODE|REF_OFF_ADC_ON;
#else
    cmd = START_BIT | CHANNEL_SELECT(channel)|CONVERT_MODE_12_BITS|DFR_MODE|REF_OFF_ADC_ON;
#endif
    sspCfg.tx_data = &cmd;
    sspCfg.rx_data = NULL;
    sspCfg.length  = 1; 
    SSP_ReadWrite (TSC2046_SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

    //for(tmp = 0x100; tmp;tmp--);

    /* Read the response */
    sspCfg.tx_data = NULL;
    sspCfg.rx_data = rx;
    sspCfg.length  = 2; 
    SSP_ReadWrite (TSC2046_SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

#if (TSC2046_CONVERSION_BITS == 8) 
    *data = (((rx[0]&0x7F) <<8) | (rx[1]>>0)) >> 7; 
#else    
    *data = (((rx[0]&0x7F) <<8) | (rx[1]>>0)) >> 3; 
#endif 
  
    CS_OFF;

    //for(tmp = 0x10; tmp;tmp--);
}

/*********************************************************************//**
 * @brief       Evaluate the coords received from TSC.
 * @param[in]   pPoints    list of coords
 * @param[in]   PointNum   the number of entries in above list
 * @param[in]   MaxVal     the maximum value of a coord
 * @param[in]   MaxDelta   the maximum delta between coords
 * @return      -1: Invalid coords, coord in case it is valid.
 **********************************************************************/
static int16_t EvalCoord(uint16_t* pPoints, uint32_t PointNum, uint16_t MaxVal, uint16_t MaxDelta)
{
   uint32_t i = 0;
   int16_t diff = 0, coord = -1;
   uint8_t coord_valid = 0;
   
   for(i = 0; i < PointNum; i++)
   {
     // ignore values are not in range
     if(pPoints[i] >= MaxVal)
     {
       coord = -1;
       coord_valid = 0;
       continue;
     }
     
     // the first valid coord
     if(coord == -1)
     {
         coord = pPoints[i];
         coord_valid = 0;
         continue;
     }
     
     // evaluate coord
     diff = pPoints[i] - coord;
     if(diff < 0)
       diff = 0 - diff;
     if(diff < MaxDelta)
     {
       coord = (coord + pPoints[i])/2;  // get mean value
       coord_valid = 1;         // at least 2 continuous coords are valid
     }
     else
     {
       coord = pPoints[i];      // new coord
       coord_valid = 0;
     }
   }
   
   if(coord_valid)
    return coord;
   return -1;
}
/*********************************************************************//**
 * @brief       Calculate the coefficient of pressure 
 * @param[in]   x           X-Coordinate
 * @param[in]   y           Y-Coordinate
 * @param[in]   z1          Z1-Coordinate
 * @param[in]   z2          Z2-Coordinate
 * @return      coefficient.
 **********************************************************************/
static int16_t CalPressureCoef(int16_t x, int16_t y, int16_t z1, int16_t z2)
{
    int16_t z = -1;

    z = x*(z2/z1 - 1);

    return z;
}
/*********************************************************************//**
 * @brief       convert the coord received from TSC to a value on truly LCD.
 * @param[in]   Coord       received coord
 * @param[in]   MinVal    the minimum value of a coord
 * @param[in]   MaxVal     the maximum value of a coord
 * @param[in]   TrueSize   the size on LCD
 * @return      the coord after converting.
 **********************************************************************/
static int16_t ConvertCoord(int16_t Coord, int16_t MinVal, int16_t MaxVal, int16_t TrueSize)
{
    int16_t tmp;
    int16_t ret;
    uint8_t convert = 0;

    if(MinVal > MaxVal)        // Swap value
    {
        tmp = MaxVal;
        MaxVal = MinVal;
        MinVal = tmp;
        convert = 1;
    }

    ret = (Coord - MinVal)*TrueSize/(MaxVal-MinVal);
    if(convert)
      ret = TrueSize - ret;

    return ret;
}
/*********************************************************************//**
 * @brief       Get Touch coordinates.
 * @param[out]  pX     X-Coord
 * @param[out]  pY     Y-Coord
 * @return      None
 **********************************************************************/
void GetTouchCoord(int16_t *pX, int16_t* pY)
{
    uint16_t i, tmp;
    int16_t coord, x=-1, y=-1, z1=-1, z2=-1, z;

    coord = 0;
    // Get X-Coordinate
    for(i = 0; i < COORD_GET_NUM; i++)
    {
        ReadWriteTSC2046(X_MEASURE,&tmp);
        X_Points[i] = tmp;
    }
    coord = EvalCoord(X_Points,COORD_GET_NUM,TSC2046_X_COORD_MAX,TSC2046_DELTA_X_VARIANCE);
    if(coord > 0)
      x = coord;
    else
      return;

    // Get Y-Coordinate
    for(i = 0; i < COORD_GET_NUM; i++)
    {
        ReadWriteTSC2046(Y_MEASURE,&tmp);
        Y_Points[i] = tmp;
    }
    coord = EvalCoord(Y_Points,COORD_GET_NUM,TSC2046_Y_COORD_MAX,TSC2046_DELTA_Y_VARIANCE);
    if(coord > 0)
      y = coord;
    else
      return;

    // Get Z1-Coordinate
    for(i = 0; i < COORD_GET_NUM; i++)
    {
        ReadWriteTSC2046(Z1_MEASURE,&tmp);
        Z1_Points[i] = tmp;
    }
    coord = EvalCoord(Z1_Points,COORD_GET_NUM,TSC2046_Z1_COORD_MAX,TSC2046_DELTA_Z1_VARIANCE);
    if(coord > 0)
      z1 = coord;
    else
      return;

    // Get Z2-Coordinate
    for(i = 0; i < COORD_GET_NUM; i++)
    {
        ReadWriteTSC2046(Z2_MEASURE,&tmp);
        Z2_Points[i] = tmp;
    }
    coord = EvalCoord(Z2_Points,COORD_GET_NUM,TSC2046_Z2_COORD_MAX,TSC2046_DELTA_Z2_VARIANCE);
    if(coord > 0)
      z2 = coord;
    else
      return;

    z = CalPressureCoef(x,y,z1,z2);
    if((z < 0) || (z > 11000))
       return;

    // Swap, adjust to truly size of LCD
    if((x >= 0) && (y >= 0))
    {
        if(TSC_Config.swap_xy)
        {
            *pY = ConvertCoord(x,TSC_Config.ad_top,TSC_Config.ad_bottom,TSC_Config.lcd_v_size); 
            *pX = ConvertCoord(y,TSC_Config.ad_left,TSC_Config.ad_right,TSC_Config.lcd_h_size);
        }
        else
        {
            *pX = ConvertCoord(x,TSC_Config.ad_top,TSC_Config.ad_bottom,TSC_Config.lcd_v_size); 
            *pY = ConvertCoord(y,TSC_Config.ad_left,TSC_Config.ad_right,TSC_Config.lcd_h_size);
        }
    }
    EnableTS();
}
#endif
#endif  /*((_CUR_USING_LCD == _RUNNING_LCD_QVGA_TFT) ||(_CUR_USING_LCD == _RUNNING_LCD_EA_REV_PB1))*/


