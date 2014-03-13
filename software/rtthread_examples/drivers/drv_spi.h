/*
 * File      : stm32f20x_40x_spi.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-06-10    xiaonong       The first version for LPC40xx
 */

#ifndef __DRV_SPI_H
#define __DRV_SPI_H

#include <stdint.h>
#include <rtthread.h>
#include <drivers/spi.h>

#include "LPC407x_8x_177x_8x.h"

//#define SPI_USE_DMA

struct lpc_spi_bus
{
    struct rt_spi_bus parent;
    LPC_SSP_TypeDef *SPI;
};

struct lpc_spi_cs
{
    LPC_GPIO_TypeDef *port;
    uint8_t pin;
};

/* public function */
rt_err_t lpc_spi_register(LPC_SSP_TypeDef *SPI,
                          struct lpc_spi_bus *lpc_spi,
                          const char *spi_bus_name);
int rt_hw_spi_init(void);

#endif // __DRV_SPI_H
