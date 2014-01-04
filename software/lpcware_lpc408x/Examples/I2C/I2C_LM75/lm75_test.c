/**********************************************************************
* $Id$      eeprom_test.c               2012-01-18
*//**
* @file     eeprom_test.c  
* @brief    An example of I2C using polling mode to test the I2C driver.
*           Using EEPROM EEPROM to transfer a number of data byte.
* @version  1.0
* @date     18. Janurary. 2012
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
#include "lpc_i2c.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup I2C_LM75_polling    I2C_LM75
 * @ingroup I2C_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
/** Used I2C device definition, should be 0 or 2 */
#define I2CDEV  0

/* Definition of internal register of EEPROM EEPROM */
/* 16 bit address */
#define TEMP_SENSOR_SLVADDR         (0x48)
#define TEMP_RESOLUTION             (125) //0.125

/* Pointer register */
#define LM75_POINTER_VALUE_BIT      (0)
#define LM75_POINTER_VALUE_MSK      (0x03)

/* Data Registers */
#define LM75_CONF_REG        (0x01)     // Configuration register
#define LM75_TEMP_REG        (0x00)     // Temperature register
#define LM75_TOS_REG         (0x03)     // Over-temp Shutdown threshold Register
#define LM75_THYST_REG       (0x02)     // Hysteresis regiser

/* Configuration regiser */
#define LM75_CFG_SHUTDOWN_MODE             (0x01<<0)
#define LM75_CFG_NORMAL_MODE               (0)
#define LM75_CFG_OS_INT_MODE               (0x01<<1)    
#define LM75_CFG_OS_COMPARATOR_MODE        (0)
#define LM75_CFG_OS_POL_HI                 (0x01<<2)
#define LM75_CFG_OS_POL_LO                 (0)
#define LM75_CFG_OS_FAULT_QUEUE_1          (0)
#define LM75_CFG_OS_FAULT_QUEUE_2          (1<<3)
#define LM75_CFG_OS_FAULT_QUEUE_4          (2<<3)
#define LM75_CFG_OS_FAULT_QUEUE_6          (3<<3)

/* Temperature regiser */
#define LM75_TEMP_VAL_BIT                  (5)
#define LM75_TEMP_VAL_BIT_NUM              (11)

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
"Hello NXP Semiconductors \n\r"
"I2C demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM Cortex-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM Cortex-M3 \n\r"
#endif
"\t - Communicate via: UART0 - 115.2 kbps \n\r"
" An example of I2C using polling mode to communicate with a Digital Temperature Sensor"
" LM75\n\r"
"********************************************************************************\n\r";
uint8_t menu2[] = "Demo terminated! \n\r";


/* Transmit setup */
I2C_M_SETUP_Type txsetup;
/* Receive setup */
I2C_M_SETUP_Type rxsetup;

/************************** PRIVATE FUNCTIONS *************************/
/*********************************************************************//**
 * @brief       Calculate the temperature value.
 * @param[in]   temp    The value read from Temperature Register.
 * @return      Temperature value
 **********************************************************************/
int32_t LM75_GetTempVal(uint16_t temp)
{
    int32_t val = (temp >> LM75_TEMP_VAL_BIT);
    
    if(val & 0x400)
        val = - ((1<<LM75_TEMP_VAL_BIT_NUM) - val)*TEMP_RESOLUTION;
    else
        val = val * TEMP_RESOLUTION;
    return val;
}
/*********************************************************************//**
 * @brief       Write regiser.
 * @param[in]   reg     register        
 * @param[in]   data    pointer to buffer sent
 * @param[in]   data_num    The number of data bytes (1 or 2 bytes)
 * @return      0: if success, otherwise (-1) returned.
 **********************************************************************/
int32_t LM75_WriteReg(uint8_t reg, uint8_t* data, uint8_t data_num)
{
    uint8_t txbuf[4];
    uint8_t txsize = 0;
    
    txbuf[txsize++] = (reg & LM75_POINTER_VALUE_MSK) << LM75_POINTER_VALUE_BIT;
    txbuf[txsize++] = data[0];
    if(data_num == 2)
        txbuf[txsize++] = data[1];
    
    txsetup.sl_addr7bit = TEMP_SENSOR_SLVADDR;
    txsetup.tx_data = txbuf;
    txsetup.tx_length = txsize;
    txsetup.rx_data = NULL;
    txsetup.rx_length = 0;
    txsetup.retransmissions_max = 3;
    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &txsetup, I2C_TRANSFER_POLLING) == SUCCESS){
        return (0);
    } else {
        return (-1);
    }
}

/*********************************************************************//**
 * @brief       Read regiser.
 * @param[in]   reg     register        
 * @param[in]   val     store return value   
 * @return      0: if success, otherwise (-1) returned.
 **********************************************************************/
int32_t LM75_ReadReg(uint8_t reg, uint8_t* data, uint8_t data_num)
{
    reg = (reg & LM75_POINTER_VALUE_MSK) << LM75_POINTER_VALUE_BIT;
    
    txsetup.sl_addr7bit = TEMP_SENSOR_SLVADDR;
    txsetup.tx_data = &reg;
    txsetup.tx_length = 1;
    txsetup.rx_data = data;
    txsetup.rx_length = data_num;
    txsetup.retransmissions_max = 3;
    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &txsetup, I2C_TRANSFER_POLLING) == SUCCESS){
        return (0);
    } else {
        return (-1);
    }
}

/*********************************************************************//**
 * @brief   Print menu
 * @param[in]   menu    Menu String
 * @return  None
 **********************************************************************/
void print_menu(uint8_t* menu)
{
    _DBG_(menu);
}
/*********************************************************************//**
 * @brief       Print temperature value
 * @param[in]   temp    value
 * @return      None
 **********************************************************************/
void print_temp_val(int32_t temp)
{
    if(temp < 0) {
        _DBG("-");
        temp = -temp;
    }
    _DBD16(temp/1000);
    temp %= 1000;
    if(temp)
    {
        _DBG(".");
        _DBC('0' + (temp/100));
        temp %= 100;
        _DBC('0' + (temp/10));
        temp %= 10;
        _DBC('0' + (temp));
    }
    _DBC(248);_DBC('C');
}

/*********************************************************************//**
 * @brief       A subroutine that will be called if there's any error
 *              on I2C operation
 * @param[in]   ErrorCode Error Code Input
 * @return      None
 **********************************************************************/
void Error_Loop(uint32_t ErrorCode)
{
    /*
     * Insert your code here...
     */
    while(1);
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief   c_entry: Main program body
 * @param[in]   None
 * @return  int
 **********************************************************************/
int c_entry(void)
{
    volatile uint32_t i;
    int32_t temp = 0;

    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    // print welcome screen
    print_menu(menu1);

    /*
     * Init I2C pin connect
     */
    PINSEL_ConfigPin (0, 27, 1);
    PINSEL_ConfigPin (0, 28, 1);

    /* I2C block ------------------------------------------------------------------- */
    // Initialize I2C peripheral
    I2C_Init((en_I2C_unitId)I2CDEV, 100000);

    /* Enable I2C1 operation */
    I2C_Cmd((en_I2C_unitId)I2CDEV, I2C_MASTER_MODE, ENABLE);

    /* Configuarion ---------------------------------------------------------- */
    _DBG_("Configurate...");
    if (LM75_WriteReg(LM75_CONF_REG, LM75_CFG_NORMAL_MODE, 1) == (-1)){
        _DBG_("Error while configurating sensor");
        Error_Loop(txsetup.status);
    }

    while(1) {
        // wait for a while
        for (i = 0x1000000; i; i--);

        if (LM75_ReadReg(LM75_TEMP_REG, (uint8_t*)&temp, 2) == (-1)){
            _DBG_("Error while reading temperature value");
            Error_Loop(rxsetup.status);
        }
        //Swap two bytes
        temp = (temp&0xFF)<<8 | ((temp>>8)&0xFF);
        temp = LM75_GetTempVal(temp);
        _DBG("Current temperature: ");
        print_temp_val(temp);
        _DBG_("");
    }
}

/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
int main(void)
{
    return c_entry();
}


#ifdef  DEBUG
/*******************************************************************************
* @brief        Reports the name of the source file and the source line number
*               where the CHECK_PARAM error has occurred.
* @param[in]    file Pointer to the source file name
* @param[in]    line assert_param error line source number
* @return       None
*******************************************************************************/
void check_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while(1);
}
#endif

/*
 * @}
 */
