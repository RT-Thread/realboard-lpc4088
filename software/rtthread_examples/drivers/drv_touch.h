#ifndef __DRV_TOUCH_H__
#define __DRV_TOUCH_H__

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*touch_isr_t)(void);

int  touch_hw_init(void);
void touch_install_isr(touch_isr_t isr);

rt_err_t tsc2046_hw_init(const char *spi_device_name);
int ft6206_hw_init(void);

#ifdef __cplusplus
}
#endif

#endif
