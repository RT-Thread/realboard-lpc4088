/*
 * File      : drv_uart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009-2013 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2013-06-10     xiaonong      The first version for LPC40xx
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#ifdef RT_USING_SPI
#include "drv_spi.h"
#include "lpc_ssp.h"
/* private rt-thread spi ops function */
static rt_err_t configure(struct rt_spi_device *device, struct rt_spi_configuration *configuration);
static rt_uint32_t xfer(struct rt_spi_device *device, struct rt_spi_message *message);

static struct rt_spi_ops lpc_spi_ops =
{
    configure,
    xfer
};

#ifdef SPI_USE_DMA
static uint8_t dummy = 0xFF;
static void DMA_RxConfiguration(struct lpc_spi_bus *lpc_spi_bus,
                                const void *send_addr,
                                void *recv_addr,
                                rt_size_t size)
{

}
#endif

static rt_err_t configure(struct rt_spi_device *device,
                          struct rt_spi_configuration *configuration)
{
    struct lpc_spi_bus *spi_bus = (struct lpc_spi_bus *)device->bus;
    /* data_width */
    if (configuration->data_width > 3 && configuration->data_width <= 16)
    {
        spi_bus->SPI->CR0 = (configuration->data_width - 1);

    }
    else
    {
        return RT_EIO;
    }
    /* baudrate */
    {
        uint32_t lpc_spi_max_clock;
        uint32_t max_hz;
        uint32_t prescale, div, cmp_clk;
        lpc_spi_max_clock = 18000000;
        max_hz = configuration->max_hz;

        if (max_hz > lpc_spi_max_clock)
        {
            max_hz = lpc_spi_max_clock;
        }
        /* Find closest divider to get at or under the target frequency.
        Use smallest prescale possible and rely on the divider to get
        the closest target frequency */
        div = 0;
        cmp_clk = 0xFFFFFFFF;
        prescale = 2;
        while (cmp_clk > max_hz)
        {
            cmp_clk = PeripheralClock / ((div + 1) * prescale);
            if (cmp_clk >  max_hz)
            {
                div++;
                if (div > 0xFF)
                {
                    div = 0;
                    prescale += 2;
                }
            }
        }
        spi_bus->SPI->CR0 &= (~(0xff << 8));
        spi_bus->SPI->CR0 |= (div << 8);
        spi_bus->SPI->CPSR = prescale;
    }

    /* CPOL */
    if (configuration->mode & RT_SPI_CPOL)
    {
        spi_bus->SPI->CR0 |= (0x01 << 6);
    }
    else
    {
        spi_bus->SPI->CR0 &= ~(0x01 << 6);
    }
    /* CPHA */
    if (configuration->mode & RT_SPI_CPHA)
    {
        spi_bus->SPI->CR0 |= (0x01 << 7);
    }
    else
    {
        spi_bus->SPI->CR0 &= ~(0x01 << 7);
    }
    /*Clear the RxFIFO*/
    {
        uint8_t i;
        uint16_t temp = temp;
        for (i = 0; i < 8; i++)
        {
            temp = spi_bus->SPI->DR;
        }
    }
    /* Enable SPI_MASTER */
    spi_bus->SPI->CR1 = (0x01 << 1);

    return RT_EOK;
}

static rt_uint32_t xfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    struct lpc_spi_bus *spi_bus = (struct lpc_spi_bus *)device->bus;
    struct rt_spi_configuration *config = &device->config;
    struct lpc_spi_cs *spi_cs = device->parent.user_data;
    rt_uint32_t size = message->length;

    /* take CS */
    if (message->cs_take)
    {
        spi_cs->port->CLR |= (0x01 << spi_cs->pin);
    }

    {
        if (config->data_width <= 8)
        {
            const rt_uint8_t *send_ptr = message->send_buf;
            rt_uint8_t *recv_ptr = message->recv_buf;
            //  rt_kprintf("size =%d",size);
            while (size--)
            {
                rt_uint8_t data = 0x00;

                if (send_ptr != RT_NULL)
                {
                    data = *send_ptr++;
                }

                //Wait until the transmit buffer is empty
                while ((spi_bus->SPI->SR & ((0x01 << 1) | (0x01 << 4))) != 0x02);
                // Send the byte
                spi_bus->SPI->DR = data;
                //Wait until a data is received
                while ((spi_bus->SPI->SR & ((0x01 << 2) | (0x01 << 4))) != 0x04);
                // Get the received data
                data = spi_bus->SPI->DR;
                if (recv_ptr != RT_NULL)
                {
                    *recv_ptr++ = data;
                }
            }
        }
        else if (config->data_width <= 16)
        {
            const rt_uint16_t *send_ptr = message->send_buf;
            rt_uint16_t *recv_ptr = message->recv_buf;

            while (size--)
            {
                rt_uint16_t data = 0xFF;

                if (send_ptr != RT_NULL)
                {
                    data = *send_ptr++;
                }

                //Wait until the transmit buffer is empty
                while ((spi_bus->SPI->SR & ((0x01 << 1) | (0x01 << 4))) != 0x02);
                // Send the byte
                spi_bus->SPI->DR = data;

                //Wait until a data is received
                while ((spi_bus->SPI->SR & ((0x01 << 2) | (0x01 << 4))) != 0x04);
                // Get the received data
                data = spi_bus->SPI->DR;

                if (recv_ptr != RT_NULL)
                {
                    *recv_ptr++ = data;
                }
            }
        }
    }

    /* release CS */
    if (message->cs_release)
    {
        spi_cs->port->SET |= (0x01 << spi_cs->pin);
    }

    return message->length;
};

/** \brief init and register lpc spi bus.
 *
 * \param SPI: lpc SPI, e.g: LPC_SSP0,LPC_SSP1,LPC_SSP2.
 * \param lpc_spi: lpc spi bus struct.
 * \param spi_bus_name: spi bus name, e.g: "spi1"
 * \return
 *
 */
rt_err_t lpc_spi_register(LPC_SSP_TypeDef *SPI,
                          struct lpc_spi_bus *lpc_spi,
                          const char *spi_bus_name)
{
    if (SPI == LPC_SSP0)
    {
        lpc_spi->SPI = LPC_SSP0;
        /*enable SSP0 power/clock*/
        LPC_SC->PCONP |= (0x01 << 21);
    }
    else if (SPI == LPC_SSP1)
    {
        lpc_spi->SPI = LPC_SSP1;
        /*enable SSP1 power/clock*/
        LPC_SC->PCONP |= (0x01 << 10);
    }
    else if (SPI == LPC_SSP2)
    {
        lpc_spi->SPI = LPC_SSP2;
        /*enable SSP2 power/clock*/
        LPC_SC->PCONP |= (0x01 << 20);
    }
    else
    {
        return RT_ENOSYS;
    }

    return rt_spi_bus_register(&lpc_spi->parent, spi_bus_name, &lpc_spi_ops);
}
/* SSP2
SPI2_MOSI: P5.0
SPI2_MISO: P5.1
SPI2_SCK : P5.2
CS1: P0.20  TOUCH
*/
int rt_hw_spi_init(void)
{
    /* register spi bus */
    {
        static struct lpc_spi_bus lpc_spi1;
        lpc_spi_register(LPC_SSP1, &lpc_spi1, "spi1");
			  LPC_IOCON->P4_20 &= ~0x07;
        LPC_IOCON->P4_20 |= 0x03;
			  LPC_IOCON->P4_22 &= ~0x07;
        LPC_IOCON->P4_22 |= 0x03;
			  LPC_IOCON->P4_23 &= ~0x07;
        LPC_IOCON->P4_23 |= 0x03;
    }
    /* attach cs */
    {
        static struct rt_spi_device spi_device;
        static struct lpc_spi_cs  spi_cs1;
        /* spi10: P4.21 */
        LPC_IOCON->P4_21 &= ~0x07;
        spi_cs1.port = LPC_GPIO4;
        spi_cs1.pin = 21;
        spi_cs1.port->DIR |= (0x01 << spi_cs1.pin);
        spi_cs1.port->SET |= (0x01 << spi_cs1.pin);
        rt_spi_bus_attach_device(&spi_device, "spi10", "spi1", (void *)&spi_cs1);
    }
		    /* register spi bus */
    {
        static struct lpc_spi_bus lpc_spi0;
        lpc_spi_register(LPC_SSP0, &lpc_spi0, "spi0");
        LPC_IOCON->P2_22 = 0xa2;
        LPC_IOCON->P2_26 = 0xa2;
        LPC_IOCON->P2_27 = 0xa2;
    }
    /* attach cs */
    {
        static struct rt_spi_device spi_device;
        static struct lpc_spi_cs  spi_cs1;
        /* spi20: P2.23 */
        LPC_IOCON->P2_23 &= ~0x07;
        spi_cs1.port = LPC_GPIO2;
        spi_cs1.pin = 23;
        spi_cs1.port->DIR |= (0x01 << spi_cs1.pin);
        spi_cs1.port->SET |= (0x01 << spi_cs1.pin);
        rt_spi_bus_attach_device(&spi_device, "spi00", "spi0", (void *)&spi_cs1);
    }
	
	return 0;
}
INIT_BOARD_EXPORT(rt_hw_spi_init);

#endif
