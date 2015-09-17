#ifndef __CARET_H__
#define __CARET_H__
/*
 * COPYRIGHT (C) 2013-2014, Shanghai Real-Thread Technology Co., Ltd
 *
 *  All rights reserved.
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
 */

struct rtgui_caret
{
    struct rtgui_widget *owner;
    /* A buffer dc that contain the caret data. We rely on the dc of the owner
     * to deal with the clip thing. */
    struct rtgui_dc_buffer *caret;
    /* Where we are going to draw on. In logic coordinate. */
    struct rtgui_rect drect;
    rtgui_timer_t *caret_timer;
    /* Caret print count. Even means hidden, odd mean shown. */
    rt_uint8_t prtcnt;
};

void rtgui_caret_init(struct rtgui_caret *car, struct rtgui_widget *owner);

void rtgui_caret_cleanup(struct rtgui_caret *car);

void rtgui_caret_fill(struct rtgui_caret *car, const struct rtgui_rect *drect, const char *text);

int rtgui_caret_is_shown(struct rtgui_caret *car);

void rtgui_caret_toggle(struct rtgui_caret *car);

void rtgui_caret_clear(struct rtgui_caret *car);

void rtgui_caret_show(struct rtgui_caret *car);

void rtgui_caret_ontimeout(rtgui_timer_t *timer, void *parameter);

void rtgui_caret_stop_timer(struct rtgui_caret *car);

rt_err_t rtgui_caret_start_timer(struct rtgui_caret *car, int tick);

#endif /* end of include guard: __CARET_H__ */

