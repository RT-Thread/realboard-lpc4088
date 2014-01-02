#include "emac.h"
#include "lpc_emac.h"
#include "lpc_pinsel.h"
#include "phylan.h"
#include <string.h>
#include <stdio.h>


/* For debugging... */
#include "debug_frmwrk.h"
#include <stdio.h>

/* Example group ----------------------------------------------------------- */
/** @defgroup EMAC_uIP	uIP
 * @ingroup EMAC_Examples
 * @{
 */

#define DB	_DBG((uint8_t *)db)
char db[64];

/* Init the LPC17xx ethernet */
BOOL_8 tapdev_init(uint8_t* EMACAddr)
{
	/* EMAC configuration type */
	EMAC_CFG_Type Emac_Config;


#if AUTO_NEGOTIATION_ENA != 0
	Emac_Config.PhyCfg.Mode = EMAC_MODE_AUTO;
#else
	#if (FIX_SPEED == SPEED_100)
		#if (FIX_DUPLEX == FULL_DUPLEX)
			Emac_Config.Mode = EMAC_MODE_100M_FULL;
		#elif (FIX_DUPLEX == HALF_DUPLEX)
			Emac_Config.Mode = EMAC_MODE_100M_HALF;
		#else
			#error Does not support this duplex option
		#endif
	#elif (FIX_SPEED == SPEED_10)
		#if (FIX_DUPLEX == FULL_DUPLEX)
				Emac_Config.Mode = EMAC_MODE_10M_FULL;
		#elif (FIX_DUPLEX == HALF_DUPLEX)
				Emac_Config.Mode = EMAC_MODE_10M_HALF;
		#else
			#error Does not support this duplex option
        #endif
    #else
        #error Does not support this speed option
    #endif
#endif

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

    _DBG_("[DEBUG]Init EMAC module");
    sprintf(db,"[DEBUG]MAC addr: %X-%X-%X-%X-%X-%X \n\r", \
             EMACAddr[0],  EMACAddr[1],  EMACAddr[2], \
              EMACAddr[3],  EMACAddr[4],  EMACAddr[5]);
    DB;

    Emac_Config.PhyCfg.Mode = EMAC_MODE_AUTO;
    Emac_Config.pbEMAC_Addr = EMACAddr;
    Emac_Config.bPhyAddr = EMAC_PHY_DEFAULT_ADDR;
    Emac_Config.nMaxFrameSize = 1536;
    Emac_Config.pfnPHYInit = PHY_Init;
    Emac_Config.pfnPHYReset = PHY_Reset;
    Emac_Config.pfnFrameReceive = NULL;
    Emac_Config.pfnErrorReceive = NULL;
    Emac_Config.pfnTransmitFinish = NULL;
    Emac_Config.pfnSoftInt = NULL;
    Emac_Config.pfnWakeup = NULL;

    // Initialize EMAC module with given parameter
    if (EMAC_Init(&Emac_Config) == ERROR){
        return (FALSE);
    }

    _DBG_("[DEBUG]Init EMAC complete");

    return (TRUE);
}

/* receive an Ethernet frame from MAC/DMA controller */
UNS_32 tapdev_read(void * pPacket)
{
    UNS_32 Size = EMAC_MAX_PACKET_SIZE;
    UNS_32 in_size;


    if(EMAC_GetBufferSts(EMAC_RX_BUFF) == EMAC_BUFF_EMPTY)
        return 0;
    
    // Check Receive data
    in_size = EMAC_GetRxFrameSize();
    if (in_size <= 4){
        return (0);
    }

    // Get size of receive data
    in_size -= 4; // trip out 4-bytes CRC field

    Size = MIN(Size,in_size);

    // Setup Rx packet
    memcpy(pPacket, (uint8_t*)EMAC_GetRxBuffer(), Size);

    // update receive status
    EMAC_UpdateRxConsumeIndex();
    return(Size);
}

/* transmit an Ethernet frame to MAC/DMA controller */
BOOL_8 tapdev_send(void *pPacket, UNS_32 size)
{
    EMAC_PACKETBUF_Type TxPack;

    // Check size
    if(size == 0){
        return(TRUE);
    }

    // check Tx Slot is available
    if (EMAC_GetBufferSts(EMAC_TX_BUFF) != EMAC_BUFF_EMPTY){
        return (FALSE);
    }

    size = MIN(size,EMAC_MAX_PACKET_SIZE);

    TxPack.pbDataBuf= pPacket;
    TxPack.ulDataLen = size;
    EMAC_WritePacketBuffer(&TxPack);

    return(TRUE);
}

/*
 * @}
 */
