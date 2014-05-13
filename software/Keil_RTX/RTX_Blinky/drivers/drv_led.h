#ifndef __DRV_LED_H
#define __DRV_LED_H
#include "lpc407x_8x_177x_8x.h"

#define LED0    (0x01<<27)
#define LED1    (0x01<<15)
#define LED2    (0x01<<16)
#define LED3    (0x01<<17)

#define led_ctrl_on(x)   LPC_GPIO4->CLR=(x)
#define led_ctrl_off(x)  LPC_GPIO4->SET=(x)

void led_gpio_init(void);

#endif
