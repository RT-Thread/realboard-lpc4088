#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

#include <rtgui/rtgui.h>
#include <rtgui/touch.h>

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

/** This provide ways to save the calibration_data to user app.
 *
 * calibration_after is a callback after the calibration has finished.  
 * User app could use this function to save the data to some where else, 
 * for example as a file, eeprom etc. No need to call this if you don't 
 * have such function.
 */
void calibration_set_after(rt_bool_t (*calibration_after)(calculate_data_t*cal));

void calibration_init(void*);
void calibration_set_data(calculate_data_t *data);

/* get the calibration ops */
struct rtgui_calibration_ops* calibration_get_ops(void);

#endif /* end of include guard: __CALIBRATION_H__ */

