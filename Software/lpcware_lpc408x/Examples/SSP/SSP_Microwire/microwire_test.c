/**********************************************************************
* $Id$      microwire_test.c                    2012-04-17
*//**
* @file     microwire_test.c
* @brief    This example describes how to use SPP peripheral in
*           MicroWire frame format
* @version  1.0
* @date     17. April. 2012
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
#include "lpc_ssp.h"
#include "lpc_libcfg.h"
#include "lpc_pinsel.h"
#include "debug_frmwrk.h"
#include "lpc_pinsel.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup SSP_MicroWire MicroWire
 * @ingroup SSP_Examples
 * @{
 */

/************************** PRIVATE DEFINTIONS *************************/
/* Idle char */
#define IDLE_CHAR   0xFF

/** Used SSP device as master definition */
#ifdef CORE_M4
#define USEDSSPDEV_M        2
#define SSPDEV_M            LPC_SSP2
#else
#define USEDSSPDEV_M        0
#define SSPDEV_M            LPC_SSP0
#endif

/** Used SSP device as slave definition */
#define USEDSSPDEV_S        1
#define SSPDEV_S            LPC_SSP1

/** Max buffer length */
#define BUFFER_SIZE         0x40

/* Location num */
#define SSP0_LOCALTION_NUM      0
#define SSP1_LOCALTION_NUM      0

/* These macro below is used in MiroWire Frame Format.
 * Since master in MicroWire Frame Format must send a Operation Command
 * to slave before each transmission between master and slave
 */
/* Write command */
#define MicroWire_WR_CMD    ((uint8_t)(0x00))
/* Read command */
#define MicroWire_RD_CMD    ((uint8_t)(0x01))

/* These variable below are used in Master SSP -------------------- */
/* Read data pointer */
uint8_t *pRdBuf_M;
/* Index of read data mode */
uint32_t RdIdx_M;
/* Length of data */
uint32_t DatLen_M;
/* Master Rx Buffer */
uint8_t Master_Rx_Buf[BUFFER_SIZE];


/* These variable below are used in Slave SSP -------------------- */
/* Write data pointer */
uint8_t *pWrBuf_S;
/* Index of write data mode */
uint32_t WrIdx_S;
/* Length of data */
uint32_t DatLen_S;
/* Slave Tx Buffer */
uint8_t Slave_Tx_Buf[BUFFER_SIZE];
/* Last command */
uint8_t Last_cmd;

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
"Hello NXP Semiconductors \n\r"
"SSP demo \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - Communicate via: UART0 - 115200 bps \n\r"
" This example uses two SSP peripherals in MicroWire frame format \n\r"
" \t one is set as master mode and the other is set as slave mode. \n\r"
"\t The master and slave transfer a number of data bytes together \n\r"
"\t in polling mode \n\r"
"********************************************************************************\n\r";

// SSP Configuration structure variable
SSP_CFG_Type SSP_ConfigStruct;

/************************** PRIVATE FUNCTIONS *************************/
void ssp_MW_SendCMD(LPC_SSP_TypeDef *SSPx, uint8_t cmd);
uint16_t ssp_MW_GetRSP(LPC_SSP_TypeDef *SSPx);
uint8_t ssp_MW_GetCMD(LPC_SSP_TypeDef *SSPx);
void ssp_MW_SendRSP(LPC_SSP_TypeDef *SSPx, uint16_t Rsp);
void print_menu(void);
void Buffer_Init(void);
void Buffer_Verify(void);
void Error_Loop(void);

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Send command to slave in master mode
 * @param[in]   None
 * @return      None
 **********************************************************************/
void ssp_MW_SendCMD(LPC_SSP_TypeDef *SSPx, uint8_t cmd)
{
    // wait for current SSP activity complete
    while (SSP_GetStatus(SSPx, SSP_STAT_BUSY) ==  SET);

    SSP_SendData(SSPx, (uint16_t) cmd);
}

/*********************************************************************//**
 * @brief       Get respond from slave after sending a command in master mode
 * @param[in]   None
 * @return      None
 **********************************************************************/
uint16_t ssp_MW_GetRSP(LPC_SSP_TypeDef *SSPx)
{
    while (SSP_GetStatus(SSPx, SSP_STAT_RXFIFO_NOTEMPTY) == RESET);
    return (SSP_ReceiveData(SSPx));
}

/*********************************************************************//**
 * @brief       Get command from master in slave mode
 * @param[in]   None
 * @return      None
 **********************************************************************/
uint8_t ssp_MW_GetCMD(LPC_SSP_TypeDef *SSPx)
{
    // Wait for coming CMD
    while (SSP_GetStatus(SSPx, SSP_STAT_RXFIFO_NOTEMPTY) == RESET);

    return ((uint8_t)(SSP_ReceiveData(SSPx)));
}

/*********************************************************************//**
 * @brief       Send respond to master in slave mode
 * @param[in]   None
 * @return      None
 **********************************************************************/
void ssp_MW_SendRSP(LPC_SSP_TypeDef *SSPx, uint16_t Rsp)
{
    // wait for current SSP activity complete
    while (SSP_GetStatus(SSPx, SSP_STAT_BUSY) ==  SET);

    SSP_SendData(SSPx, Rsp);
}

/*********************************************************************//**
 * @brief       Initialize buffer
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Buffer_Init(void)
{
    uint32_t i;

    for (i = 0; i < BUFFER_SIZE; i++) {
        Slave_Tx_Buf[i] = i;
        Master_Rx_Buf[i] = 0;
    }

}

/*********************************************************************//**
 * @brief       Verify buffer
 * @param[in]   none
 * @return      None
 **********************************************************************/
void Buffer_Verify(void)
{
    uint32_t i;
    uint8_t *pSTx = (uint8_t *) &Slave_Tx_Buf[0];
    uint8_t *pMRx = (uint8_t *) &Master_Rx_Buf[0];

    for ( i = 0; i < BUFFER_SIZE; i++ )
    {
        if (*pMRx++ != *pSTx++)
        {
            /* Call Error Loop */
            Error_Loop();
        }
    }
}

/*********************************************************************//**
 * @brief       Error Loop (called by Buffer_Verify() if any error)
 * @param[in]   none
 * @return      None
 **********************************************************************/
void Error_Loop(void)
{
    /* Loop forever */
    _DBG_("Verify fail!\n\r");
    while (1);
}

/*********************************************************************//**
 * @brief       Print Welcome menu
 * @param[in]   none
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG(menu1);
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main MICROWIRE program body
 * @param[in]   None
 * @return      int
 **********************************************************************/
int c_entry(void)
{
    uint32_t cnt;

    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    // print welcome screen
    print_menu();

    /*
     * Initialize SSP pin connect
     */
#ifdef CORE_M4
    //SSP2 pins
    PINSEL_ConfigPin(5, 2, 2);      // SCK J5-19
    PINSEL_ConfigPin(5, 3, 2);      // SSEL J3-24
    PINSEL_ConfigPin(5, 1, 2);      // MISO J5-20
    PINSEL_ConfigPin(5, 0, 2);      // MOSI J3-23
#else
     // SSP0 pins
    PINSEL_ConfigPin(0, 15, 2);    // SCK J5-19
    PINSEL_ConfigPin(0, 16, 2);    // SSEL J3-24
    PINSEL_ConfigPin(0, 17, 2);    // MISO J5-20
    PINSEL_ConfigPin(0, 18, 2);    // MOSI J3-23
#endif    

    // SSP1 pins  
    PINSEL_ConfigPin(0, 7, 2);    // SCK J5.17
    PINSEL_SetFilter(0, 7, 0);

    PINSEL_ConfigPin(0, 6, 2);     // SSEL J3.18     
    
    PINSEL_ConfigPin(0, 8, 2);     // MISO J3.19
    PINSEL_SetFilter(0, 8, 0);     

    PINSEL_ConfigPin(0, 9, 2);     // MOSI J5.18
    PINSEL_SetFilter(0, 9, 0);

    /* Initializing Master SSP device section ------------------------------------------- */
    // initialize SSP configuration structure to default
    SSP_ConfigStructInit(&SSP_ConfigStruct);
    // Re-configure SSP to MicroWire frame format
    SSP_ConfigStruct.FrameFormat = SSP_FRAME_MICROWIRE;
    // Initialize SSP peripheral with parameter given in structure above
    SSP_Init(SSPDEV_M, &SSP_ConfigStruct);

    // Enable SSP peripheral
    SSP_Cmd(SSPDEV_M, ENABLE);


    /* Initializing Slave SSP device section ------------------------------------------- */
    // initialize SSP configuration structure to default
    SSP_ConfigStructInit(&SSP_ConfigStruct);
    /* Re-configure mode for SSP device */
    SSP_ConfigStruct.Mode = SSP_SLAVE_MODE;
    // Re-configure SSP to MicroWire frame format
    SSP_ConfigStruct.FrameFormat = SSP_FRAME_MICROWIRE;
    // Initialize SSP peripheral with parameter given in structure above
    SSP_Init(SSPDEV_S, &SSP_ConfigStruct);

    // Enable SSP peripheral
    SSP_Cmd(SSPDEV_S, ENABLE);


    /* Initializing Buffer section ------------------------------------------------- */
    Buffer_Init();

    /* Start Transmit/Receive between Master and Slave ----------------------------- */
    pRdBuf_M = (uint8_t *)&Master_Rx_Buf[0];
    RdIdx_M = 0;
    DatLen_M = BUFFER_SIZE;
    pWrBuf_S = (uint8_t *)&Slave_Tx_Buf[0];
    WrIdx_S = 0;
    DatLen_S = BUFFER_SIZE;
    /* Force Last command to Read command as default */
    Last_cmd = MicroWire_RD_CMD;

    /* Clear all remaining data in RX FIFO */
    while (SSP_GetStatus(SSPDEV_M, SSP_STAT_RXFIFO_NOTEMPTY))
    {
        SSP_ReceiveData(SSPDEV_M);
    }
    while (SSP_GetStatus(SSPDEV_S, SSP_STAT_RXFIFO_NOTEMPTY))
    {
        SSP_ReceiveData(SSPDEV_S);
    }
    _DBG_("Press '1' to start transfer...");
    while (_DG != '1');
    for (cnt = 0; cnt < BUFFER_SIZE; cnt++)
    {
        /* The slave must initialize data in FIFO for immediately transfer from master
         * due to last received command
         */
        if (Last_cmd == MicroWire_RD_CMD)
        {
            // Then send the respond to master, this contains data
            ssp_MW_SendRSP(SSPDEV_S, (uint16_t) *(pWrBuf_S + WrIdx_S++));
        }
        else
        {
            // Then send the respond to master, this contains data
            ssp_MW_SendRSP(SSPDEV_S, 0xFF);
        }
        /* Master must send a read command to slave,
         * the slave then respond with its data in FIFO
         */
        ssp_MW_SendCMD(SSPDEV_M, MicroWire_RD_CMD);

        // Master receive respond
        *(pRdBuf_M + RdIdx_M++) = (uint8_t) ssp_MW_GetRSP(SSPDEV_M);

        // Re-assign Last command
        Last_cmd = ssp_MW_GetCMD(SSPDEV_S);
    }

    /* Verify buffer */
    Buffer_Verify();

    _DBG_("Verify success!\n\r");

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
