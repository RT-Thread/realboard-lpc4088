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

#include <rtthread.h>
#include <rtgui/rtgui.h>
#include <rtgui/dc.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/widgets/caret.h>

#define _BCP(x)     ((struct rtgui_dc_buffer*)x)

static void _caret_ondraw(struct rtgui_caret *);

void rtgui_caret_init(struct rtgui_caret *car, struct rtgui_widget *owner)
{
    RT_ASSERT(car);

    car->owner       = owner;
    car->caret       = RT_NULL;
    car->caret_timer = RT_NULL;
    car->prtcnt      = 0;
}

void rtgui_caret_cleanup(struct rtgui_caret *car)
{
    if (car->caret)
        rtgui_dc_destory(RTGUI_DC(car->caret));
    if (car->caret_timer)
        rtgui_timer_destory(car->caret_timer);
}

void rtgui_caret_fill(struct rtgui_caret *car,
                       const struct rtgui_rect *drect,
                       const char *text)
{
    struct rtgui_rect brect;
    struct rtgui_dc_buffer *bdc;

	RT_ASSERT(car != RT_NULL);

    if (car->caret == RT_NULL)
    {
        car->caret = _BCP(rtgui_dc_buffer_create(rtgui_rect_width(*drect),
                                                 rtgui_rect_height(*drect)));
    }
    else
    {
        bdc = (struct rtgui_dc_buffer*)car->caret;
        if (bdc->height != rtgui_rect_height(*drect) ||
            bdc->width  != rtgui_rect_width(*drect))
        {
            rtgui_dc_destory(RTGUI_DC(car->caret));
            car->caret = _BCP(rtgui_dc_buffer_create(rtgui_rect_width(*drect),
                                                     rtgui_rect_height(*drect)));
        }
    }

    if (!car->caret)
        return;

    car->drect = *drect;

    bdc = (struct rtgui_dc_buffer*)car->caret;

    RTGUI_DC_BC(car->caret) = RTGUI_WIDGET_BACKGROUND(car->owner);
    RTGUI_DC_FC(car->caret) = RTGUI_WIDGET_FOREGROUND(car->owner);

    /* _fill_rect will fill the rect relative to car->caret. */
    brect.x1 = brect.y1 = 0;
    brect.x2 = bdc->width;
    brect.y2 = bdc->height;
    rtgui_dc_fill_rect(RTGUI_DC(car->caret), &brect);

    /* FIXME: offset to the text? */
    rtgui_dc_draw_text(RTGUI_DC(car->caret), text, &brect);
}

int rtgui_caret_is_shown(struct rtgui_caret *car)
{
    return car->prtcnt % 1;
}

void rtgui_caret_toggle(struct rtgui_caret *car)
{
    RT_ASSERT(car);

    car->prtcnt++;
    _caret_ondraw(car);
}

void rtgui_caret_clear(struct rtgui_caret *car)
{
    RT_ASSERT(car);

    car->prtcnt = 0;
    _caret_ondraw(car);
}

void rtgui_caret_show(struct rtgui_caret *car)
{
    RT_ASSERT(car);

    car->prtcnt = 1;
    _caret_ondraw(car);
}

static void _caret_ondraw(struct rtgui_caret *car)
{
    struct rtgui_dc *dc;
    struct rtgui_dc_buffer *bdc;

    RT_ASSERT(car != RT_NULL);

    if (car->caret == RT_NULL)
        return;

    dc = rtgui_dc_begin_drawing(car->owner);
    if (dc == RT_NULL)
        return;

    bdc = car->caret;
    if (car->prtcnt % 2)
    {
        rt_uint8_t *buf, *obuf;
        int i, bpp, s;

        bpp = rtgui_color_get_bpp(bdc->pixel_format);
        /* Avoid overflow on malloc. */
        RT_ASSERT(bdc->width * bpp < RT_UINT32_MAX / bdc->height);
        s = bdc->width * bdc->height * bpp;
        buf = rtgui_malloc(s);
        if (!buf)
        {
            rtgui_dc_end_drawing(dc);
            return;
        }

        for (i = 0; i < s; i++)
        {
            buf[i] = ~bdc->pixel[i];
        }
        /* Hijack the pixel data to draw the inverted color. */
        obuf = bdc->pixel;
        bdc->pixel = buf;

        rtgui_dc_blit(RTGUI_DC(car->caret), RT_NULL, dc, &car->drect);
        rtgui_dc_end_drawing(dc);

        bdc->pixel = obuf;
        rtgui_free(buf);
    }
    else
    {
        rtgui_dc_blit(RTGUI_DC(car->caret), RT_NULL, dc, &car->drect);
        rtgui_dc_end_drawing(dc);
    }
}

void rtgui_caret_ontimeout(rtgui_timer_t *timer, void *parameter)
{
    struct rtgui_caret *car = parameter;

    rtgui_caret_toggle(car);
}

void rtgui_caret_stop_timer(struct rtgui_caret *car)
{
    if (!car->caret_timer)
        return;

    rtgui_timer_stop(car->caret_timer);
}

rt_err_t rtgui_caret_start_timer(struct rtgui_caret *car, int tick)
{
    if (!car->caret_timer)
    {
        car->caret_timer = rtgui_timer_create(tick,
                                              RT_TIMER_FLAG_PERIODIC,
                                              rtgui_caret_ontimeout,
                                              car);
        if (!car->caret_timer)
            return -RT_ENOMEM;
    }

    rtgui_timer_start(car->caret_timer);
    return RT_EOK;
}

