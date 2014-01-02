/**********************************************************************
* $Id$        Can_SimpleTxRx.c        2012-09-17
*//**
* @file       Can_SimpleTxRx.c
* @brief      This example used to test CAN feature combined with PCAN.
* @version    1.0
* @date       17. September. 2012
* @author     NXP MCU SW Application Team
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
#include "lpc_types.h"
#include "lpc_can.h"
#include "lpc_pinsel.h"
#include "lpc_exti.h"
#include "lpc_clkpwr.h"
#include "debug_frmwrk.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup Can_SimpleTxRx    CAN SimpleTxRx
 * @ingroup CAN_Examples
 * @{
 */

/************************** PRIVATE DEFINTIONS*************************/
#define USED_CAN_NO         CAN_1
#define RX_MSG_ID            0x100
#define RX_EXT_MSG_ID        0x00100000

#define TX_MSG_ID            (0x200)
#define TX_EXT_MSG_ID        0x00200000


/************************** PRIVATE VARIABLES *************************/
__IO CAN_MSG_Type TxMsg;
__IO CAN_MSG_Type RxMsg;
__IO Bool RxFlg =  FALSE;
__IO Bool TxFlg =  FALSE;



/* Extern variables from can.c */
extern volatile uint32_t BOffCnt;
extern volatile uint32_t EWarnCnt;
extern volatile uint32_t StuffErrCnt;
extern volatile uint32_t FormErrCnt;
extern volatile uint32_t AckErrCnt;
extern volatile uint32_t Bit1ErrCnt;
extern volatile uint32_t Bit0ErrCnt;
extern volatile uint32_t CRCErrCnt;
extern volatile uint32_t ND1ErrCnt;

uint8_t menu[]=
    "********************************************************************************\n\r"
    "Hello NXP Semiconductors \n\r"
    "CAN demo \n\r"
#ifdef CORE_M4
    "\t - MCU: LPC407x_8x \n\r"
    "\t - Core: ARM CORTEX-M4 \n\r"
#else
    "\t - MCU: LPC177x_8x \n\r"
    "\t - Core: ARM CORTEX-M3 \n\r"
#endif
    "\t - UART Communicate via: 115200 bps \n\r"
    "use CAN to transmit and receive Message to/from CAN Analyzer\n\r"
    "********************************************************************************\n\r";


/************************** PRIVATE FUNCTION *************************/
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
 * @brief       Error Loop
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Error_Loop()
{
    /*
     * Insert your code here...
     */
    while(1);
}

/*********************************************************************//**
 * @brief       Print Message via COM
 * param[in]    msg: point to CAN_MSG_Type object that will be printed
 * @return      none
 **********************************************************************/
void PrintMessage(CAN_MSG_Type* CAN_Msg)
{
    uint32_t data;

    _DBG("Message ID:     ");

    _DBH32(CAN_Msg->id);_DBG_("");

    _DBG("Message length: ");

    _DBH32(CAN_Msg->len);_DBG_(" BYTES");

    _DBG("Message type:   ");

    if(CAN_Msg->type==DATA_FRAME)
    {
        _DBG_("DATA FRAME ");
    }
    else
        _DBG_("REMOTE FRAME ");

    _DBG("Message format: ");

    if(CAN_Msg->format==STD_ID_FORMAT)
    {
        _DBG_("STANDARD ID FRAME FORMAT");
    }
    else
        _DBG_("EXTENDED ID FRAME FORMAT");

    _DBG("Message dataA:  ");

    data = (CAN_Msg->dataA[0])|(CAN_Msg->dataA[1]<<8)|(CAN_Msg->dataA[2]<<16)|(CAN_Msg->dataA[3]<<24);

    _DBH32(data);_DBG_("");

    data = (CAN_Msg->dataB[0])|(CAN_Msg->dataB[1]<<8)|(CAN_Msg->dataB[2]<<16)|(CAN_Msg->dataB[3]<<24);

    _DBG("Message dataB:  ");

    _DBH32(data);_DBG_("");

    _DBG_("");
}


/*********************************************************************//**
 * @brief       Setup Acceptance Filter Table
 * @param[in]   none
 * @return      none
 * Note:        not use Group Standard Frame, just use for Explicit
 *              Standard and Extended Frame
 **********************************************************************/
CAN_ERROR CAN_SetupAFTable(void) {
    CAN_ERROR result;
    /* Set up Explicit Standard Frame Format Identifier Section
     * In this simple test, it has 1 entry with ID of 0x200
     */
    result = CAN_LoadExplicitEntry(USED_CAN_NO, RX_MSG_ID, STD_ID_FORMAT);
    if(result != CAN_OK)
        return result;
    
    /* Set up Explicit Extended Frame Format Identifier Section
     * In this simple test, it has 16 entries ID
     */
    result = CAN_LoadExplicitEntry(USED_CAN_NO, RX_EXT_MSG_ID, EXT_ID_FORMAT);
    
    return result;
}


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief        Event Router IRQ Handler
 * @param[in]    None
 * @return       None
 **********************************************************************/
void CAN_IRQHandler(void)
{
    uint8_t IntStatus;
    /* get interrupt status
     * Note that: Interrupt register CANICR will be reset after read.
     * So function "CAN_IntGetStatus" should be call only one time
     */
    IntStatus = CAN_IntGetStatus(USED_CAN_NO);

    //check receive interrupt
    if((IntStatus >> 0) & 0x01)
    {
        CAN_ReceiveMsg(USED_CAN_NO, (CAN_MSG_Type *)&RxMsg);
        RxFlg = TRUE;
    }
}
/*********************************************************************//**
 * @brief       External interrupt 0 handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void EINT0_IRQHandler(void)
{
      //clear the EINT0 flag
      EXTI_ClearEXTIFlag(EXTI_EINT0);
      TxFlg = TRUE;
}


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief        c_entry: main function
 * @param[in]    none
 * @return       int
 **********************************************************************/
int c_entry(void) { /* Main Program */
    EXTI_InitTypeDef EXTICfg;
    uint8_t data_cnt = 0;
    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();
    print_menu(menu);

    /* Initialize EXT pin and registers
     * P2.10 as /EINT0
     */
    PINSEL_ConfigPin(2,10,1);

    EXTI_Init();

    EXTICfg.EXTI_Line = EXTI_EINT0;
    /* edge sensitive */
    EXTICfg.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    EXTICfg.EXTI_polarity = EXTI_POLARITY_LOW_ACTIVE_OR_FALLING_EDGE;
    EXTI_ClearEXTIFlag(EXTI_EINT0);
    EXTI_Config(&EXTICfg);

    NVIC_EnableIRQ(EINT0_IRQn);
    

    /* Pin configuration
     * CAN1: select P0.0 as RD1. P0.1 as TD1
     * CAN2: select P0.4 as RD2, P0.5 as RD2
     */

#if (USED_CAN_NO == CAN_1)
    PINSEL_ConfigPin (0, 0, 1);

    PINSEL_ConfigPin (0, 1, 1);
#else
    PINSEL_ConfigPin (0, 4, 2);

    PINSEL_ConfigPin (0, 5, 2);
#endif

    _DBG_("CAN init\n\r");
    
    //Initialize CAN
    CAN_Init(USED_CAN_NO, 500000);

    //Enable Interrupt
    CAN_IRQCmd(USED_CAN_NO, CANINT_RIE, ENABLE);

    // Set AF Mode
    CAN_SetAFMode(CAN_NORMAL);

    // Set up AF Look-up Table
    if(CAN_SetupAFTable() != CAN_OK)
    {
        _DBG_("Setup AF Look-up Table ERROR!!!");
        Error_Loop();
    }
    
    //Enable CAN Interrupt
    NVIC_EnableIRQ(CAN_IRQn);

    TxMsg.id = TX_MSG_ID;
    TxMsg.len = 8;
    TxMsg.format = STD_ID_FORMAT;
    TxMsg.type = DATA_FRAME;
    TxMsg.dataA[0] = TxMsg.dataA[1] = TxMsg.dataA[2] = TxMsg.dataA[3] = data_cnt++;
    TxMsg.dataB[0] = TxMsg.dataB[1] = TxMsg.dataB[2] = TxMsg.dataB[3] = data_cnt++;
    _DBG_("Send a message...");
    PrintMessage((CAN_MSG_Type*)&TxMsg);
    CAN_SendMsg(USED_CAN_NO,(CAN_MSG_Type*)&TxMsg);

    while (1)                                          // Loop forever
    {

        if(RxFlg)
        {
            uint8_t i = 0;
            _DBG_("Message received!");
            PrintMessage((CAN_MSG_Type*)&TxMsg);
            
            // Send back to the PC
            for(i = 0; i < 4; i++)
            {
                TxMsg.dataA[i] = RxMsg.dataA[i];
                TxMsg.dataB[i] = RxMsg.dataB[i];                
            }
            RxFlg = FALSE;
            _DBG_("Send a message...");
            PrintMessage((CAN_MSG_Type*)&TxMsg);
            CAN_SendMsg(USED_CAN_NO,(CAN_MSG_Type*)&TxMsg);
        }
        if(TxFlg)
        {
            TxFlg=FALSE;
            TxMsg.dataA[0] = TxMsg.dataA[1] = TxMsg.dataA[2] = TxMsg.dataA[3] = data_cnt++;
            TxMsg.dataB[0] = TxMsg.dataB[1] = TxMsg.dataB[2] = TxMsg.dataB[3] = data_cnt++;
            _DBG_("Send a message...");
            PrintMessage((CAN_MSG_Type*)&TxMsg);
            CAN_SendMsg(USED_CAN_NO,(CAN_MSG_Type*)&TxMsg);
        }

        CLKPWR_Sleep();                                    // Enter normal sleep mode
    }
}

/* With ARM and GHS toolsets, the entry point is main() - this will
 allow the linker to generate wrapper code to setup stacks, allocate
 heap area, and initialize and copy code and data segments. For GNU
 toolsets, the entry point is through __start() in the crt0_gnu.asm
 file, and that startup code will setup stacks and data */
int main(void) {
    return c_entry();
}


/**
 * @}
 */
