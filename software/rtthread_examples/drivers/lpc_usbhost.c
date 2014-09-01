/**********************************************************************
* $Id$      lpc_usbhost.c           2011-09-05
*//**
* @file     lpc_usbhost.c
* @brief        Host Controller functions.
* @version  1.0
* @date     05. September. 2011
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
/** @addtogroup USBHostLite
 * @{
 */
#ifdef __BUILD_WITH_EXAMPLE__
#include "lpc_libcfg.h"
#else
#include "lpc_libcfg_default.h"
#endif /* __BUILD_WITH_EXAMPLE__ */
#ifdef _USBHost
/*
**************************************************************************************************************
*                                            INCLUDE HEADER FILES
**************************************************************************************************************
*/
#include <rtthread.h>
#include <rtdevice.h>
#include "lpc_pinsel.h"
#include "ohci.h"

static rt_uint32_t USB_RAM_BASE[1024 * 2] SECTION("USB_STACK");
#define HOST_BASE_ADDR      (rt_uint32_t)&USB_RAM_BASE[0]

/*********************************************************************//**
 * @brief       Init host controller.
 * @param[in]   None.
 * @return      None.
 **********************************************************************/
void  Host_Init(void)
{

    LPC_SC->PCONP   |= 0x80000000;      /* Enable USB Interface */

    LPC_USB->OTGClkCtrl   = 0x00000019;  /* Enable USB host clock, OTG clock & AHB master clock     */
    while ((LPC_USB->OTGClkSt & 0x00000019) != 0x19);

    LPC_USB->StCtrl = 0x1;

    /* Configure USB2*/
	
     /* USB_VBUS */  
    LPC_IOCON->P1_30 &= ~0x07;
	  LPC_IOCON->P1_30 |= 0x02;
	   /* USB_OVRCR2   */
	  LPC_IOCON->P1_31 &= ~0x07;
	  LPC_IOCON->P1_31 |= 0x01;
	  /* USB_D+2  */
    LPC_IOCON->P0_31 &= ~0x07;
	  LPC_IOCON->P0_31 |= 0x01;
    /* USB_PPWR2    */
	  LPC_IOCON->P0_12 &= ~0x07;
	  LPC_IOCON->P0_12 |= 0x01;
		/* USB_CONNECT2 */
	  LPC_IOCON->P0_14 &= ~0x07;
	  LPC_IOCON->P0_14 |= 0x03;
   
    ohci_config(LPC_USB_BASE, HOST_BASE_ADDR, 2);

    /* Enable the USB Interrupt */
    NVIC_EnableIRQ(USB_IRQn);               /* enable USB interrupt */
    NVIC_SetPriority(USB_IRQn, 0);          /* highest priority */
}

/*********************************************************************//**
 * @brief       services the interrupt caused by host controller.
 * @param[in]   None.
 * @return      None.
 **********************************************************************/
void  USB_IRQHandler(void)
{
    ohci_irq_handler();
}

void lpc_usbh_register(void)
{
    Host_Init();

    ohci_usbh_register("usbh");
}

#endif /*_USBHost*/
/**
* @}
*/

