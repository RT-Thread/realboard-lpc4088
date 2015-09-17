/*
 * File      : touch.h
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
 * 2014-03-11     Bernard      first version
 */

#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <rtgui/event.h>

#define RTGUI_TOUCH_SMOOTH		8

struct rtgui_calibration_ops
{
	void (*calibration_post)(rt_uint16_t x, rt_uint16_t y);
	rt_uint16_t (*calibrate_x)(rt_uint16_t adc_x, rt_uint16_t adc_y);
	rt_uint16_t (*calibrate_y)(rt_uint16_t adc_x, rt_uint16_t adc_y);
};

void rtgui_touch_init(struct rtgui_calibration_ops* ops);

/* post a (x,y) coordinate to rtgui system */
void rtgui_touch_post(int type, rt_uint16_t x, rt_uint16_t y);

#ifdef RTGUI_USING_CALIBRATION
/* enter calibration mode by calibration APP */
void rtgui_touch_enter_calibration(void);
void rtgui_touch_exit_calibration(void);
#endif

/* do calibration in rtgui server */
rt_bool_t rtgui_touch_do_calibration(struct rtgui_event_touch *event);

#endif

