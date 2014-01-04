/**********************************************************************
* $Id$      Uart_FullModem.c            2011-06-02
*//**
* @file     Uart_FullModem.c
* @brief    This example describes how to use UART1 full-modem function
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
#include "bsp.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup UART_FullModem    UART Full Modem
 * @ingroup UART_Examples
 * @{
 */
#define _LPC_UART       UART_1


/************************** PRIVATE DEFINITIONS *************************/

// buffer size definition
#define UART_RING_BUFSIZE 256

/* Auto RTS and Auto CTS definition:
 * - 1: Enable Auto RTS and CTS function
 * - 0: Disable this function, in this case, handle manually
 * modem functionality */
#define AUTO_RTS_CTS_USE    0

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
" UART Full Modem example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - UART Communication: 115200 bps \n\r"
" By this example, UART is configured to Full Modem Mode.\n\r"
"  + Please press any key to be echoed\n\r"
"  + Press 'r' to re-show the welcome string\n\r"
"  + Press ESC to terminate\n\r"
"********************************************************************************\n\r";

uint8_t menu2[] = "UART Full Modem is terminated!\n\r";

// UART Ring buffer
UART_RING_BUFFER_T rb;

// RTS State
__IO int32_t RTS_State;

// Current Tx Interrupt enable state
__IO FlagStatus TxIntStat;


/************************** PRIVATE FUNCTIONS *************************/
/* Interrupt service routines */
void UART1_IRQHandler(void);
void UART1_IntTransmit(void);
void UART1_IntReceive(void);
void UART1_IntErr(uint8_t bLSErrType);

uint32_t UARTReceive(UART_ID_Type UartID, uint8_t *rxbuf, uint32_t buflen);
uint32_t UARTSend(UART_ID_Type UartID, uint8_t *txbuf, uint32_t buflen);
void print_menu(void);


/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       UART1 interrupt handler sub-routine
 * @param[in]   None
 * @return      None
 **********************************************************************/
void UART1_IRQHandler(void)
{
    uint8_t modemSts;
    uint32_t intSrc, curIntr, lineSts;

    /* Determine the interrupt source */
    intSrc = UART_GetIntId(_LPC_UART);

    curIntr = intSrc & UART_IIR_INTID_MASK;

    /*
     * In case of using UART1 with full modem,
     * interrupt ID = 0 that means modem status interrupt has been detected
     */

    if (curIntr == UART1_IIR_INTID_MODEM)
    {
        // Check Modem status
        modemSts = UART_FullModemGetStatus(_LPC_UART);

#if (AUTO_RTS_CTS_USE == 0)
        // Check CTS status change flag
        if (modemSts & UART1_MODEM_STAT_DELTA_CTS)
        {
            // if CTS status is active, continue to send data
            if (modemSts & UART1_MODEM_STAT_CTS)
            {
                // Re-Enable Tx
                UART_TxCmd(_LPC_UART, ENABLE);
            }
            // Otherwise, Stop current transmission immediately
            else
            {
                // Disable Tx
                UART_TxCmd(_LPC_UART, DISABLE);
            }
        }
#endif
    }

    // Receive Line Status
    if (curIntr == UART_IIR_INTID_RLS)
    {
        // Check line status
        lineSts = UART_GetLineStatus(_LPC_UART);

        // Mask out the Receive Ready and Transmit Holding empty status
        lineSts &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE \
                                    | UART_LSR_BI | UART_LSR_RXFE);

        // If any error exist
        if (lineSts)
        {
            UART1_IntErr(lineSts);
        }
    }

    // Receive Data Available or Character time-out
    if ((curIntr == UART_IIR_INTID_RDA) || (curIntr == UART_IIR_INTID_CTI))
    {
        UART1_IntReceive();
    }

    // Transmit Holding Empty
    if (curIntr == UART_IIR_INTID_THRE)
    {
        UART1_IntTransmit();
    }
}

/********************************************************************//**
 * @brief       UART1 receive function (ring buffer used)
 * @param[in]   None
 * @return      None
 *********************************************************************/
void UART1_IntReceive(void)
{
    uint8_t tmpc;
    uint32_t rLen;

    while (1)
    {
        // Call UART read function in UART driver
        rLen = UART_Receive(_LPC_UART, &tmpc, 1, NONE_BLOCKING);

        // If data received
        if (rLen)
        {
            /* If buffer will be full and RTS is driven manually,
             * RTS pin should be forced into INACTIVE state
             */
#if (AUTO_RTS_CTS_USE == 0)
            if (__BUF_WILL_FULL(rb.rx_head, rb.rx_tail))
            {
                if (RTS_State == ACTIVE)
                {
                    // Disable request to send through RTS line
                    UART_FullModemForcePinState(_LPC_UART, UART1_MODEM_PIN_RTS, INACTIVE);

                    RTS_State = INACTIVE;
                }
            }
#endif

            /* Check if buffer is more space
             * If no more space, remaining character will be trimmed out
             */
            if (!__BUF_IS_FULL(rb.rx_head,rb.rx_tail))
            {
                rb.rx[rb.rx_head] = tmpc;

                __BUF_INCR(rb.rx_head);
            }
        }
        // no more data
        else
        {
            break;
        }
    }
}


/********************************************************************//**
 * @brief       UART1 transmit function (ring buffer used)
 * @param[in]   None
 * @return      None
 *********************************************************************/
void UART1_IntTransmit(void)
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
        if (UART_Send(_LPC_UART, (uint8_t *)&rb.tx[rb.tx_tail], 1, NONE_BLOCKING))
        {
            /* Update transmit ring FIFO tail pointer */
            __BUF_INCR(rb.tx_tail);
        }
        else
        {
            break;
        }
    }

    /* If there is no more data to send, disable the transmit
       interrupt - else enable it or keep it enabled */
    if (__BUF_IS_EMPTY(rb.tx_head, rb.tx_tail))
    {
        UART_IntConfig(_LPC_UART, UART_INTCFG_THRE, DISABLE);

        // Reset Tx Interrupt state
        TxIntStat = RESET;
    }
    else
    {
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
void UART1_IntErr(uint8_t bLSErrType)
{
    // Loop forever
    while (1)
    {
        // For testing purpose
    }
}

/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       UART transmit function for interrupt mode (using ring buffers)
 * @param[in]   UARTPort    Selected UART peripheral used to send data,
 *              should be UART1.
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
    if (TxIntStat == RESET)
    {
        UART1_IntTransmit();
    }
    /*
     * Otherwise, re-enables Tx Interrupt
     */
    else
    {
        UART_IntConfig(UartID, UART_INTCFG_THRE, ENABLE);
    }

    return bytes;
}

/*********************************************************************//**
 * @brief       UART read function for interrupt mode (using ring buffers)
 * @param[in]   UARTPort    Selected UART peripheral used to send data,
 *              should be UART1.
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

#if (AUTO_RTS_CTS_USE == 0)
        /* In case of driving RTS manually, this pin should be
         * release into ACTIVE state if buffer is free
         */
        if (RTS_State == INACTIVE)
        {
            if (!__BUF_WILL_FULL(rb.rx_head, rb.rx_tail))
            {
                // Disable request to send through RTS line
                UART_FullModemForcePinState(_LPC_UART, UART1_MODEM_PIN_RTS, ACTIVE);
                RTS_State = ACTIVE;
            }
        }
#endif
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
 * @brief       c_entry: Main UART-FULLMODEM program body
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
    __IO FlagStatus exitflag;
    uint8_t buffer[10];

#if (_CURR_USING_BRD == _EA_PA_BOARD)
    //It does not used port 3 for this example bcs there's no connection
    //from the target OEM board to the base board directly. They use an IC
    //to control this link between 2 boards
    PINSEL_ConfigPin(2, 0, 2);//UART1 - TXD
    PINSEL_ConfigPin(2, 1, 2);//UART1 - RXD
    PINSEL_ConfigPin(2, 2, 2);//UART1 - CTS
    PINSEL_ConfigPin(2, 3, 2);//UART1 - DCD
    PINSEL_ConfigPin(2, 4, 2);//UART1 - DSR
    PINSEL_ConfigPin(2, 5, 2);//UART1 - DTR
    PINSEL_ConfigPin(2, 6, 2);//UART1 - RI
    PINSEL_ConfigPin(0, 22, 1);//UART1 - RTS
#else
    PINSEL_ConfigPin(3, 16, 3);//UART1 - TXD
    PINSEL_ConfigPin(3, 17, 3);//UART1 - RXD
    PINSEL_ConfigPin(3, 18, 3);//UART1 - CTS
    PINSEL_ConfigPin(3, 19, 3);//UART1 - DCD
    PINSEL_ConfigPin(3, 20, 3);//UART1 - DSR
    PINSEL_ConfigPin(3, 21, 3);//UART1 - DTR
    PINSEL_ConfigPin(3, 22, 3);//UART1 - RI
    PINSEL_ConfigPin(3, 30, 2);//UART1 - RTS
#endif

    /* Initialize UART Configuration parameter structure to default state:
     * Baudrate = 115200 bps
     * 8 data bit
     * 1 Stop bit
     * None parity
     */
    UART_ConfigStructInit(&UARTConfigStruct);

    // Initialize UART1 peripheral with given to corresponding parameter
    UART_Init(_LPC_UART, &UARTConfigStruct);

    // Initialize FIFOConfigStruct to default state:
    UARTFIFOConfigStruct.FIFO_DMAMode = DISABLE;
    UARTFIFOConfigStruct.FIFO_Level = UART_FIFO_TRGLEV0;
    UARTFIFOConfigStruct.FIFO_ResetRxBuf = ENABLE;
    UARTFIFOConfigStruct.FIFO_ResetTxBuf = ENABLE;

    // Initialize FIFO for UART1 peripheral
    UART_FIFOConfig(_LPC_UART, &UARTFIFOConfigStruct);

#if (AUTO_RTS_CTS_USE==0)
    /*
     * Determine current state of CTS pin to enable Tx
     * activity
     */
    if (UART_FullModemGetStatus(_LPC_UART) & UART1_MODEM_STAT_CTS)
    {
        // Enable UART Transmit
        UART_TxCmd(_LPC_UART, ENABLE);
    }
#else
    // Enable UART Transmit
    UART_TxCmd(_LPC_UART, ENABLE);
#endif

    // Reset ring buf head and tail idx
    __BUF_RESET(rb.rx_head);
    __BUF_RESET(rb.rx_tail);
    __BUF_RESET(rb.tx_head);
    __BUF_RESET(rb.tx_tail);

#if AUTO_RTS_CTS_USE
    UART_FullModemConfigMode(LPC_UART1, UART1_MODEM_MODE_AUTO_RTS, ENABLE);

    UART_FullModemConfigMode(LPC_UART1, UART1_MODEM_MODE_AUTO_CTS, ENABLE);
#else
    // Enable Modem status interrupt
    UART_IntConfig(_LPC_UART, UART_INTCFG_MS, ENABLE);

    // Enable CTS1 signal transition interrupt
    UART_IntConfig(_LPC_UART, UART_INTCFG_CTS, ENABLE);

    // Force RTS pin state to ACTIVE
    UART_FullModemForcePinState(_LPC_UART, UART1_MODEM_PIN_RTS, ACTIVE);

    //RESET RTS State flag
    RTS_State = ACTIVE;
#endif


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

    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(UART1_IRQn, ((0x01<<3)|0x01));

    /* Enable Interrupt for UART1 channel */
    NVIC_EnableIRQ(UART1_IRQn);

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
            //ESC character received from serial terminal will end the program
            if (buffer[idx] == 27)
            {
                /* ESC key, set exit flag */
                UARTSend(_LPC_UART, menu2, sizeof(menu2));

                exitflag = SET;
            }
            else if (buffer[idx] == 'r')
            {
                /* Echo it back */
                UARTSend(_LPC_UART, &buffer[idx], 3);
                
                print_menu();
            }
            else
            {
                buffer[idx + 1] = '\n';
                buffer[idx + 2] = '\r';

                /* Echo it back */
                UARTSend(_LPC_UART, &buffer[idx], 3);
            }

            idx++;
        }
    }

    // wait for current transmission complete - THR must be empty
    while (UART_CheckBusy(_LPC_UART) == SET);

    // DeInitialize UART1 peripheral
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
