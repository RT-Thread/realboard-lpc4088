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

#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"
#include "lpc_i2c.h"

#ifdef RT_USING_I2C

#ifdef RT_USING_I2C_BITOPS

struct stm32_i2c_bit_data
{
    struct {
        LPC_GPIO_TypeDef *port;
        uint8_t     pin;
    } scl, sda;
};

static void gpio_set_sda(void *data, rt_int32_t state)
{
    struct stm32_i2c_bit_data *bd = data;

    if (state)
    {
        bd->sda.port->SET = (1<<bd->sda.pin);
    }
    else
    {
        bd->sda.port->CLR = (1<<bd->sda.pin);
    }
}

static void gpio_set_scl(void *data, rt_int32_t state)
{
    struct stm32_i2c_bit_data *bd = data;

    if (state)
    {
        bd->scl.port->SET = (1<<bd->scl.pin);
    }
    else
    {
        bd->scl.port->CLR = (1<<bd->scl.pin);
    }
}

static rt_int32_t gpio_get_sda(void *data)
{
    struct stm32_i2c_bit_data *bd = data;

    return (bd->sda.port->PIN >> bd->sda.pin) & 0x01;
}

static rt_int32_t gpio_get_scl(void *data)
{
    struct stm32_i2c_bit_data *bd = data;

    return (bd->scl.port->PIN >> bd->scl.pin) & 0x01;
}

static void gpio_udelay(rt_uint32_t us)
{
    volatile rt_int32_t i;
    for (; us > 0; us--)
    {
        i = 10;
        while (i--);
    }
}

#else /* RT_USING_I2C_BITOPS */

struct lpc_i2c_bus
{
    struct rt_i2c_bus_device parent;
    LPC_I2C_TypeDef *I2C;
};

static rt_uint32_t lpc_i2c_start(LPC_I2C_TypeDef *I2Cx)
{
    I2Cx->CONCLR = I2C_I2CONCLR_SIC;

    I2Cx->CONSET = I2C_I2CONSET_STA;

    // Wait for complete
    while (!(I2Cx->CONSET & I2C_I2CONSET_SI));

    I2Cx->CONCLR = I2C_I2CONCLR_STAC;

    return (I2Cx->STAT & I2C_STAT_CODE_BITMASK);
}
static rt_err_t lpc_i2c_stop(LPC_I2C_TypeDef *I2Cx)
{
    /* Make sure start bit is not active */
    if (I2Cx->CONSET & I2C_I2CONSET_STA)
    {
        I2Cx->CONCLR = I2C_I2CONCLR_STAC;
    }

    I2Cx->CONSET = I2C_I2CONSET_STO;

    I2Cx->CONCLR = I2C_I2CONCLR_SIC;
    return RT_EOK;
}


static rt_size_t lpc_i2c_recv_bytes(LPC_I2C_TypeDef *I2Cx, struct rt_i2c_msg *msg)
{
    rt_size_t bytes = 0;
    rt_size_t len = msg->len;
    rt_uint32_t stat = 0;
    while (len--)
    {
        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
        if (len == 0)
        {
            I2Cx->CONCLR = I2C_I2CONCLR_AAC;
        }
        else
        {
            I2Cx->CONSET = I2C_I2CONSET_AA;
        }
        while (!(I2Cx->CONSET & I2C_I2CONSET_SI));

        msg->buf[bytes++]  = (uint8_t)(I2Cx->DAT & I2C_I2DAT_BITMASK);
        stat = I2Cx->STAT & I2C_STAT_CODE_BITMASK;
        if (len && (I2C_I2STAT_M_RX_DAT_ACK != stat))
        {
            i2c_dbg("i2c recv error on the byte of %d,send ack error!\n", bytes);
            return bytes;
        }
        else if ( (len == 0) && (I2C_I2STAT_M_RX_DAT_NACK != stat) )
        {
            i2c_dbg("i2c recv error on the byte of %d,send nack error!\n", bytes);
            return bytes;
        }
    }

    return bytes;
}


static rt_size_t lpc_i2c_send_bytes(LPC_I2C_TypeDef *I2Cx, struct rt_i2c_msg *msg)
{
    rt_size_t bytes = 0;
    rt_size_t len = msg->len;
    rt_uint32_t stat = 0;
    /* Make sure start bit is not active */
    if (I2Cx->CONSET & I2C_I2CONSET_STA)
    {
        I2Cx->CONCLR = I2C_I2CONCLR_STAC;
    }
    while (len--)
    {
        I2Cx->CONCLR = I2C_I2CONCLR_SIC;
        I2Cx->DAT = msg->buf[bytes++] & I2C_I2DAT_BITMASK;

        while (!(I2Cx->CONSET & I2C_I2CONSET_SI));

        stat = I2Cx->STAT & I2C_STAT_CODE_BITMASK;
        if (I2C_I2STAT_M_TX_DAT_ACK != stat)
        {
            i2c_dbg("send data error ,i2c is not ack!\n");
            return bytes;
        }
    }

    return bytes;
}
static void i2c_set_clock(LPC_I2C_TypeDef *I2Cx, uint32_t clock)
{
    uint32_t temp;

    temp = PeripheralClock / clock;

    /* Set the I2C clock value to register */
    I2Cx->SCLH = (uint32_t)(temp / 2);

    I2Cx->SCLL = (uint32_t)(temp - I2Cx->SCLH);
}

static rt_uint32_t i2c_send_addr(LPC_I2C_TypeDef *I2Cx, struct rt_i2c_msg *msg)
{
    rt_uint16_t addr;
    rt_uint16_t flags = msg->flags;
    /* Make sure start bit is not active */
    if (I2Cx->CONSET & I2C_I2CONSET_STA)
    {
        I2Cx->CONCLR = I2C_I2CONCLR_STAC;
    }
    /* Test on the direction to set/reset the read/write bit */
    addr = msg->addr << 1;
    if (flags & RT_I2C_RD)
    {
        /* Set the address bit0 for read */
        addr |= 1;
    }
    I2Cx->CONCLR = I2C_I2CONCLR_SIC;
    /* Send the address */
    I2Cx->DAT = addr & I2C_I2DAT_BITMASK;

    while (!(I2Cx->CONSET & I2C_I2CONSET_SI));

    return (I2Cx->STAT & I2C_STAT_CODE_BITMASK);
}


static rt_size_t lpc_i2c_xfer(struct rt_i2c_bus_device *bus,
                              struct rt_i2c_msg msgs[], rt_uint32_t num)
{
    struct rt_i2c_msg *msg;
    rt_uint32_t i;
    rt_err_t ret = RT_ERROR;
    rt_uint32_t stat = 0;
    struct lpc_i2c_bus *lpc_i2c = (struct lpc_i2c_bus *)bus;
    /*start the i2c bus*/
    stat = lpc_i2c_start(lpc_i2c->I2C);
    if ((I2C_I2STAT_M_TX_RESTART != stat) && (I2C_I2STAT_M_TX_START != stat))
    {
        i2c_dbg("start the i2c bus failed,i2c bus stop!\n");
        goto out;
    }
    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];
        if (!(msg->flags & RT_I2C_NO_START))
        {
            if (i)
            {
                stat = lpc_i2c_start(lpc_i2c->I2C);
                if ((I2C_I2STAT_M_TX_RESTART != stat) && (I2C_I2STAT_M_TX_START != stat))
                {
                    i2c_dbg("restart the i2c bus failed,i2c bus stop!\n");
                    goto out;
                }
            }
            stat = i2c_send_addr(lpc_i2c->I2C, msg);
            if (I2C_I2STAT_M_TX_SLAW_ACK != stat && I2C_I2STAT_M_RX_SLAR_ACK != stat)
            {
                i2c_dbg("send i2c address but no ack,i2c stop!");
                goto out;
            }
        }
        if (msg->flags & RT_I2C_RD)
        {
            ret = lpc_i2c_recv_bytes(lpc_i2c->I2C, msg);
            if (ret >= 1)
                i2c_dbg("read %d byte%s\n",
                        ret, ret == 1 ? "" : "s");
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = -RT_EIO;
                goto out;
            }
        }
        else
        {
            ret = lpc_i2c_send_bytes(lpc_i2c->I2C, msg);
            if (ret >= 1)
                i2c_dbg("write %d byte%s\n",
                        ret, ret == 1 ? "" : "s");
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = -RT_ERROR;
                goto out;
            }
        }
    }
    ret = i;

out:
    i2c_dbg("send stop condition\n");
    lpc_i2c_stop(lpc_i2c->I2C);

    return ret;
}


static const struct rt_i2c_bus_device_ops i2c_ops =
{
    lpc_i2c_xfer,
    RT_NULL,
    RT_NULL
};



/** \brief init and register lpc spi bus.
*
* \param SPI: lpc SPI, e.g: LPC_SSP0,LPC_SSP1,LPC_SSP2.
* \param lpc_spi: lpc spi bus struct.
* \param spi_bus_name: spi bus name, e.g: "spi1"
* \return
*
*/
rt_err_t lpc_i2c_register(LPC_I2C_TypeDef *I2Cx,
                          struct lpc_i2c_bus *lpc_i2c,
                          const char *spi_bus_name)
{
    if (I2Cx == LPC_I2C0)
    {
        lpc_i2c->I2C = LPC_I2C0;
        /*enable I2C0 power clock*/
        LPC_SC->PCONP |= (0x01 << 7);
    }
    else if (I2Cx == LPC_I2C1)
    {
        lpc_i2c->I2C = LPC_I2C1;
        /*enable I2C1 power clock*/
        LPC_SC->PCONP |= (0x01 << 19);
    }
    else if (I2Cx == LPC_I2C2)
    {
        lpc_i2c->I2C = LPC_I2C2;
        /*enable I2C2 power clock*/
        LPC_SC->PCONP |= (0x01 << 26);
    }
    else
    {
        return RT_ENOSYS;
    }

    return  rt_i2c_bus_device_register(&lpc_i2c->parent, spi_bus_name);
}
#endif /* RT_USING_I2C_BITOPS */

int rt_hw_i2c_init(void)
{
#ifdef RT_USING_I2C_BITOPS
    /* register I2C1: SCL/P0_20 SDA/P0_19 */
    {
        static struct rt_i2c_bus_device i2c_device;

        static const struct stm32_i2c_bit_data _i2c_bdata =
        {
            /* SCL */ {LPC_GPIO0, 20},
            /* SDA */ {LPC_GPIO0, 19},
        };

        static const struct rt_i2c_bit_ops _i2c_bit_ops =
        {
            (void*)&_i2c_bdata,
            gpio_set_sda,
            gpio_set_scl,
            gpio_get_sda,
            gpio_get_scl,

            gpio_udelay,

            5,
            100
        };

        LPC_IOCON->P0_19 = (1<<10) | (1<<5) | 0; // OD,HYS,GPIO
        LPC_IOCON->P0_20 = (1<<10) | (1<<5) | 0; // OD,HYS,GPIO

        _i2c_bdata.scl.port->SET = (1<<_i2c_bdata.scl.pin);
        _i2c_bdata.sda.port->SET = (1<<_i2c_bdata.sda.pin);
        _i2c_bdata.scl.port->DIR |= (1<<_i2c_bdata.scl.pin);
        _i2c_bdata.sda.port->DIR |= (1<<_i2c_bdata.sda.pin);

        i2c_device.priv = (void *)&_i2c_bit_ops;
        rt_i2c_bit_add_bus(&i2c_device, "i2c1");
    } /* register I2C */


#else /* RT_USING_I2C_BITOPS */
    static struct lpc_i2c_bus lpc_i2c1;
    LPC_IOCON->P0_19 &= ~0x1F;    /*  I2C I/O config */
    LPC_IOCON->P0_19 |= (0x03 | (0x1 << 10)); /* make it open-drain, I2C SDA */
    LPC_IOCON->P0_20 &= ~0x1F;
    LPC_IOCON->P0_20 |= (0x03 | (0x1 << 10)); /* make it open-drain, I2C SCL */
    i2c_set_clock(LPC_I2C1, 100000);
    /* set I2C1 operation to default */
    LPC_I2C1->CONCLR = (I2C_I2CONCLR_AAC | I2C_I2CONCLR_STAC | I2C_I2CONCLR_I2ENC);
    /* enable I2C0 and work in MASTER MODE */
    LPC_I2C1->CONSET = I2C_I2CONSET_I2EN;
    rt_i2c_core_init();
    rt_memset((void *)&lpc_i2c1, 0, sizeof(struct lpc_i2c_bus));
    lpc_i2c1.parent.ops = &i2c_ops;
    lpc_i2c_register(LPC_I2C1, &lpc_i2c1, "i2c1");

#endif /* RT_USING_I2C_BITOPS */

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_i2c_init);

#endif /* RT_USING_I2C */
