/**********************************************************************
* $Id$      master.c               2011-01-09
*//**
* @file     I2C\I2C_Master\master.c 
* @brief        I2C master example
* @version  1.0
* @date     09.Jan.2011
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
#include "lpc_i2c.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup I2C_master    I2C_Master
 * @ingroup I2C_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
/** Used I2C device as master definition */
#define I2CDEV_M        (0)
/** Slave address*/
#define I2CDEV_S_ADDR   (0x90>>1)
/** Transfer Mode */
#define I2CDEV_TRANSFER_POLLING        0  /*0: interrupt mode, 1: polling mode */
/** Max buffer length */
#define BUFFER_SIZE         0x10


/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
"Hello NXP Semiconductors \n\r"
"I2C demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - This example uses two I2C peripheral, one set as master and \n\r"
"\t the other set as slave \n\r"
"\t Test Master mode as alone device \n\r"
"********************************************************************************\n\r";

/** These global variables below used in interrupt mode - Slave device ----------------*/
__IO uint8_t Master_Buf[BUFFER_SIZE];
__IO uint8_t master_test[2];
__IO Bool complete;

/************************** PRIVATE FUNCTIONS *************************/
void print_menu(void);
void Buffer_Init(uint8_t type);

/*-------------------------PRIVATE FUNCTIONS-----------------------------*/
/*********************************************************************//**
 * @brief       Print Welcome menu
 * @param[in]   none
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG_(menu1);
}

/*********************************************************************//**
 * @brief       Initialize buffer
 * @param[in]   type:
 *              - 0: Initialize Master_Buf with increment value from 0
 *                  Fill all member in Slave_Buf with 0
 *              - 1: Initialize Slave_Buf with increment value from 0
 *                  Fill all member in Master_Buf with 0
 * @return      None
 **********************************************************************/
void Buffer_Init(uint8_t type)
{
    uint32_t i;

    if (type)
    {
        for (i = 0; i < BUFFER_SIZE; i++) {
            Master_Buf[i] = i;
        }
    }
    else
    {
        for (i = 0; i < BUFFER_SIZE; i++) {
            Master_Buf[i] = 0;
        }
    }
}
/*********************************************************************//**
 * @brief       Print buffer data
 * @param[in]   buff        Buffer address
 *                         size        Buffer size
 *
 * @return      None
 **********************************************************************/
void Buffer_Print(uint8_t* buff, uint32_t size)
{
    uint32_t i = 0;
    for (i = 0; i < size; i++) {
    if (i%10 == 0) {
       _DBG_("");_DBG("    ");
    }
    else
        _DBG(", ");
        _DBH(buff[i]);
    if( i ==  size - 1)
       _DBG_("");
    }
     _DBG_("");
}
#if (I2CDEV_TRANSFER_POLLING == 0)
/*********************************************************************//**
 * @brief       I2C Interrupt Handler
 * @param[in]   None
 *
 * @return      None
 **********************************************************************/
#if ((I2CDEV_M == 0))
void I2C0_IRQHandler(void)
#elif ((I2CDEV_M == 1))
void I2C1_IRQHandler(void)
#elif ((I2CDEV_M == 2))
void I2C2_IRQHandler(void)
#else
void I2C_IRQHandler(void)
#endif
{
     I2C_MasterHandler((en_I2C_unitId)I2CDEV_M);
    if (I2C_MasterTransferComplete((en_I2C_unitId)I2CDEV_M)){
          complete = TRUE;
    }   
}
#endif
/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      int
 **********************************************************************/
int c_entry(void)
{
    I2C_M_SETUP_Type transferMCfg;
    uint32_t tempp;
    uint8_t *pdat;

    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    print_menu();


    /* I2C block ------------------------------------------------------------------- */

    /*
     * Init I2C pin connect
     */
#if ((I2CDEV_M == 0))
    PINSEL_ConfigPin (5, 2, 5);
    PINSEL_ConfigPin (5, 3, 5);
#elif ((I2CDEV_M == 1))
    PINSEL_ConfigPin (0, 19, 3);
    PINSEL_ConfigPin (0, 20, 3);
    PINSEL_SetOpenDrainMode(0, 19, ENABLE);
    PINSEL_SetOpenDrainMode(0, 20, ENABLE);
    PINSEL_SetPinMode(0, 19, PINSEL_BASICMODE_PLAINOUT);
    PINSEL_SetPinMode(0, 20, PINSEL_BASICMODE_PLAINOUT);
#elif ((I2CDEV_M == 2)&& (_CURR_USING_OEM_BRD != LPC4088_OEM_BOARD))
    PINSEL_ConfigPin (0, 10, 2);
    PINSEL_ConfigPin (0, 11, 2);
    PINSEL_SetOpenDrainMode(0, 10, ENABLE);
    PINSEL_SetOpenDrainMode(0, 11, ENABLE);
    PINSEL_SetPinMode(0, 10, PINSEL_BASICMODE_PLAINOUT);
    PINSEL_SetPinMode(0, 11, PINSEL_BASICMODE_PLAINOUT);
#else
    #error "Please choose the correct peripheral."
#endif

    // Initialize Slave I2C peripheral
    I2C_Init((en_I2C_unitId)I2CDEV_M, 100000);

    /* Enable Slave I2C operation */
    I2C_Cmd((en_I2C_unitId)I2CDEV_M, I2C_MASTER_MODE, ENABLE);

    /* Transmit -------------------------------------------------------- */
    _DBG_("Press '1' to transmit");
    while (_DG != '1');


    /* Initialize buffer */
    Buffer_Init(1);
     _DBG_("Transmit Data: ");
    Buffer_Print((uint8_t*)Master_Buf, sizeof(Master_Buf));

    /* Start I2C slave device first */
    transferMCfg.sl_addr7bit = I2CDEV_S_ADDR;
    transferMCfg.tx_data = Master_Buf;
    transferMCfg.tx_length = sizeof(Master_Buf);
    transferMCfg.rx_data = NULL;
    transferMCfg.rx_length = 0;
    transferMCfg.retransmissions_max = 3;
    transferMCfg.tx_count = 0;
    transferMCfg.rx_count = 0;
    transferMCfg.retransmissions_count = 0;
     #if (I2CDEV_TRANSFER_POLLING == 0)
    complete = FALSE;
    I2C_MasterTransferData((en_I2C_unitId)I2CDEV_M, &transferMCfg, I2C_TRANSFER_INTERRUPT);
     while(!complete) ;
     #else
    I2C_MasterTransferData((en_I2C_unitId)I2CDEV_M, &transferMCfg, I2C_TRANSFER_POLLING);
    #endif


    /* Receive -------------------------------------------------------- */
    _DBG_("Press '2' to receive");
    while(_DG != '2');

    /* Initialize buffer */
    Buffer_Init(0);

    /* Start I2C slave device first */
    transferMCfg.sl_addr7bit = I2CDEV_S_ADDR;
    transferMCfg.tx_data = NULL ;
    transferMCfg.tx_length = 0;
    transferMCfg.rx_data = Master_Buf;
    transferMCfg.rx_length = sizeof(Master_Buf);
    transferMCfg.retransmissions_max = 3;
    transferMCfg.tx_count = 0;
    transferMCfg.rx_count = 0;
    transferMCfg.retransmissions_count = 0;
     #if (I2CDEV_TRANSFER_POLLING == 0)
    complete = FALSE;
    I2C_MasterTransferData((en_I2C_unitId)I2CDEV_M, &transferMCfg, I2C_TRANSFER_INTERRUPT);
     while(!complete) ;
     #else
    I2C_MasterTransferData((en_I2C_unitId)I2CDEV_M, &transferMCfg, I2C_TRANSFER_POLLING);
    #endif

    pdat = (uint8_t*)Master_Buf;
    _DBG_("Receive Data: ");
    Buffer_Print((uint8_t*)Master_Buf, sizeof(Master_Buf));

    // Verify
    for (tempp = 0; tempp < sizeof(Master_Buf); tempp++){
        if (*pdat != (tempp)){
                 _DBG_("Verify error ");
             break;
        }
    pdat++;
    }
    if (tempp == sizeof(Master_Buf)){
        _DBG_("Verify successfully");
    }

    /* Transmit and receive -------------------------------------------------------- */
    _DBG_("Press '3' to Transmit, then repeat start and receive...");
    while (_DG != '3');

    /* Initialize buffer */
    Buffer_Init(0);
    master_test[0] = 0xAA;
    master_test[1] = 0x55;

    /* Start I2C slave device first */
    transferMCfg.sl_addr7bit = I2CDEV_S_ADDR;
    transferMCfg.tx_data = master_test ;
    transferMCfg.tx_length = sizeof(master_test);
    transferMCfg.rx_data = Master_Buf;
    transferMCfg.rx_length = sizeof(Master_Buf);
    transferMCfg.tx_count = 0;
    transferMCfg.rx_count = 0;
    transferMCfg.retransmissions_count = 0;
    transferMCfg.retransmissions_max = 3;
     #if (I2CDEV_TRANSFER_POLLING == 0)
    complete = FALSE;
    I2C_MasterTransferData((en_I2C_unitId)I2CDEV_M, &transferMCfg, I2C_TRANSFER_INTERRUPT);
     while(!complete) ;
     #else
    I2C_MasterTransferData((en_I2C_unitId)I2CDEV_M, &transferMCfg, I2C_TRANSFER_POLLING);
    #endif

    pdat = (uint8_t*)Master_Buf;
    _DBG_("Transmit Data: ");
    Buffer_Print((uint8_t*)master_test, sizeof(master_test));
    _DBG_("Receive Data: ");
    Buffer_Print((uint8_t*)Master_Buf, sizeof(Master_Buf));
    // Verify
    for (tempp = 0; tempp < sizeof(Master_Buf); tempp++){
        if (*pdat++ != tempp){
            _DBG_("Verify error: ");
            break;
        }
    }
    if (tempp == sizeof(Master_Buf)){
        _DBG_("Verify successfully");
    }
    I2C_DeInit((en_I2C_unitId)I2CDEV_M);
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

/*
 * @}
 */
