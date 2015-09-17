#include <rtgui/rtgui.h>
#include <rtgui/event.h>
#include <rtgui/touch.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_server.h>

static struct rtgui_calibration_ops *_cali_ops = RT_NULL;
static rt_bool_t _calibrating = RT_FALSE;
static rt_uint16_t x_prev = 0, y_prev = 0;

rt_bool_t rtgui_touch_do_calibration(struct rtgui_event_touch *event)
{
#ifdef RTGUI_USING_CALIBRATION
	/* calibration mode */
	if (_calibrating == RT_TRUE)
	{
		/* only do the calibration on touch UP */
		if (event->up_down == RTGUI_TOUCH_UP && _cali_ops != RT_NULL &&
			_cali_ops->calibration_post != RT_NULL)
			_cali_ops->calibration_post(event->x, event->y);

		return RT_FALSE;
	}
#endif

	if (event->up_down == RTGUI_TOUCH_UP)
	{
		x_prev = 0; y_prev = 0;
	}
	else
	{
		if (!((x_prev > event->x + RTGUI_TOUCH_SMOOTH) ||
			(x_prev < event->x - RTGUI_TOUCH_SMOOTH) ||
			(y_prev > event->y + RTGUI_TOUCH_SMOOTH) ||
			(y_prev < event->y - RTGUI_TOUCH_SMOOTH)))
		{
			/* smooth the touch point */
			return RT_FALSE;
		}

		x_prev = event->x;
		y_prev = event->y;
	}

	/* update touch (x, y) with calibrated data */
	if (_cali_ops != RT_NULL && _cali_ops->calibrate_x != RT_NULL)
		event->x = _cali_ops->calibrate_x(event->x, event->y);
	if (_cali_ops != RT_NULL && _cali_ops->calibrate_y != RT_NULL)
		event->y = _cali_ops->calibrate_y(event->x, event->y);

	return RT_TRUE;
}

#ifdef RTGUI_USING_CALIBRATION
void rtgui_touch_enter_calibration(void)
{
	_calibrating = RT_TRUE;
}

void rtgui_touch_exit_calibration(void)
{
	_calibrating = RT_FALSE;
}
#endif

void rtgui_touch_post(int type, rt_uint16_t x, rt_uint16_t y)
{
    struct rtgui_event_touch etouch;
	RTGUI_EVENT_TOUCH_INIT(&etouch);
	etouch.x = x;
	etouch.y = y;
	etouch.up_down = type;

	rtgui_server_post_event(&etouch.parent, sizeof(struct rtgui_event_touch));
}

/* Initializae RTGUI touch with calibration operations */
void rtgui_touch_init(struct rtgui_calibration_ops* ops)
{
	_cali_ops = ops;
}
