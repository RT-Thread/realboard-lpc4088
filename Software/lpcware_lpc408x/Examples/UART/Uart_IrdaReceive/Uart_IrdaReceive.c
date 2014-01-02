/**********************************************************************
* $Id$      Uart_IrdaReceive.c          2011-06-02
*//**
* @file     Uart_IrdaReceive.c
* @brief    This example describes how to using UART in IrDA Receive mode
* @version  1.0
* @date     02. June. 2011
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

#include "lpc_uart.h"
#include "lpc_pinsel.h"
#include "lpc_gpio.h"
#include "pca9532.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup UART_IrDA_Receive UART IrDA Receiving
 * @ingroup UART_IrDA_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
#define TEST_IRDA UART_4


#define I2CDEV              LPC_I2C0//PCA9532 link to I2C0 only

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main UART program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    // UART Configuration structure variable
    UART_CFG_Type UARTConfigStruct;

    // UART FIFO configuration Struct variable
    UART_FIFO_CFG_Type UARTFIFOConfigStruct;

    uint32_t len;
    uint8_t buffer;

    pca9532_Configure_st_t pca9532Cfg;
    uint32_t cnt = 0;

    Pca9532_Init(200000);

    // Initialize UART3 pin connect
    //P2.8 as UART4_RXD with func 0x03 for recieving the signal from Infra module
    PINSEL_ConfigPin(2, 9, 3);

    /* Initialize UART Configuration parameter structure to default state:
     * Baudrate = 115200 bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */
    UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize UART3 peripheral with given to corresponding parameter
    UART_Init(TEST_IRDA, &UARTConfigStruct);

    /* Initialize FIFOConfigStruct to default state:
     *              - FIFO_DMAMode = DISABLE
     *              - FIFO_Level = UART_FIFO_TRGLEV0
     *              - FIFO_ResetRxBuf = ENABLE
     *              - FIFO_ResetTxBuf = ENABLE
     *              - FIFO_State = ENABLE
     */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 & UART3 peripheral
    UART_FIFOConfig(TEST_IRDA, &UARTFIFOConfigStruct);

    //Configure and enable IrDA mode on UART
    UART_IrDACmd(TEST_IRDA, ENABLE);

    /* Read some data from the buffer */
    while (1)
    {
        len = 0;

        while(len == 0)
        {
            len = UART_Receive(TEST_IRDA, &buffer, 1, NONE_BLOCKING);
        }

        // Control the LEDs via this channel of PCA9532 IC
        // Actually don't care these configurations
        pca9532Cfg.led_blinking_freq_0 = 30;
        pca9532Cfg.led_freq0_unit = PCA9532_CALCULATING_TIME_IN_SECOND;
        pca9532Cfg.duty_cycle_0 = 0;//percent//not used

        pca9532Cfg.led_blinking_freq_1 = 0;
        pca9532Cfg.led_freq1_unit = PCA9532_CALCULATING_TIME_IN_SECOND;
        pca9532Cfg.duty_cycle_1 = 0; //not used

        for (cnt = 0; cnt < 8; cnt++)
        {
            pca9532Cfg.led_settings[cnt] = PCA9532_LED_LEVEL_DEFAULT;

            if(((buffer >> cnt) & 0x01) == 0x01)
            {
                pca9532Cfg.led_settings[8 + cnt] = PCA9532_LED_LEVEL_ON;
            }
            else
            {
                pca9532Cfg.led_settings[8 + cnt] = PCA9532_LED_LEVEL_OFF;
            }
        }

        Pca9532_LedOutputControl(&pca9532Cfg);
    }

}

/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
int main(void)
{
    c_entry();
    return 0;
}



/**
 * @}
 */
