#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__
#include <rtgui/rtgui.h>
#define RT_TOUCH_NORMAL            0
#define RT_TOUCH_CALIBRATION_DATA  1
#define RT_TOUCH_CALIBRATION       2

typedef struct
{
    int x[5], xfb[5];
    int y[5], yfb[5];
} calibration_typedef;
typedef struct 
{
 int  x_coord[3];
 int  y_coord[3];
 int  scaling;
}calculate_data_t;
typedef void (*rt_touch_calibration_func_t)(rt_uint16_t x, rt_uint16_t y);

/** This let the user space to restore the last calibration data.
 *
 * calibration_restore is a callback before calibration started. If it returns
 * RT_TRUE, the calibration won't be started. In this condition, you must setup
 * the calibration_data via something like:
 *
 *     device = rt_device_find("touch");
 *     if(device != RT_NULL)
 *         rt_device_control(device, RT_TOUCH_CALIBRATION_DATA, &data);
 *
 * It it returns RT_FALSE, the normal calibration process will be started. If
 * you don't have such feature, there is no need to call this function. The
 * calibration will always be started by RTGUI.
 */
void calibration_set_restore(rt_bool_t (*calibration_restore)(void));

/** This provide ways to save the calibration_data to user space.
 *
 * calibration_after is a callback after the calibration has finished.  User
 * space could use this function to save the data to some where else. No need
 * to call this if you don't have such function.
 */
void calibration_set_after(rt_bool_t (*calibration_after)(calculate_data_t*cal));

void calibration_init(void);
void calibration_set_data(calculate_data_t *data);
rt_uint16_t  rtgui_calibrate_y(rt_uint16_t adc_x, rt_uint16_t adc_y);
rt_uint16_t  rtgui_calibrate_x(rt_uint16_t adc_x, rt_uint16_t adc_y);
#endif /* end of include guard: __CALIBRATION_H__ */
