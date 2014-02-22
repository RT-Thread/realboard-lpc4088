#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <rtthread.h>

#define RT_TOUCH_NORMAL             0
#define RT_TOUCH_CALIBRATION_DATA   1
#define RT_TOUCH_CALIBRATION        2

typedef void (*rt_touch_calibration_func_t)(rt_uint16_t x, rt_uint16_t y);

int rtgui_touch_hw_init(void);

#endif
