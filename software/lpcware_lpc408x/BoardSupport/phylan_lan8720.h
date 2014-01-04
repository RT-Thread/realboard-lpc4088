/**********************************************************************
* $Id$      phylan_lan8720.h            2011-06-02
*//**
* @file     phylan_lan8720.h
* @brief    Contains all macro definitions and function prototypes
*           support for external PHY IC LAN8720 to work with LAN
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


/* Peripheral group ----------------------------------------------------------- */
/** @defgroup  Phylan_LAN8720   Phylan LAN8720
 * @ingroup LPC CMSIS_Board_Support
 * @{
 */
 
#ifndef __PHY_LAN8720_H_
#define __PHY_LAN8720_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define LAN8720_MII_ADDR            0x01        //Default PHY device address
#define EMAC_PHY_DEFAULT_ADDR       LAN8720_MII_ADDR


/* LAN8720 PHY Basic Registers */
#define LAN8720_PHY_REG_BMCR        0x00        // Basic Mode Control Register
#define LAN8720_PHY_REG_BMSR        0x01        // Basic Mode Status Register


/* LAN8720 PHY Extended Registers */
#define LAN8720_PHY_REG_IDR1        0x02        // PHY Identifier 1
#define LAN8720_PHY_REG_IDR2        0x03        // PHY Identifier 2
#define LAN8720_PHY_REG_ANAR        0x04        // Auto-Negotiation Advertisement
#define LAN8720_PHY_REG_ANLPAR      0x05        // Auto-Neg. Link Partner Abitily
#define LAN8720_PHY_REG_ANER        0x06        // Auto-Neg. Expansion Register


/* LAN8720 PHY Vendor-specific Registers */
#define LAN8720_PHY_REG_SRR         0x10        // Silicon Revision Register
#define LAN8720_PHY_REG_MCSR        0x11        // Mode Control/Status Register
#define LAN8720_PHY_REG_SR          0x12        // Special Register
#define LAN8720_PHY_REG_SECR        0x1A        // Symbol Error Conter Register
#define LAN8720_PHY_REG_CSIR        0x1B        // Control/Status Indication Reg
#define LAN8720_PHY_REG_SITC        0x1C        // Special Internal testability Controls
#define LAN8720_PHY_REG_ISR         0x1D        // Interrupt Source Register
#define LAN8720_PHY_REG_IMR         0x1E        // Interrupt Mask Register
#define LAN8720_PHY_REG_PHYCSR      0x1F        // PHY Special Control/Status Reg


/* PHY Basic Mode Control Register (BMCR) bitmap definitions */
#define LAN8720_PHY_BMCR_RESET_POS              (15)    //Reset
#define LAN8720_PHY_BMCR_LOOPBACK_POS           (14)    //Loop back
#define LAN8720_PHY_BMCR_SPEEDSEL_POS           (13)    //Speed selection
#define LAN8720_PHY_BMCR_AUTONEG_POS            (12)    //Auto Negotiation
#define LAN8720_PHY_BMCR_PWRDWN_POS             (11)    //Power down mode
#define LAN8720_PHY_BMCR_ISOLATE_POS            (10)    //Isolate
#define LAN8720_PHY_BMCR_RESTART_AN_POS         (9)     //Restart auto negotiation
#define LAN8720_PHY_BMCR_DUPLEX_POS             (8)     //Duplex mode
#define LAN8720_PHY_BMCR_COLLISION_POS          (7)     //Collistion test mode

/* PHY Basic Mode Status Status Register (BMSR) bitmap definitions */
#define LAN8720_PHY_BMSR_100BT4_POS             (15)    //100Base-T4
#define LAN8720_PHY_BMSR_100BTX_FULL_POS        (14)    //100Base-TX Full Duplex
#define LAN8720_PHY_BMSR_100BTX_HALF_POS        (13)    //100Base-TX Half Duplex
#define LAN8720_PHY_BMSR_10BT_FULL_POS          (12)    //10Base-TX Full Duplex
#define LAN8720_PHY_BMSR_10BT_HALF_POS          (11)    //10Base-TX Half Duplex
#define LAN8720_PHY_BMSR_AN_COMPLETE_POS        (5)     //Auto-Negotiate Complete
#define LAN8720_PHY_BMSR_REMOTE_FAULT_POS       (4)     //Remote Fault
#define LAN8720_PHY_BMSR_AN_ABILITY_POS         (3)     //Auto-Negotiate Ability
#define LAN8720_PHY_BMSR_LINK_ESTABLISHED_POS   (2)     //Link Status
#define LAN8720_PHY_BMSR_JABBER_DETECT_POS      (1)     //Jabber Detect
#define LAN8720_PHY_BMSR_EXTCAPBILITY_POS       (0)     //Extended Capabilities

//The Common Registers that are using in all PHY IC with EMAC component of LPC1788
#define EMAC_PHY_REG_BMCR                   LAN8720_PHY_REG_BMCR
#define EMAC_PHY_REG_BMSR                   LAN8720_PHY_REG_BMSR
#define EMAC_PHY_REG_IDR1                   LAN8720_PHY_REG_IDR1
#define EMAC_PHY_REG_IDR2                   LAN8720_PHY_REG_IDR2


#define EMAC_PHY_BMCR_RESET                 (1 << LAN8720_PHY_BMCR_RESET_POS)
#define EMAC_PHY_BMCR_POWERDOWN             (1 << LAN8720_PHY_BMCR_PWRDWN_POS)
#define EMAC_PHY_BMCR_SPEED_SEL             (1 << LAN8720_PHY_BMCR_SPEEDSEL_POS)
#define EMAC_PHY_BMCR_DUPLEX                (1 << LAN8720_PHY_BMCR_DUPLEX_POS)
#define EMAC_PHY_BMCR_AN                    (1 << LAN8720_PHY_BMCR_AUTONEG_POS)


#define EMAC_PHY_BMSR_100BT4                (1 << LAN8720_PHY_BMSR_100BT4_POS)
#define EMAC_PHY_BMSR_100TX_FULL            (1 << LAN8720_PHY_BMSR_100BTX_FULL_POS)
#define EMAC_PHY_BMSR_100TX_HALF            (1 << LAN8720_PHY_BMSR_100BTX_HALF_POS)
#define EMAC_PHY_BMSR_10BT_FULL             (1 << LAN8720_PHY_BMSR_10BT_FULL_POS)
#define EMAC_PHY_BMSR_10BT_HALF             (1 << LAN8720_PHY_BMSR_10BT_HALF_POS)
#define EMAC_PHY_BMSR_REMOTE_FAULT          (1 << LAN8720_PHY_BMSR_REMOTE_FAULT_POS)
#define EMAC_PHY_BMSR_LINK_ESTABLISHED      (1 << LAN8720_PHY_BMSR_LINK_ESTABLISHED_POS)


#define LAN8720_PHY_ID1             (0x0007)

#define LAN8720_PHY_ID2_OUI         (0x0030) //Organizationally Unique Identifer Number
#define LAN8720_PHY_ID2_MANF_MODEL  (0x000F) //Manufacturer Model Number
#define LAN8720_PHY_ID2_CRIT        (((LAN8720_PHY_ID2_OUI & 0x3F) << 6) | (LAN8720_PHY_ID2_MANF_MODEL & 0x3F))

#define EMAC_PHY_ID1_CRIT           (LAN8720_PHY_ID1)
#define EMAC_PHY_ID2_CRIT           (LAN8720_PHY_ID2_CRIT)




/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif /* __PHY_LAN8720_H_ */

