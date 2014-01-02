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
/** @defgroup I2C_EEPROM_polling    I2C_EEPROM
 * @ingroup I2C_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
/** Used I2C device definition, should be 0 or 2 */
#define I2CDEV  0

/* Definition of internal register of EEPROM EEPROM */
/* 16 bit address */
#define EEPROM_SLVADDR      (0xA0>>1)
#define ADDRESS_OFFSET      (0)
#define ADDRESS_SIZE        (2)
#define DATA_OFFSET         (2)
#define DATA_SIZE           (64)
#define WRITE_ADDRESS       (0x0100)
#define PAGE_SIZE           (64)

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
"Hello NXP Semiconductors \n\r"
"I2C demo \n\r"
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM Cortex-M3 \n\r"
"\t - Communicate via: UART0 - 115.2 kbps \n\r"
" An example of I2C using polling mode to test the I2C driver \n\r"
" EEPROM to transfer a number of data byte \n\r"
"********************************************************************************\n\r";
uint8_t menu2[] = "Demo terminated! \n\r";


/* Data using for transferring to EEPROM */
uint8_t EEPROM_wrdat[DATA_SIZE+ADDRESS_SIZE] ;

uint8_t EEPROM_rddat[DATA_SIZE] ;

/* Transmit setup */
I2C_M_SETUP_Type txsetup;
/* Receive setup */
I2C_M_SETUP_Type rxsetup;

/************************** PRIVATE FUNCTIONS *************************/
int32_t EEPROM_Write(void);
int32_t EEPROM_Read(void);
void print_menu(uint8_t* menu);
void Error_Loop(uint32_t ErrorCode);

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief   Write a number of data byte into EEPROM EEPROM
 * @param[in]   None
 * @return  0: if success, otherwise (-1) returned.
 **********************************************************************/
int32_t EEPROM_Write(void)
{
    txsetup.sl_addr7bit = EEPROM_SLVADDR;
    txsetup.tx_data = EEPROM_wrdat;
    txsetup.tx_length = sizeof(EEPROM_wrdat);
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
 * @brief   Read a number of data byte from EEPROM EEPROM
 * @param[in]   None
 * @return  0: if success, otherwise (-1) returned.
 **********************************************************************/
int32_t EEPROM_Read(void)
{

    rxsetup.sl_addr7bit = EEPROM_SLVADDR;
    rxsetup.tx_data = EEPROM_wrdat; // Get address to read at writing address
    rxsetup.tx_length = ADDRESS_SIZE;
    rxsetup.rx_data = EEPROM_rddat;
    rxsetup.rx_length = sizeof(EEPROM_rddat);
    rxsetup.retransmissions_max = 3;

    if (I2C_MasterTransferData((en_I2C_unitId)I2CDEV, &rxsetup, I2C_TRANSFER_POLLING) == SUCCESS){
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
    volatile int32_t tmp, i;

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
    I2C_Init((en_I2C_unitId)I2CDEV, 200000);

    /* Enable I2C1 operation */
    I2C_Cmd((en_I2C_unitId)I2CDEV, I2C_MASTER_MODE, ENABLE);

    EEPROM_wrdat[ADDRESS_OFFSET] =  (WRITE_ADDRESS >> 8)&0xFF;
    EEPROM_wrdat[ADDRESS_OFFSET+1] = WRITE_ADDRESS & 0xFF;
    for(i = 0; i<DATA_SIZE; i++)
    {
         EEPROM_wrdat[i+DATA_OFFSET] = i;
    }

    /* Transmit data ---------------------------------------------------------- */
    _DBG_("Sending...");
    if (EEPROM_Write() == (-1)){
        _DBG_("Error while sending data");
        Error_Loop(txsetup.status);
    }
    _DBG_("Complete!");

    // wait for a while
    for (tmp = 0x100000; tmp; tmp--);

    /* Receive data ---------------------------------------------------------- */
    _DBG_("Reading...");
    if (EEPROM_Read() == (-1)){
        _DBG_("Error while reading data");
        Error_Loop(rxsetup.status);
    }
    _DBG_("Complete!");

    // Verify data
    for (tmp = sizeof(EEPROM_rddat)-1; tmp>=0; tmp--){
        if (EEPROM_rddat[tmp] != EEPROM_wrdat[tmp+DATA_OFFSET]){
            _DBG_("Verify Data error!");
            break;
        }
    }

    if(tmp < 0)
    {
        _DBG_("Verify Successfully"); 
    }

     print_menu(menu2);
    /* Loop forever */
    while(1);
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
