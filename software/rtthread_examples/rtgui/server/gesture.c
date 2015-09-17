/*
 * File      : gesture.c
 * This file is part of RT-Thread GUI
 * COPYRIGHT (C) 2012-2014, Shanghai Real-Thread Electronic Technology Co.,Ltd
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2014-05-06     Bernard      first version
 */

#include <rtgui/rtgui.h>
#include <rtgui/event.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>

#include "topwin.h"
#include "gesture.h"

static rt_uint16_t _last_x = RT_UINT16_MAX;
static rt_uint16_t _last_y = RT_UINT16_MAX;
#define _GS_HANDLING	((_last_x != RT_UINT16_MAX) && (_last_y  != RT_UINT16_MAX))

static int _rtgui_gesture_recognition(struct rtgui_event_mouse* event)
{
	int gesture = RTGUI_GESTURE_NONE;

	if (_GS_HANDLING)
	{
		rt_int16_t dx, dy;
		rt_int16_t rx, ry;
		struct rtgui_rect rect;

		/* ignore down button again */
		if (event->button & RTGUI_MOUSE_BUTTON_DOWN) return gesture;

		/* get driver rect */
		rtgui_graphic_driver_get_rect(RT_NULL, &rect);
		rx = rtgui_rect_width(rect)/2;
		ry = rtgui_rect_height(rect)/2;

		dx = event->x - _last_x;
		dy = event->y - _last_y;
		if (_UI_ABS(dx) < rx/3 && _UI_ABS(dy) < ry/3)
		{
			/* ignore, little gesture */
			_last_x = RT_UINT16_MAX;
			_last_y = RT_UINT16_MAX;
			return gesture;
		}

		if (dx > 0)
		{
			if (dy > 0)
			{
				if ((float)dx/(float)dy > (float)rx/(float)ry) gesture = RTGUI_GESTURE_RIGHT;
				else gesture = RTGUI_GESTURE_UP;
			}
			else
			{
				dy = _UI_ABS(dy);
				if ((float)dx/(float)dy > (float)rx/(float)ry) gesture = RTGUI_GESTURE_RIGHT;
				else gesture = RTGUI_GESTURE_DOWN;
			}
		}
		else
		{
			dx = _UI_ABS(dx);
			if (dy > 0)
			{
				if ((float)dx/(float)dy > (float)rx/(float)ry) gesture = RTGUI_GESTURE_LEFT;
				else gesture = RTGUI_GESTURE_UP;
			}
			else
			{
				dy = _UI_ABS(dy);
				if ((float)dx/(float)dy > (float)rx/(float)ry) gesture = RTGUI_GESTURE_LEFT;
				else gesture = RTGUI_GESTURE_DOWN;
			}
		}
	}
	else if (event->button & RTGUI_MOUSE_BUTTON_DOWN)
	{
		_last_x = event->x;
		_last_y = event->y;
	}
	else
	{
		_last_x = RT_UINT16_MAX;
		_last_y = RT_UINT16_MAX;
	}

	return gesture;
}

int rtgui_gesture_handle(struct rtgui_event_mouse* event, struct rtgui_topwin *wnd)
{
	rt_uint16_t type;
	struct rtgui_event_gesture gevent;

	type = _rtgui_gesture_recognition(event);
	if (type != RTGUI_GESTURE_NONE)
	{
		RTGUI_EVENT_GESTURE_INIT(&gevent, type);

		gevent.wid = wnd->wid;
		gevent.start_x = _last_x; gevent.start_y = _last_y;
		gevent.end_x = event->x; gevent.end_y = event->y;

		_last_x = RT_UINT16_MAX;
		_last_y = RT_UINT16_MAX;

		/* send gesture event */
        rtgui_send(wnd->app, (struct rtgui_event *)&gevent, sizeof(struct rtgui_event_gesture));
		return 0;
	}

	return -1;
}
