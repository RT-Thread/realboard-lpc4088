/******************************************************************
 *****                                                        *****
 *****  Name: cs8900.c                                        *****
 *****  Ver.: 1.0                                             *****
 *****  Date: 07/05/2001                                      *****
 *****  Auth: Andreas Dannenberg                              *****
 *****        HTWK Leipzig                                    *****
 *****        university of applied sciences                  *****
 *****        Germany                                         *****
 *****  Func: ethernet packet-driver for use with LAN-        *****
 *****        controller CS8900 from Crystal/Cirrus Logic     *****
 *****                                                        *****
 *****  Keil: Module modified for use with Philips            *****
 *****        LPC2478 EMAC Ethernet controller                *****
 *****                                                        *****
 ******************************************************************/
#include <stdio.h>
#include "EMAC.h"
#include "tcpip.h"
#include "LPC407x_8x_177x_8x.h"
#include "lpc_emac.h"
#include "lpc_pinsel.h"
#include "lpc_clkpwr.h"
#include "phylan.h"

static uint16_t *rptr = NULL;
static uint32_t rx_size[EMAC_MAX_FRAME_NUM];
static uint16_t* rx_ptr[EMAC_MAX_FRAME_NUM];
static uint8_t rx_done = 0, read_done = 0;


// configure port-pins for use with LAN-controller,
// reset it and send the configuration-sequence
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
void Init_EMAC(void)
{
// Initializes the LPC_EMAC ethernet controller
    volatile unsigned int delay;
    /* LPC_EMAC configuration type */
    EMAC_CFG_Type Emac_Config;

    /* LPC_EMAC address */
    uint8_t EMACAddr[] = {MYMAC_1, MYMAC_2, MYMAC_3, MYMAC_4, MYMAC_5, MYMAC_6};

    /* Enable P1 Ethernet Pins. */
    /* on rev. 'A' and later, P1.6 should NOT be set. */
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

    Emac_Config.PhyCfg.Mode= EMAC_MODE_AUTO;
    Emac_Config.pbEMAC_Addr = EMACAddr;
    Emac_Config.bPhyAddr = EMAC_PHY_DEFAULT_ADDR;
    Emac_Config.nMaxFrameSize = 1536;
    Emac_Config.pfnPHYInit = PHY_Init;
    Emac_Config.pfnPHYReset = PHY_Reset;
    Emac_Config.pfnFrameReceive = FrameReceiveCallback;
    Emac_Config.pfnErrorReceive = ErrorReceiveCallback;
    Emac_Config.pfnTransmitFinish = NULL;
    Emac_Config.pfnSoftInt = NULL;
    Emac_Config.pfnWakeup = NULL;
    // Initialize LPC_EMAC module with given parameter
    while (EMAC_Init(&Emac_Config) == ERROR)
    {
    // Delay for a while then continue initializing LPC_EMAC module
        for (delay = 0x100000; delay; delay--);
    }
	NVIC_EnableIRQ(ENET_IRQn);
}
// save the pointer to received frame buffer,
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
void FrameReceiveCallback(uint16_t* pData, uint32_t size)
{

  rx_ptr[rx_done] = pData;
  rx_size[rx_done] = size;
  rx_done++;
  if(rx_done >= EMAC_MAX_FRAME_NUM)
     rx_done = 0;
}

// handle errors
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
void ErrorReceiveCallback(int32_t errCode)
{
  
}
// reads a word in little-endian byte order from RX_BUFFER
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
uint16_t ReadHalfWord_EMAC(void)
{
    return (*rptr++);
}

// reads a word in big-endian byte order from RX_FRAME_PORT
// (useful to avoid permanent byte-swapping while reading
// TCP/IP-data)
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
uint16_t ReadHalfWordBE_EMAC(void)
{
  uint16_t ReturnValue;

  ReturnValue = SwapBytes (*rptr++);
  return (ReturnValue);
}

// copies bytes from frame port to MCU-memory
// NOTES: * an odd number of byte may only be transfered
//          if the frame is read to the end!
//        * MCU-memory MUST start at word-boundary
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
void ReadFrame_EMAC(void *Dest, uint16_t Size)
{
    uint16_t * piDest;
    piDest = Dest;
    while (Size > 1)
    {
        *piDest++ = ReadHalfWord_EMAC();
        Size -= 2;
    }

    if (Size)
    {
    /* check for leftover byte...
    the LAN-Controller will return 0
    for the highbyte*/
        *(uint8_t *)piDest = (char)ReadHalfWord_EMAC();
    }
}

// does a dummy read on frame-I/O-port
// NOTE: only an even number of bytes is read!
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
void DummyReadFrame_EMAC(uint16_t Size)
{
/* discards an EVEN number of bytes from RX-fifo */

    while (Size > 1)
    {
        ReadHalfWord_EMAC();
        Size -= 2;
    }
}
// Reads the length of the received ethernet frame and checks if the
// destination address is a broadcast message or not
// returns the frame length
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
uint16_t StartReadFrame(void)
{
    rptr = rx_ptr[read_done];
    read_done++;
    if(read_done >= EMAC_MAX_FRAME_NUM)
       read_done = 0;
    return(rx_size[read_done]);
}

/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
void EndReadFrame(void)
{
    rx_size[read_done] = 0;
}
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
unsigned int CheckFrameReceived(void)
{
    if (rx_done != read_done)
    {
        return (1);
    }
    else
    {
    return (0);
    }
}

// check if ethernet controller is ready to accept the
// frame we want to send
/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
unsigned int Rdy4Tx(void)
{
    return ((EMAC_GetBufferSts(EMAC_TX_BUFF) == EMAC_BUFF_EMPTY) ? 1:0);
}


/*********************************************************************//**
 * @brief       
 * @param[in]   
 * @return      
 **********************************************************************/
void SendFrame(void *Source, unsigned int Size)
{
    EMAC_PACKETBUF_Type packet;
    packet.pbDataBuf = Source;
    packet.ulDataLen =  Size;
    EMAC_WritePacketBuffer(&packet);
    //EMAC_Transmit(Source,Size);
}

