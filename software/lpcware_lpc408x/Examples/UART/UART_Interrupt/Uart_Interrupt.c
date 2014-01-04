/**********************************************************************
* $Id$      Uart_Interrupt.c            2011-06-02
*//**
* @file     Uart_Interrupt.c
* @brief    This example describes how to using UART in interrupt mode
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
#include "bsp.h"
#include "LPC407x_8x_177x_8x.h"
#include "lpc_uart.h"
#include "lpc_pinsel.h"

/** @defgroup UART_Interrupt    UART Interrupt
 * @ingroup UART_Examples
 * @{
 */


/************************** PRIVATE DEFINTIONS *************************/
#define UART_TEST_NUM       1

#if (UART_TEST_NUM == 0)
#define _LPC_UART           UART_0
#define _UART_IRQ           UART0_IRQn
#define _UART_IRQHander     UART0_IRQHandler
#elif (UART_TEST_NUM == 1)
#define _LPC_UART           UART_1
#define _UART_IRQ           UART1_IRQn
#define _UART_IRQHander     UART1_IRQHandler
#elif (UART_TEST_NUM == 2)
#define _LPC_UART           UART_2
#define _UART_IRQ           UART2_IRQn
#define _UART_IRQHander     UART2_IRQHandler
#elif (UART_TEST_NUM == 3)
#define _LPC_UART           UART_3
#define _UART_IRQ           UART3_IRQn
#define _UART_IRQHander     UART3_IRQHandler
#elif (UART_TEST_NUM == 4)
#define _LPC_UART           UART_4
#define _UART_IRQ           UART4_IRQn
#define _UART_IRQHander     UART4_IRQHandler
#endif
/* buffer size definition */
#define UART_RING_BUFSIZE 256

/* Buf mask */
#define __BUF_MASK (UART_RING_BUFSIZE-1)
/* Check buf is full or not */
#define __BUF_IS_FULL(head, tail) ((tail&__BUF_MASK)==((head+1)&__BUF_MASK))
/* Check buf will be full in next receiving or not */
#define __BUF_WILL_FULL(head, tail) ((tail&__BUF_MASK)==((head+2)&__BUF_MASK))
/* Check buf is empty */
#define __BUF_IS_EMPTY(head, tail) ((head&__BUF_MASK)==(tail&__BUF_MASK))
/* Reset buf */
#define __BUF_RESET(bufidx) (bufidx=0)
#define __BUF_INCR(bufidx)  (bufidx=(bufidx+1)&__BUF_MASK)


/************************** PRIVATE TYPES *************************/
/** @brief UART Ring buffer structure */
typedef struct
{
    __IO uint32_t tx_head;                /*!< UART Tx ring buffer head index */
    __IO uint32_t tx_tail;                /*!< UART Tx ring buffer tail index */
    __IO uint32_t rx_head;                /*!< UART Rx ring buffer head index */
    __IO uint32_t rx_tail;                /*!< UART Rx ring buffer tail index */
    __IO uint8_t  tx[UART_RING_BUFSIZE];  /*!< UART Tx data ring buffer */
    __IO uint8_t  rx[UART_RING_BUFSIZE];  /*!< UART Rx data ring buffer */
} UART_RING_BUFFER_T;


/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[]=
"\n\r********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" UART Interrupt example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" This example used to test UART Interrupt with using ring buffer.\n\r"
"  + Please press any key to be echoed\n\r"
"  + Press 'r' to re-show the welcome string\n\r"
"  + Press ESC to terminate\n\r"
"********************************************************************************\n\r";

uint8_t menu2[] = "\n\rUART Interrupt demo terminated!";

// UART Ring buffer
UART_RING_BUFFER_T rb;

// Current Tx Interrupt enable state
__IO FlagStatus TxIntStat;


/************************** PRIVATE FUNCTIONS *************************/
/* Interrupt service routines */
void _UART_IRQHander(void);
void UART_IntErr(uint8_t bLSErrType);
void UART_IntTransmit(void);
void UART_IntReceive(void);

uint32_t UARTReceive(UART_ID_Type UartID, uint8_t *rxbuf, uint32_t buflen);
uint32_t UARTSend(UART_ID_Type UartID, uint8_t *txbuf, uint32_t buflen);
void print_menu(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       UART0 interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void _UART_IRQHander(void)
{
    uint32_t intsrc, tmp, tmp1;

    /* Determine the interrupt source */
    intsrc = UART_GetIntId(_LPC_UART);
    tmp = intsrc & UART_IIR_INTID_MASK;

    // Receive Line Status
    if (tmp == UART_IIR_INTID_RLS){
        // Check line status
        tmp1 = UART_GetLineStatus(_LPC_UART);
        // Mask out the Receive Ready and Transmit Holding empty status
        tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE \
                | UART_LSR_BI | UART_LSR_RXFE);
        // If any error exist
        if (tmp1) {
                UART_IntErr(tmp1);
        }
    }

    // Receive Data Available or Character time-out
    if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI)){
            UART_IntReceive();
    }

    // Transmit Holding Empty
    if (tmp == UART_IIR_INTID_THRE){
            UART_IntTransmit();
    }

}

/********************************************************************//**
 * @brief       UART receive function (ring buffer used)
 * @param[in]   None
 * @return      None
 *********************************************************************/
void UART_IntReceive(void)
{
    uint8_t tmpc;
    uint32_t rLen;

    while(1){
        // Call UART read function in UART driver
        rLen = UART_Receive(_LPC_UART, &tmpc, 1, NONE_BLOCKING);
        // If data received
        if (rLen){
            /* Check if buffer is more space
             * If no more space, remaining character will be trimmed out
             */
            if (!__BUF_IS_FULL(rb.rx_head,rb.rx_tail)){
                rb.rx[rb.rx_head] = tmpc;
                __BUF_INCR(rb.rx_head);
            }
        }
        // no more data
        else {
            break;
        }
    }
}

/********************************************************************//**
 * @brief       UART transmit function (ring buffer used)
 * @param[in]   None
 * @return      None
 *********************************************************************/
void UART_IntTransmit(void)
{
    // Disable THRE interrupt
    UART_IntConfig(_LPC_UART, UART_INTCFG_THRE, DISABLE);

    /* Wait for FIFO buffer empty, transfer UART_TX_FIFO_SIZE bytes
     * of data or break whenever ring buffers are empty */
    /* Wait until THR empty */
    while (UART_CheckBusy(_LPC_UART) == SET);

    while (!__BUF_IS_EMPTY(rb.tx_head,rb.tx_tail))
    {
        /* Move a piece of data into the transmit FIFO */
        if (UART_Send(_LPC_UART, (uint8_t *)&rb.tx[rb.tx_tail], 1, NONE_BLOCKING)){
        /* Update transmit ring FIFO tail pointer */
        __BUF_INCR(rb.tx_tail);
        } else {
            break;
        }
    }

    /* If there is no more data to send, disable the transmit
       interrupt - else enable it or keep it enabled */
    if (__BUF_IS_EMPTY(rb.tx_head, rb.tx_tail)) {
        UART_IntConfig(_LPC_UART, UART_INTCFG_THRE, DISABLE);
        // Reset Tx Interrupt state
        TxIntStat = RESET;
    }
    else{
        // Set Tx Interrupt state
        TxIntStat = SET;
        UART_IntConfig(_LPC_UART, UART_INTCFG_THRE, ENABLE);
    }
}


/*********************************************************************//**
 * @brief       UART Line Status Error
 * @param[in]   bLSErrType  UART Line Status Error Type
 * @return      None
 **********************************************************************/
void UART_IntErr(uint8_t bLSErrType)
{
    // Loop forever
    while (1){
        // For testing purpose
    }
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       UART transmit function for interrupt mode (using ring buffers)
 * @param[in]   UARTPort    Selected UART peripheral used to send data,
 *              should be UART0
 * @param[out]  txbuf Pointer to Transmit buffer
 * @param[in]   buflen Length of Transmit buffer
 * @return      Number of bytes actually sent to the ring buffer
 **********************************************************************/
uint32_t UARTSend(UART_ID_Type UartID, uint8_t *txbuf, uint32_t buflen)
{
    uint8_t *data = (uint8_t *) txbuf;
    uint32_t bytes = 0;

    /* Temporarily lock out UART transmit interrupts during this
       read so the UART transmit interrupt won't cause problems
       with the index values */
    UART_IntConfig(UartID, UART_INTCFG_THRE, DISABLE);

    /* Loop until transmit run buffer is full or until n_bytes
       expires */
    while ((buflen > 0) && (!__BUF_IS_FULL(rb.tx_head, rb.tx_tail)))
    {
        /* Write data from buffer into ring buffer */
        rb.tx[rb.tx_head] = *data;
        data++;

        /* Increment head pointer */
        __BUF_INCR(rb.tx_head);

        /* Increment data count and decrement buffer size count */
        bytes++;
        buflen--;
    }

    /*
     * Check if current Tx interrupt enable is reset,
     * that means the Tx interrupt must be re-enabled
     * due to call UART_IntTransmit() function to trigger
     * this interrupt type
     */
    if (TxIntStat == RESET) {
        UART_IntTransmit();
    }
    /*
     * Otherwise, re-enables Tx Interrupt
     */
    else {
        UART_IntConfig(UartID, UART_INTCFG_THRE, ENABLE);
    }

    return bytes;
}


/*********************************************************************//**
 * @brief       UART read function for interrupt mode (using ring buffers)
 * @param[in]   UARTPort    Selected UART peripheral used to send data,
 *              should be UART0
 * @param[out]  rxbuf Pointer to Received buffer
 * @param[in]   buflen Length of Received buffer
 * @return      Number of bytes actually read from the ring buffer
 **********************************************************************/
uint32_t UARTReceive(UART_ID_Type UartID, uint8_t *rxbuf, uint32_t buflen)
{
    uint8_t *data = (uint8_t *) rxbuf;
    uint32_t bytes = 0;

    /* Temporarily lock out UART receive interrupts during this
       read so the UART receive interrupt won't cause problems
       with the index values */
    UART_IntConfig(UartID, UART_INTCFG_RBR, DISABLE);

    /* Loop until receive buffer ring is empty or
        until max_bytes expires */
    while ((buflen > 0) && (!(__BUF_IS_EMPTY(rb.rx_head, rb.rx_tail))))
    {
        /* Read data from ring buffer into user buffer */
        *data = rb.rx[rb.rx_tail];
        data++;

        /* Update tail pointer */
        __BUF_INCR(rb.rx_tail);

        /* Increment data count and decrement buffer size count */
        bytes++;
        buflen--;
    }

    /* Re-enable UART interrupts */
    UART_IntConfig(UartID, UART_INTCFG_RBR, ENABLE);

    return bytes;
}

/*********************************************************************//**
 * @brief   Print Welcome Screen Menu subroutine
 * @param   None
 * @return  None
 **********************************************************************/
void print_menu(void)
{
    uint32_t tmp, tmp2;
    uint8_t *pDat;

    tmp = sizeof(menu1);
    tmp2 = 0;
    pDat = (uint8_t *)&menu1[0];
    while(tmp)
    {
        tmp2 = UARTSend(_LPC_UART, pDat, tmp);
        pDat += tmp2;
        tmp -= tmp2;
    }
}

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

    uint32_t idx, len;
    FlagStatus exitflag;
    uint8_t buffer[10];

#if (UART_TEST_NUM == 0)
    /*
     * Initialize UART0 pin connect
     * P0.2: U0_TXD
     * P0.3: U0_RXD
     */
    PINSEL_ConfigPin(0,2,1);
    PINSEL_ConfigPin(0,3,1);
#elif (UART_TEST_NUM == 1)
    /*
     * Initialize UART1 pin connect
     * P2.0: U1_TXD
     * P2.1: U1_RXD
     */
    PINSEL_ConfigPin(2,0,2);
    PINSEL_ConfigPin(2,1,2);
#elif (UART_TEST_NUM == 2)
#if (_CURR_USING_OEM_BRD == LPC4088_OEM_BOARD)
    /*
     * Initialize UART2 pin connect
     * P4.22: U2_TXD
     * P4.23: U2_RXD
     */
    PINSEL_ConfigPin(4,22,2);
    PINSEL_ConfigPin(4,23,2);
#else
    /*
     * Initialize UART2 pin connect
     * P0.10: U2_TXD
     * P0.11: U2_RXD
     */
    PINSEL_ConfigPin(0,10,1);
    PINSEL_ConfigPin(0,11,1);
#endif    
#elif (UART_TEST_NUM == 3)
    /*
     * Initialize UART2 pin connect
     * P0.2: U3_TXD
     * P0.3: U3_RXD
     */
    PINSEL_ConfigPin(0,2,2);
    PINSEL_ConfigPin(0,3,2);
#elif (UART_TEST_NUM == 4)
    /*
     * Initialize UART2 pin connect
     * P0.22: U4_TXD
     * P2.9: U4_RXD
     */
    PINSEL_ConfigPin(0,22,3);
    PINSEL_ConfigPin(2,9,3);
#endif

    /* Initialize UART Configuration parameter structure to default state:
     * Baudrate = 115200 bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */
    UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize UART0 peripheral with given to corresponding parameter
    UART_Init(_LPC_UART, &UARTConfigStruct);


    /* Initialize FIFOConfigStruct to default state:
     *              - FIFO_DMAMode = DISABLE
     *              - FIFO_Level = UART_FIFO_TRGLEV0
     *              - FIFO_ResetRxBuf = ENABLE
     *              - FIFO_ResetTxBuf = ENABLE
     *              - FIFO_State = ENABLE
     */
    UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

    // Initialize FIFO for UART0 peripheral
    UART_FIFOConfig(_LPC_UART, &UARTFIFOConfigStruct);


    // Enable UART Transmit
    UART_TxCmd(_LPC_UART, ENABLE);

    /* Enable UART Rx interrupt */
    UART_IntConfig(_LPC_UART, UART_INTCFG_RBR, ENABLE);
    /* Enable UART line status interrupt */
    UART_IntConfig(_LPC_UART, UART_INTCFG_RLS, ENABLE);
    /*
     * Do not enable transmit interrupt here, since it is handled by
     * UART_Send() function, just to reset Tx Interrupt state for the
     * first time
     */
    TxIntStat = RESET;

    // Reset ring buf head and tail idx
    __BUF_RESET(rb.rx_head);
    __BUF_RESET(rb.rx_tail);
    __BUF_RESET(rb.tx_head);
    __BUF_RESET(rb.tx_tail);

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(_UART_IRQ, ((0x01<<3)|0x01));

    /* Enable Interrupt for UART0 channel */
    NVIC_EnableIRQ(_UART_IRQ);


    // print welcome screen
    print_menu();

    // reset exit flag
    exitflag = RESET;

    /* Read some data from the buffer */
    while (exitflag == RESET)
    {
       len = 0;
        while (len == 0)
        {
            len = UARTReceive(_LPC_UART, buffer, sizeof(buffer));
        }

        /* Got some data */
        idx = 0;
        while (idx < len)
        {
            if (buffer[idx] == 27)
            {
                /* ESC key, set exit flag */
                UARTSend(_LPC_UART, menu2, sizeof(menu2));
                exitflag = SET;
            }
            else if (buffer[idx] == 'r')
            {
                /* Echo it back */
                UARTSend(_LPC_UART, &buffer[idx], 1);

                print_menu();
            }
            else
            {
                /* Echo it back */
                UARTSend(_LPC_UART, &buffer[idx], 1);
            }
            idx++;
        }
    }

    // wait for current transmission complete - THR must be empty
    while (UART_CheckBusy(_LPC_UART));

    // DeInitialize UART0 peripheral
    UART_DeInit(_LPC_UART);

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
    c_entry();
    return 0;
}



/**
 * @}
 */
