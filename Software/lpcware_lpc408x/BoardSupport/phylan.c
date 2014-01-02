/**********************************************************************
* $Id$      phylan.c            2011-11-01
*//**
* @file     phylan.c
* @brief    Contains all macro definitions and function prototypes
*           support for external PHY IC LAN8720 to work with LAN
* @version  1.0
* @date     01. November. 2011
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
#ifdef _EMAC
#include "phylan.h"

/** @defgroup  Phylan   Phylan Setting
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */

/*********************************************************************//**
 * @brief       Initialize PHY
 * @param[in]   pConfig PHY Configuration
 * @return      SUCCESS/ERROR
 *
 * Note:
 * This function does:
 *       - Reset PHY by writting to BMCR register
 *       - Setup PHY: PHY Mode, PHY Speed, PHY Full/Half Duplex
 **********************************************************************/
int32_t PHY_Init(EMAC_PHY_CFG_Type *pConfig)
{
   if(PHY_Reset() < 0)
   {
      return (ERROR);
   }   
   
   // Set PHY mode
   if (PHY_SetMode(pConfig->Mode) < 0)
   {
      return (ERROR);
   }
   return SUCCESS;
}
/*********************************************************************//**
 * @brief       Reset PHY
 * @param[in]   None
 * @return      SUCCESS/ERROR
 **********************************************************************/
int32_t PHY_Reset(void)
{
    int32_t regv,tout;
    
    /* Put the DP83848C in reset mode */
    EMAC_Write_PHY(EMAC_PHY_REG_BMCR, EMAC_PHY_BMCR_RESET);

    /* Wait for hardware reset to end. */
    for (tout = EMAC_PHY_RESP_TOUT; tout >= 0; tout--)
    {
        regv = EMAC_Read_PHY (EMAC_PHY_REG_BMCR);

        if (!(regv & (EMAC_PHY_BMCR_RESET | EMAC_PHY_BMCR_POWERDOWN)))
        {
            /* Reset complete, device not Power Down. */
            break;
        }

        if (tout == 0)
        {
            // Time out, return ERROR
            return (ERROR);
        }
    }
    
    return SUCCESS;
}

/*********************************************************************//**
 * @brief       Check specified PHY status in EMAC peripheral
 * @param[in]   ulPHYState  Specified PHY Status Type, should be:
 *                          - EMAC_PHY_STAT_LINK: Link Status
 *                          - EMAC_PHY_STAT_SPEED: Speed Status
 *                          - EMAC_PHY_STAT_DUP: Duplex Status
 * @return      Status of specified PHY status (0 or 1).
 *              (-1) if error.
 *
 * Note:
 * For EMAC_PHY_STAT_LINK, return value:
 * - 0: Link Down
 * - 1: Link Up
 * For EMAC_PHY_STAT_SPEED, return value:
 * - 0: 10Mbps
 * - 1: 100Mbps
 * For EMAC_PHY_STAT_DUP, return value:
 * - 0: Half-Duplex
 * - 1: Full-Duplex
 **********************************************************************/
int32_t PHY_CheckStatus(uint32_t ulPHYState)
{
    int32_t regv, tmp;

    regv = EMAC_Read_PHY (EMAC_PHY_REG_BMSR);

    switch(ulPHYState)
    {
        case EMAC_PHY_STAT_LINK:
            tmp = (regv & EMAC_PHY_BMSR_LINK_ESTABLISHED) ? 1 : 0;
            break;

        case EMAC_PHY_STAT_SPEED:
            tmp = ((regv & EMAC_PHY_BMSR_100BT4)
                        || (regv & EMAC_PHY_BMSR_100TX_FULL)
                        || (regv & EMAC_PHY_BMSR_100TX_HALF)) ? 1 : 0;
            break;

        case EMAC_PHY_STAT_DUP:
            tmp = ((regv & EMAC_PHY_BMSR_100TX_FULL)
                        || (regv & EMAC_PHY_BMSR_10BT_FULL)) ? 1 : 0;
            break;

        default:
            tmp = -1;
            break;
    }

    return (tmp);
}


/*********************************************************************//**
 * @brief       Set specified PHY mode in EMAC peripheral
 * @param[in]   ulPHYState  Specified PHY mode, should be:
 *                          - EMAC_MODE_AUTO
 *                          - EMAC_MODE_10M_FULL
 *                          - EMAC_MODE_10M_HALF
 *                          - EMAC_MODE_100M_FULL
 *                          - EMAC_MODE_100M_HALF
 * @return      Return (0) if no error, otherwise return (-1)
 **********************************************************************/
int32_t PHY_SetMode(uint32_t ulPHYMode)
{
    int32_t id1, id2, tout;

    /* Check if this is a DP83848C PHY. */
    id1 = EMAC_Read_PHY (EMAC_PHY_REG_IDR1);
    id2 = EMAC_Read_PHY (EMAC_PHY_REG_IDR2);

    if ((id1 == EMAC_PHY_ID1_CRIT) && ((id2 >> 4) == EMAC_PHY_ID2_CRIT))
    {
        /* Configure the PHY device */
        switch(ulPHYMode)
        {
            case EMAC_MODE_AUTO:
                /* Use auto-negotiation about the link speed. */
                EMAC_Write_PHY (EMAC_PHY_REG_BMCR, EMAC_PHY_AUTO_NEG);
                /* Wait to complete Auto_Negotiation */
                for (tout = EMAC_PHY_RESP_TOUT; tout>=0; tout--)
                {
                }
                break;

            case EMAC_MODE_10M_FULL:
                /* Connect at 10MBit full-duplex */
                EMAC_Write_PHY (EMAC_PHY_REG_BMCR, EMAC_PHY_FULLD_10M);
                break;

            case EMAC_MODE_10M_HALF:
                /* Connect at 10MBit half-duplex */
                EMAC_Write_PHY (EMAC_PHY_REG_BMCR, EMAC_PHY_HALFD_10M);
                break;

            case EMAC_MODE_100M_FULL:
                /* Connect at 100MBit full-duplex */
                EMAC_Write_PHY (EMAC_PHY_REG_BMCR, EMAC_PHY_FULLD_100M);
                break;

            case EMAC_MODE_100M_HALF:
                /* Connect at 100MBit half-duplex */
                EMAC_Write_PHY (EMAC_PHY_REG_BMCR, EMAC_PHY_HALFD_100M);
                break;

            default:
                // un-supported
                return (-1);
        }
    }
    // It's not correct module ID
    else
    {
        return (-1);
    }

    // Update EMAC configuration with current PHY status
    if (PHY_UpdateStatus() < 0)
    {
        return (-1);
    }

    // Complete
    return (0);
}


/*********************************************************************//**
 * @brief       Auto-Configures value for the EMAC configuration register to
 *              match with current PHY mode
 * @param[in]   None
 * @return      Return (0) if no error, otherwise return (-1)
 *
 * Note: The EMAC configuration will be auto-configured:
 *      - Speed mode.
 *      - Half/Full duplex mode
 **********************************************************************/
int32_t PHY_UpdateStatus(void)
{
    int32_t regv, tout;

    /* Check the link status. */
    for (tout = EMAC_PHY_RESP_TOUT; tout>=0; tout--)
    {
        regv = EMAC_Read_PHY (EMAC_PHY_REG_BMSR);

        //Check Link Status
        if (regv & EMAC_PHY_BMSR_LINK_ESTABLISHED)
        {
            /* Link is on. */
            break;
        }

        if (tout == 0)
        {
            // time out
            return (-1);
        }
    }

    /* Configure Full/Half Duplex mode. */
    if((regv & EMAC_PHY_BMSR_100TX_FULL) || (regv & EMAC_PHY_BMSR_10BT_FULL))
    {
        /* Full duplex is enabled. */
        EMAC_SetFullDuplexMode(ENABLE);
    }
    else if ((regv & EMAC_PHY_BMSR_100TX_HALF) || (regv & EMAC_PHY_BMSR_10BT_HALF))
    {
        /* Half duplex mode. */
        EMAC_SetFullDuplexMode(DISABLE);
    }

    /* Configure 100MBit/10MBit mode. */
    if ((regv & EMAC_PHY_BMSR_100BT4)
                || (regv & EMAC_PHY_BMSR_100TX_FULL)
                || (regv & EMAC_PHY_BMSR_100TX_HALF))
    {
        /* 100MBit mode. */
        EMAC_SetPHYSpeed(ENABLE);
    }
    else if((regv & EMAC_PHY_BMSR_10BT_FULL) || (regv & EMAC_PHY_BMSR_10BT_HALF))
    {
        /* 10MBit mode. */
        EMAC_SetPHYSpeed(DISABLE);
    }

    // Complete
    return (0);
}
/**
 * @}
 */

#endif /*_EMAC*/

