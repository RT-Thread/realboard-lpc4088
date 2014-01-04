/**********************************************************************
* $Id$      transceiver_sn74lvc16245.c          2011-06-02
*//**
* @file     transceiver_sn74lvc16245.c
* @brief    This SN74LVC16245 is mounted on the LPC7188 OEM Board (target
*           board, not for the base board)
*           Contains all functions support this IC transceiver
*           This is actually to do preparation for output of port 3 pins
*           of LPC1788 (P3.0 til P3.31)
*           For Port 4 of the LPC1788, it's already configured by the
*           register of SJ3 jumpers (at position 1-2).
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
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _GPIO
#include "transceiver_sn74lvc16245.h"


/*********************************************************************//**
 * @brief       Init the output for SN74x by enable the OE pin
 * @param[in]   port3Grp    port 3 group number, should be:
 *                  - SN74X_PORT3_GROUP_0: group 0, from pin P3.0 .. P3.7
 *                  - SN74X_PORT3_GROUP_1: group 1, from pin P3.8 .. P3.15
 *                  - SN74X_PORT3_GROUP_2: group 2, from pin P3.16 .. P3.23
 *                  - SN74X_PORT3_GROUP_3: group 3, from pin P3.24 .. P3.31
 * @return      None
 **********************************************************************/
void sn74x_Init(uint8_t port3Grp)
{
    uint8_t portCntrl;
    uint32_t pinMaskCtrl;
    
    GPIO_Init();

    switch(port3Grp)
    {
        case SN74X_PORT3_GROUP_0:
            portCntrl = SN74X_OE0_CONTROLLED_PORT;
            pinMaskCtrl = SN74X_OE0_CONTROLLED_PINMASK;
            break;

        case SN74X_PORT3_GROUP_1:
            portCntrl = SN74X_OE1_CONTROLLED_PORT;
            pinMaskCtrl = SN74X_OE1_CONTROLLED_PINMASK;
            break;

        case SN74X_PORT3_GROUP_2:
            portCntrl = SN74X_OE2_CONTROLLED_PORT;
            pinMaskCtrl = SN74X_OE2_CONTROLLED_PINMASK;
            break;

        case SN74X_PORT3_GROUP_3:
            portCntrl = SN74X_OE3_CONTROLLED_PORT;
            pinMaskCtrl = SN74X_OE3_CONTROLLED_PINMASK;
            break;

        default:
            //trap the problem on input param
            while(1);
            break;
    }

    GPIO_SetDir(portCntrl, pinMaskCtrl, GPIO_DIRECTION_OUTPUT);

    //Set the GPIO to LOW to enable the OE pin for the SN74x IC
    GPIO_ClearValue(portCntrl, pinMaskCtrl);

    return;
}

/*********************************************************************//**
 * @brief       Stop all the output from SN74x by disable OE pin
 * @param[in]   port3Grp    port 3 group number, should be:
 *                  - SN74X_PORT3_GROUP_0: group 0, from pin P3.0 .. P3.7
 *                  - SN74X_PORT3_GROUP_1: group 1, from pin P3.8 .. P3.15
 *                  - SN74X_PORT3_GROUP_2: group 2, from pin P3.16 .. P3.23
 *                  - SN74X_PORT3_GROUP_3: group 3, from pin P3.24 .. P3.31
 * @return      None
 **********************************************************************/
void sn74x_DeInit(uint8_t port3Grp)
{
    uint8_t portCntrl;
    uint32_t pinMaskCtrl;

    switch(port3Grp)
    {
        case SN74X_PORT3_GROUP_0:
            portCntrl = SN74X_OE0_CONTROLLED_PORT;
            pinMaskCtrl = SN74X_OE0_CONTROLLED_PINMASK;
            break;

        case SN74X_PORT3_GROUP_1:
            portCntrl = SN74X_OE1_CONTROLLED_PORT;
            pinMaskCtrl = SN74X_OE1_CONTROLLED_PINMASK;
            break;

        case SN74X_PORT3_GROUP_2:
            portCntrl = SN74X_OE2_CONTROLLED_PORT;
            pinMaskCtrl = SN74X_OE2_CONTROLLED_PINMASK;
            break;

        case SN74X_PORT3_GROUP_3:
            portCntrl = SN74X_OE3_CONTROLLED_PORT;
            pinMaskCtrl = SN74X_OE3_CONTROLLED_PINMASK;
            break;

        default:
            //trap the problem on input param
            while(1);
            break;
    }

    GPIO_SetDir(portCntrl, pinMaskCtrl, GPIO_DIRECTION_OUTPUT);

    //Set the GPIO to HIGH to disable the OE pin for the SN74x IC
    GPIO_SetValue(portCntrl, pinMaskCtrl);

    return;
}

/*********************************************************************//**
 * @brief       Stop all the output from SN74x by disable OE pin
 * @param[in]   dataDir data direction, should be:
 *                  - SN74X_GRP_DATA_INPUT: data input
 *                  - SN74X_GRP_DATA_OUTPUT: data output
 * @return      None
 **********************************************************************/
void sn74x_SetDataDir(uint8_t dataDir)
{
    uint32_t val = 0;

    switch (dataDir)
    {
        case SN74X_GRP_DATA_INPUT:
            val = 1;
            break;

        case SN74X_GRP_DATA_OUTPUT:
            val = 0;
            break;

        default:
            // Trap the error input
            while(1);
            break;
    }

    GPIO_OutputValue(SN74X_DIR_CONTROLLED_PORT, SN74X_DIR_CONTROLLED_PINMASK, val);

    return;
}
#endif /*_GPIO*/
/*********************************************************************************
**                            End Of File
*********************************************************************************/
