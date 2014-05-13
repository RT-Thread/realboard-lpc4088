#include "drv_led.h"

void led_gpio_init(void)
{
    /* led0 : P4.27,led1:P4.15 ,led2:P4.16 ,led3:P4.17*/
    /* set P4.27,P4.15,P4.16,P4.17 as GPIO. */
    LPC_IOCON->P4_27 = 0x00;
    LPC_IOCON->P4_15 = 0x00;
    LPC_IOCON->P4_16 = 0x00;
    LPC_IOCON->P4_17 = 0x00;
    /* set P4.14,P4.15,P4.16,P4.17  output. */
    LPC_GPIO4->DIR |= (0x07 << 15|0x01<<27);
    /* turn off all the led */
    LPC_GPIO4->SET = (0x07 << 15|0x01<<27);
}


