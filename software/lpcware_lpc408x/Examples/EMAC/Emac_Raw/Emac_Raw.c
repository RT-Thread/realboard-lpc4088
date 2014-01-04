/**********************************************************************
* $Id$      Emac_Raw.c  2011-06-02
*//**
* @file     Emac_Raw.c
* @brief    This example used to test EMAC operation on LPC1768
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
#include "string.h"
#include "crc32.h"
#include "lpc_emac.h"
#include "lpc_gpio.h"
#include "lpc_pinsel.h"
#include "system_LPC407x_8x_177x_8x.h"
#include "phylan.h"

/* For debugging... */
#include "debug_frmwrk.h"
#include <stdio.h>

/* Example group ----------------------------------------------------------- */
/** @defgroup EMAC_EmacRaw  Emac Raw
 * @ingroup EMAC_Examples
 * @{
 */

/* CONFIGURABLE MACROS ----------------------------------------------- */
/* For the EMAC test, there are two ways to test:
    - TX_ONLY and BOUNCE_RX flags can be set one at a time, not both.
    When TX_ONLY is set to 1, it's a TX_ONLY packet from the MCB1700
    board to the LAN. Use the traffic analyzer such as ethereal, once
    the program is running, the packets can be monitored on the traffic
    analyzer.
    - When BOUNCE_RX is set to 1 (TX_ONLY needs to reset to 0), it's a
    test to test both TX and RX, use the traffic generator/analyzer,
    you can creat a packet with the destination address as that on the
    MCB1700 board, use the traffic generator to send packets, as long
    as the destination address matches, MCB1700 will reverse the source
    and destination address and send the packets back on the network.
    ENABLE_WOL flag is used to test power down and WOL functionality.
    BOUNCE_RX flag needs to be set to 1 when WOL is being tested.
*/
#define TX_ONLY             0
#define BOUNCE_RX           1

#define ENABLE_WOL          1
#define ENABLE_HASH         1


#if TX_ONLY
/* This is the MAC address of LPC1768 */
#define EMAC_ADDR12     0x0000101F
#define EMAC_ADDR34     0x0000E012
#define EMAC_ADDR56     0x00001D0C
/* A pseudo destination MAC address is defined for
 * both TX_ONLY and BOUNCE_RX test */
#define EMAC_DST_ADDR12     0x0000E386
#define EMAC_DST_ADDR34     0x00006BDA
#define EMAC_DST_ADDR56     0x00005000
#endif

#if BOUNCE_RX
/* This is the MAC address of LPC1768 */
#define EMAC_ADDR12     0x0000E386
#define EMAC_ADDR34     0x00006BDA
#define EMAC_ADDR56     0x00005000
/* A pseudo destination MAC address is defined for
 * both TX_ONLY and BOUNCE_RX test */
#define EMAC_DST_ADDR12     0x0000101F
#define EMAC_DST_ADDR34     0x0000E012
#define EMAC_DST_ADDR56     0x00001D0C
#endif

/* LED definitions */
#define PD_LED_PIN      (1<<6)
#define TX_LED_PIN      (1<<5)
#define RX_LED_PIN      (1<<4)
#define KB_LED_PIN      (1<<3)
#define BLINK_LED_PIN   (1<<2)
#define LED2_MASK       ((1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6))
#define LED1_MASK       ((1<<28) | (1<<29) | (1<<31))


/* INTERNAL MACROS ----------------------------------------------- */

#define TX_PACKET_SIZE      114

#define MYMAC_1     ((EMAC_ADDR12 & 0xFF00) >> 8)
#define MYMAC_2     ((EMAC_ADDR12 & 0xFF))
#define MYMAC_3     ((EMAC_ADDR34 & 0xFF00) >> 8)
#define MYMAC_4     ((EMAC_ADDR34 & 0xFF))
#define MYMAC_5     ((EMAC_ADDR56 & 0xFF00) >> 8)
#define MYMAC_6     ((EMAC_ADDR56 & 0xFF))

#define DB  _DBG((uint8_t *)db_)


/*  PRIVATE VARIABLES ----------------------------------------------- */
char db_[64];

#ifdef __IAR_SYSTEMS_ICC__
/* Global Tx Buffer data */
#pragma data_alignment=4
uint8_t gTxBuf[TX_PACKET_SIZE + 0x10];
/* Global Rx Buffer data */
#pragma data_alignment=4
uint8_t gRxBuf[TX_PACKET_SIZE + 0x10];
#else
/* Global Tx Buffer data */
uint8_t __attribute__ ((aligned (4))) gTxBuf[TX_PACKET_SIZE + 0x10];
/* Global Rx Buffer data */
uint8_t __attribute__ ((aligned (4))) gRxBuf[TX_PACKET_SIZE + 0x10];
#endif

/* EMAC address */
uint8_t EMACAddr[] = {MYMAC_6, MYMAC_5, MYMAC_4, MYMAC_3, MYMAC_2, MYMAC_1};
//uint8_t EMACAddr[] = {MYMAC_1, MYMAC_2, MYMAC_3, MYMAC_4, MYMAC_5, MYMAC_6};

#if ENABLE_HASH
uint8_t MulticastAddr[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};      //multicast addresst: start with 0x01
#endif

/* Tx, Rx Counters */
__IO uint32_t RXOverrunCount = 0;
__IO uint32_t RXErrorCount = 0;
__IO uint32_t TXUnderrunCount = 0;
__IO uint32_t TXErrorCount = 0;
__IO uint32_t RxFinishedCount = 0;
__IO uint32_t TxFinishedCount = 0;
__IO uint32_t TxDoneCount = 0;
__IO uint32_t RxDoneCount = 0;
__IO uint32_t ReceiveLength = 0;
__IO Bool PacketReceived = FALSE;

/* Tx Only variables */
#if TX_ONLY
__IO FlagStatus Pressed = RESET;
#endif

#if ENABLE_WOL
__IO uint32_t WOLCount = 0;
#endif

/************************** PRIVATE FUNCTON **********************************/
/* Interrupt service routines */
#if TX_ONLY
void EINT0_Init(void);
void EINT0_IRQHandler(void);
#endif

void PacketGen(uint8_t *txptr);
void LED_Init (void);
void LED_Blink(uint32_t pattern);
void Usr_Init_Emac(void);

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
/*********************************************************************//**
 * @brief       Ethernet error handler
 * @param[in]   None
 * @return      None
 **********************************************************************/

void ErrorReceiveCallback(int32_t errCode)
{
    if((errCode & EMAC_OVERRUN_ERR))
        {
            RXOverrunCount++;
            _DBG_("Rx overrun");
        }

        /*-----------  receive error -------------*/
        /* Note:
         * The EMAC doesn't distinguish the frame type and frame length,
         * so, e.g. when the IP(0x8000) or ARP(0x0806) packets are received,
         * it compares the frame type with the max length and gives the
         * "Range" error. In fact, this bit is not an error indication,
         * but simply a statement by the chip regarding the status of
         * the received frame
         */
        if ((errCode & (EMAC_ALIGN_ERR | EMAC_RANGE_ERR | EMAC_LENGTH_ERR | 
                                EMAC_SYMBOL_ERR| EMAC_CRC_ERR | EMAC_RX_NO_DESC_ERR)))
        {
                RXErrorCount++;
                _DBG_("Rx error: ");
        }


        /*------------------- Transmit Underrun -----------------------*/
        if ((errCode & EMAC_UNDERRUN_ERR))
        {
            TXUnderrunCount++;
            _DBG_("Tx under-run");
        }

        /*------------------- Transmit Error --------------------------*/
        if ((errCode & (EMAC_LATE_COLLISION_ERR | EMAC_EXCESSIVE_COLLISION_ERR | 
                              EMAC_EXCESSIVE_DEFER_ERR| EMAC_TX_NO_DESC_ERR)))
        {
            TXErrorCount++;
            _DBG_("Tx error");
        }

#if ENABLE_WOL
        /* ------------------ Wakeup Event Interrupt ------------------*/
        /* Never gone here since interrupt in this
         * functionality has been disable, even if in wake-up mode
         */
        if ((errCode & EMAC_INT_WAKEUP))
        {
            WOLCount++;
        }
#endif
}
 /*********************************************************************//**
  * @brief       Handle the received frame
  * @param[in]   None
  * @return      None
  **********************************************************************/
 void FrameReceiveCallback(uint16_t* pData, uint32_t size)
{
  _DBG_("Rx done");
  RxDoneCount++;
  PacketReceived = TRUE;

  if(size > TX_PACKET_SIZE + 0x10)
     ErrorReceiveCallback(EMAC_INT_RX_ERR);

  memcpy(gRxBuf, pData,size); 
  ReceiveLength = size;
}
 /*********************************************************************//**
  * @brief       Process transmit-finish-event
  * @param[in]   None
  * @return      None
  **********************************************************************/
void TransmitFinishCallback(void)
{
  _DBG_("Tx done");
}

#if TX_ONLY
/*********************************************************************//**
 * @brief       External interrupt 0 service routine handler
 * @param[in]   None
 * @return      None
 **********************************************************************/
void EINT0_IRQHandler(void)
{
    LPC_SC->EXTINT |= 0x1;  //clear the EINT0 flag
    LED_Blink(KB_LED_PIN);
    Pressed = SET;
}
#endif

/*-------------------------PRIVATE FUNCTIONS-----------------------------------*/
/*********************************************************************//**
 * @brief       Create a perfect packet for TX
 * @param[in]   pointer to TX packet
 * @return      None
 **********************************************************************/
void PacketGen( uint8_t *txptr )
{
  int i;
  uint32_t crcValue;
  uint32_t BodyLength = TX_PACKET_SIZE - 14;

  /* Dest address */
#if TX_ONLY && ENABLE_HASH
 *(txptr+0) = MulticastAddr[0];
 *(txptr+1) = MulticastAddr[1];
 *(txptr+2) = MulticastAddr[2];
 *(txptr+3) = MulticastAddr[3];
 *(txptr+4) = MulticastAddr[4];
 *(txptr+5) = MulticastAddr[5];
#else
  *(txptr+0) = EMAC_DST_ADDR56 & 0xFF;
  *(txptr+1) = (EMAC_DST_ADDR56 >> 0x08) & 0xFF;
  *(txptr+2) = EMAC_DST_ADDR34 & 0xFF;
  *(txptr+3) = (EMAC_DST_ADDR34 >> 0x08) & 0xFF;
  *(txptr+4) = EMAC_DST_ADDR12 & 0xFF;
  *(txptr+5) = (EMAC_DST_ADDR12 >> 0x08) & 0xFF;
#endif

  /* Src address */
  *(txptr+6) = EMAC_ADDR56 & 0xFF;
  *(txptr+7) = (EMAC_ADDR56 >> 0x08) & 0xFF;
  *(txptr+8) = EMAC_ADDR34 & 0xFF;
  *(txptr+9) = (EMAC_ADDR34 >> 0x08) & 0xFF;
  *(txptr+10) = EMAC_ADDR12 & 0xFF;
  *(txptr+11) = (EMAC_ADDR12 >> 0x08) & 0xFF;

  /* Type or length, body length is TX_PACKET_SIZE - 14 bytes */
  *(txptr+12) = BodyLength & 0xFF;
  *(txptr+13) = (BodyLength >> 0x08) & 0xFF;

  /* Skip the first 14 bytes for dst, src, and type/length */
  for ( i=0; i < BodyLength; i++ )
  {
    *(txptr+i+14) = 0x55;
  }

  // Calculate CRC
  crcValue = crc32_bfr( txptr, TX_PACKET_SIZE );

  // Add 4-byte CRC
  *(txptr+TX_PACKET_SIZE) = (0xff & crcValue);
  *(txptr+TX_PACKET_SIZE+1) = 0xff & (crcValue >> 8 );
  *(txptr+TX_PACKET_SIZE+2) = 0xff & (crcValue >> 16);
  *(txptr+TX_PACKET_SIZE+3) = 0xff & (crcValue >> 24);
}

/*********************************************************************//**
 * @brief       Init LEDs
 * @param[in]   None
 * @return      None
 **********************************************************************/
void LED_Init (void)
{
//  uint8_t temp;
//
//  PinCfg.Funcnum = 0;
//  PinCfg.OpenDrain = 0;
//  PinCfg.Pinmode = 0;
//  PinCfg.Portnum = 2;
//  for (temp = 2; temp <= 6; temp++){
//      PinCfg.Pinnum = temp;
//      PINSEL_ConfigPin(&PinCfg);
//  }
//
//  PinCfg.Funcnum = 0;
//  PinCfg.OpenDrain = 0;
//  PinCfg.Pinmode = 0;
//  PinCfg.Portnum = 1;
//  PinCfg.Pinnum = 28;
//  PINSEL_ConfigPin(&PinCfg);
//  PinCfg.Pinnum = 29;
//  PINSEL_ConfigPin(&PinCfg);
//  PinCfg.Pinnum = 31;
//  PINSEL_ConfigPin(&PinCfg);
//
//
//  // Set direction to output
//  LPC_GPIO2->FIODIR |= LED2_MASK;
//  LPC_GPIO1->FIODIR |= LED1_MASK;
//
//  /* Turn off all LEDs */
//  LPC_GPIO2->FIOCLR = LED2_MASK;
//  LPC_GPIO1->FIOCLR = LED1_MASK;
}
/*********************************************************************//**
 * @brief       LED blink. This is used for WOL test only
 * @param[in]   None
 * @return      None
 **********************************************************************/
void LED_Blink( uint32_t pattern )
{
//  uint32_t j;
//
//  LPC_GPIO2->FIOSET = pattern;
//  for ( j = 0; j < 0x100000; j++ );
//  LPC_GPIO2->FIOCLR = pattern;
//  for ( j = 0; j < 0x100000; j++ );
}

#if TX_ONLY
/*********************************************************************//**
 * @brief       External interrupt 0 initialize
 * @param[in]   None
 * @return      None
 **********************************************************************/
void EINT0_Init(void)
{
    /* P2.10 as /EINT0 */
    PINSEL_ConfigPin(2,10,1);

    //Initialize EXT registers
    LPC_SC->EXTINT = 0x0;
    LPC_SC->EXTMODE = 0x0;
    LPC_SC->EXTPOLAR = 0x0;

    /* edge sensitive */
    LPC_SC->EXTMODE = 0xF;
    /* falling-edge sensitive */
    LPC_SC->EXTPOLAR = 0x0;
    /* External Interrupt Flag cleared*/
    LPC_SC->EXTINT = 0xF;

    NVIC_SetPriority(EINT0_IRQn, 4);
    NVIC_EnableIRQ(EINT0_IRQn);
}
#endif

/*********************************************************************//**
 * @brief       User EMAC initialize
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Usr_Init_Emac(void)
{
    /* EMAC configuration type */
    EMAC_CFG_Type Emac_Config;
    volatile uint32_t i;
    /*
     * Enable P1 Ethernet Pins:
     * P1.0 - ENET_TXD0
     * P1.1 - ENET_TXD1
     * P1.4 - ENET_TX_EN
     * P1.8 - ENET_CRS
     * P1.9 - ENET_RXD0
     * P1.10 - ENET_RXD1
     * P1.14 - ENET_RX_ER
     * P1.15 - ENET_REF_CLK
     * P1.16 - ENET_MDC
     * P1.17 - ENET_MDIO
     */
    PINSEL_ConfigPin(1,0,1);
    PINSEL_ConfigPin(1,1,1);
    PINSEL_ConfigPin(1,4,1);
    PINSEL_ConfigPin(1,8,1);
    PINSEL_ConfigPin(1,9,1);
    PINSEL_ConfigPin(1,10,1);
    PINSEL_ConfigPin(1,14,1);
    PINSEL_ConfigPin(1,15,1);
    PINSEL_ConfigPin(1,16,1);
    PINSEL_ConfigPin(1,17,1);

#if TX_ONLY
    _DBG_("Init EMAC module on TX");
#endif

#if BOUNCE_RX
    _DBG_("Init EMAC module on RX");
#endif
    sprintf(db_,"MAC[1..6] addr: %X-%X-%X-%X-%X-%X \n\r", \
             EMACAddr[0],  EMACAddr[1],  EMACAddr[2], \
              EMACAddr[3],  EMACAddr[4],  EMACAddr[5]);
    DB;

    Emac_Config.PhyCfg.Mode = EMAC_MODE_AUTO;
    Emac_Config.pbEMAC_Addr = EMACAddr;
    Emac_Config.bPhyAddr = EMAC_PHY_DEFAULT_ADDR;
    Emac_Config.nMaxFrameSize = 1536;
    Emac_Config.pfnPHYInit = PHY_Init;
    Emac_Config.pfnPHYReset = PHY_Reset;
    Emac_Config.pfnFrameReceive = FrameReceiveCallback;
    Emac_Config.pfnErrorReceive = ErrorReceiveCallback;
    Emac_Config.pfnTransmitFinish = TransmitFinishCallback;
    Emac_Config.pfnSoftInt = NULL;
    Emac_Config.pfnWakeup = NULL;
    // Initialize EMAC module with given parameter
    while (EMAC_Init(&Emac_Config) == ERROR){
        // Delay for a while then continue initializing EMAC module
        _DBG_("Error during initializing EMAC, restart after a while");
        for (i = 0x100000; i; i--);
    }
	NVIC_EnableIRQ(ENET_IRQn);
    _DBG_("Initialize EMAC complete");
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main EMAC program body
 * @param[in]   None
 * @return      int
 **********************************************************************/
int c_entry (void)
{
    /* Data Packet format */
    EMAC_PACKETBUF_Type DataPacket;

    uint8_t *txptr;
    uint32_t i = 0;

#if TX_ONLY
    volatile uint32_t j;
#endif

#if BOUNCE_RX
    uint8_t *rxptr;
#endif

    NVIC_SetPriorityGrouping(4);  //sets PRIGROUP to 3:2 (XXX:YY)

    //Init LED
    LED_Init();

    /* Initialize debug via UART0
    * – 115200bps
    * – 8 data bit
    * – No parity
    * – 1 stop bit
    * – No flow control
    */
    debug_frmwrk_init();

    // Init EMAC
    Usr_Init_Emac();

#if TX_ONLY
    EINT0_Init();

    txptr = (uint8_t *)gTxBuf;

    /* pre-format the transmit packets */
    PacketGen(txptr);
#endif

#if ENABLE_HASH
    EMAC_SetHashFilter(MulticastAddr, ENABLE);
    EMAC_SetFilterMode(EMAC_RFC_UCAST_HASH_EN|EMAC_RFC_MCAST_HASH_EN,ENABLE);
#endif


#if BOUNCE_RX
    /* copy just received data from RX buffer to TX buffer and send out */
    txptr = (uint8_t *)gTxBuf;
    rxptr = (uint8_t *)gRxBuf;
#endif

#if ENABLE_WOL

    _DBG_("Enter Sleep mode now...");
    /*
    * On default state, All Multicast frames, All Broadcast frames and Frame that matched
    * with station address (unicast) are accepted.
    * To make WoL is possible, enable Rx Magic Packet and RxFilter Enable WOL
    */
    EMAC_SetFilterMode((EMAC_RFC_PFILT_WOL_EN | EMAC_RFC_MAGP_WOL_EN), ENABLE);

    for (i = 0; i < 5; i++)
    {
        LED_Blink(PD_LED_PIN);  /* Indicating system is in power down now. */
    }

    // Disable irq interrupt
    __disable_irq();

    /* Currently, support Sleep mode */
    /* enter sleep mode */
    LPC_SC->PCON = 0x0;

    /* Sleep Mode*/
    __WFI();

    // CPU will be suspend here...

    /* From power down to WOL, the PLL needs to be reconfigured,
    otherwise, the CCLK will be generated from 4Mhz IRC instead
    of main OSC 12Mhz */
    /* Initialize system clock */
    SystemInit();

    /*
    * Initialize debug via UART
    */
    debug_frmwrk_init();

    /*
    * Init LED
    */
    LED_Init();

    _DBG_("Wake up from sleep mode");

    /* Calling EMACInit() is overkill which also initializes the PHY, the
    main reason to do that is to make sure the descriptors and descriptor
    status for both TX and RX are clean and ready to use. It won't go wrong. */
    Usr_Init_Emac();

    // Re-Enable irq interrupt
    __enable_irq();

#endif                                      /* endif ENABLE_WOL */

#if BOUNCE_RX

#if ENABLE_HASH
        EMAC_SetHashFilter(MulticastAddr, ENABLE);
        EMAC_SetFilterMode(EMAC_RFC_UCAST_HASH_EN|EMAC_RFC_MCAST_HASH_EN,ENABLE);
#endif

    while( 1 )
    {
        LED_Blink(BLINK_LED_PIN);

        if ( PacketReceived == TRUE )
        {
            PacketReceived = FALSE;

            /* Reverse Source and Destination, then copy the body */
            memcpy( (uint8_t *)txptr, (uint8_t *)(rxptr+6), 6);
            *(txptr+6) = EMAC_ADDR56 & 0xFF;
            *(txptr+7) = (EMAC_ADDR56 >> 0x08) & 0xFF;
            *(txptr+8) = EMAC_ADDR34 & 0xFF;
            *(txptr+9) = (EMAC_ADDR34 >> 0x08) & 0xFF;
            *(txptr+10) = EMAC_ADDR12 & 0xFF;
            *(txptr+11) = (EMAC_ADDR12 >> 0x08) & 0xFF;
            memcpy( (uint8_t *)(txptr+12), (uint8_t *)(rxptr+12), (ReceiveLength - 12));

            _DBG_("Send packet");

            DataPacket.pbDataBuf = (uint32_t *)txptr;
            DataPacket.ulDataLen = ReceiveLength;

            EMAC_WritePacketBuffer(&DataPacket);

        }
    }
#endif  /* endif BOUNCE_RX */

#if TX_ONLY
    /* Transmit packets only */
    while ( 1 )
    {
        for ( j = 0; j < 0x200000; j++ );   /* delay */

        while (Pressed == RESET)
        {
            LED_Blink(BLINK_LED_PIN);
        }

        Pressed = RESET;

        txptr = (uint8_t *)gTxBuf;

        _DBG_("Send packet");

        LED_Blink(TX_LED_PIN);

        DataPacket.pbDataBuf = (uint32_t *)txptr;

        // Note that there're 4-byte CRC added

        DataPacket.ulDataLen = TX_PACKET_SIZE + 4;

        EMAC_WritePacketBuffer(&DataPacket);

        //for ( j = 0; j < 0x200000; j++ ); /* delay */
    }
#endif  /* endif TX_ONLY */

    return 0;
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

/*****************************************************************************
**                            End Of File
*****************************************************************************/

/*
 * @}
 */
