#include <rtthread.h>
#include "board.h"
#include "lpc407x_8x_177x_8x.h"

#include "spi_wifi_rw009.h"

/*
SPI RS = P16
WIFI INT: P2_25
*/
#define WIFI_INT_PIN     25
#define WIFI_RST_PIN     21

rt_bool_t spi_wifi_is_busy(void)
{
    if (LPC_GPIO2->PIN & (1 << WIFI_INT_PIN))
    {
        return RT_FALSE;
    }

    return RT_TRUE;
}

void spi_wifi_int_cmd(int cmd)
{
    if (cmd)
    {
        LPC_GPIOINT->IO2IntClr = (1 << WIFI_INT_PIN);
        LPC_GPIOINT->IO2IntEnF |= (0x01 << WIFI_INT_PIN);
    }
    else
    {

        LPC_GPIOINT->IO2IntEnF &= ~(0x01 << WIFI_INT_PIN);
        LPC_GPIOINT->IO2IntClr = (1 << WIFI_INT_PIN);

    }
}

extern void spi_wifi_isr(int vector);
void GPIO_IRQHandler(void)
{
    if ((LPC_GPIOINT->IO2IntStatF >> WIFI_INT_PIN) & 0x01)
    {
        /* disable interrupt */
        spi_wifi_isr(WIFI_INT_PIN);
        LPC_GPIOINT->IO2IntClr = (1 << WIFI_INT_PIN);
    }
}

void spi_wifi_hw_init(void)
{
     /* P2.21 wifi RST */
    {
        LPC_IOCON->P2_21 &= ~0x07;
        LPC_GPIO2->DIR &= ~(0x01 << WIFI_RST_PIN);

        LPC_GPIO2->CLR = (0x01 << WIFI_RST_PIN);
        rt_thread_delay(RT_TICK_PER_SECOND/10);
        LPC_GPIO2->SET = (0x01 << WIFI_RST_PIN);
        rt_thread_delay(RT_TICK_PER_SECOND/10);
    }

    /* P2.25 wifi INT */
    {
        LPC_IOCON->P2_25 &= ~0x07;
        LPC_GPIO2->DIR &= ~(0x01 << WIFI_INT_PIN);
    }

    /* Configure  EXTI  */
    LPC_GPIOINT->IO2IntEnF |= (0x01 << WIFI_INT_PIN);
    LPC_GPIOINT->IO2IntClr = (1 << WIFI_INT_PIN);
    NVIC_SetPriority(GPIO_IRQn, ((0x01 << 3) | 0x01));
    NVIC_EnableIRQ(GPIO_IRQn);
}
