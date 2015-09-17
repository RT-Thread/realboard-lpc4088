/*
 * File      : driver.c
 * This file is part of RTGUI in RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-10-04     Bernard      first version
 */
#include <rtthread.h>
#include <rtgui/driver.h>
#include <rtgui/region.h>
#include <rtgui/rtgui_system.h>
#include <string.h>

extern const struct rtgui_graphic_driver_ops *rtgui_pixel_device_get_ops(int pixel_format);
extern const struct rtgui_graphic_driver_ops *rtgui_framebuffer_get_ops(int pixel_format);

static struct rtgui_graphic_driver _driver;
static struct rtgui_graphic_driver *_current_driver = &_driver;

#ifdef RTGUI_USING_VFRAMEBUFFER
#ifndef RTGUI_VFB_PIXEL_FMT
#define RTGUI_VFB_PIXEL_FMT		RTGRAPHIC_PIXEL_FORMAT_RGB565
#endif

#include <rtgui/dc.h>
static struct rtgui_graphic_driver _vfb_driver = {0};
static void _graphic_driver_vmode_init(void)
{
	if (_vfb_driver.width != _driver.width || _vfb_driver.height != _driver.height)
	{
		if (_vfb_driver.framebuffer != RT_NULL) rtgui_free((void*)_vfb_driver.framebuffer);

		_vfb_driver.device = RT_NULL;
		_vfb_driver.pixel_format = RTGUI_VFB_PIXEL_FMT;
		_vfb_driver.bits_per_pixel = rtgui_color_get_bits(RTGUI_VFB_PIXEL_FMT);
		_vfb_driver.width  = _driver.width;
		_vfb_driver.height = _driver.height;
		_vfb_driver.pitch  = _driver.width * _UI_BITBYTES(_vfb_driver.bits_per_pixel);
		_vfb_driver.framebuffer = rtgui_malloc(_vfb_driver.height * _vfb_driver.pitch);
        rt_memset(_vfb_driver.framebuffer, 0, _vfb_driver.height * _vfb_driver.pitch);
		_vfb_driver.ext_ops = RT_NULL;
		_vfb_driver.ops = rtgui_framebuffer_get_ops(_vfb_driver.pixel_format);
	}
}

void rtgui_graphic_driver_vmode_enter(void)
{
	rtgui_screen_lock(RT_WAITING_FOREVER);
	_current_driver = &_vfb_driver;
}
RTM_EXPORT(rtgui_graphic_driver_vmode_enter);

void rtgui_graphic_driver_vmode_exit(void)
{
	_current_driver = &_driver;
	rtgui_screen_unlock();
}
RTM_EXPORT(rtgui_graphic_driver_vmode_exit);

rt_bool_t rtgui_graphic_driver_is_vmode(void)
{
	if (_current_driver == &_vfb_driver)
		return RT_TRUE;

	return RT_FALSE;
}
RTM_EXPORT(rtgui_graphic_driver_is_vmode);

struct rtgui_dc*
rtgui_graphic_driver_get_rect_buffer(const struct rtgui_graphic_driver *driver,
                                     struct rtgui_rect *r)
{
    int w, h;
    struct rtgui_dc_buffer *buffer;
    rt_uint8_t *pixel, *dst;
    struct rtgui_rect src, rect;

    /* use virtual framebuffer in default */
    if (driver == RT_NULL)
        driver = &_vfb_driver;

    if (r == RT_NULL)
    {
        rtgui_graphic_driver_get_rect(driver, &rect);
    }
    else
    {
        rtgui_graphic_driver_get_rect(driver, &src);
        rect = *r;
        rtgui_rect_intersect(&src, &rect);
    }

    w = rtgui_rect_width (rect);
    h = rtgui_rect_height(rect);
    if (!(w && h) || driver->framebuffer == RT_NULL)
        return RT_NULL;

    /* create buffer DC */
    buffer = (struct rtgui_dc_buffer*)rtgui_dc_buffer_create_pixformat(driver->pixel_format, w, h);
    if (buffer == RT_NULL)
        return (struct rtgui_dc*)buffer;

    /* get source pixel */
    pixel = (rt_uint8_t*)driver->framebuffer
        + rect.y1 * driver->pitch
        + rect.x1 * rtgui_color_get_bpp(driver->pixel_format);

    dst = buffer->pixel;

    while (h--)
    {
        rt_memcpy(dst, pixel, buffer->pitch);

        dst += buffer->pitch;
        pixel += driver->pitch;
    }

    return (struct rtgui_dc*)buffer;
}
RTM_EXPORT(rtgui_graphic_driver_get_rect_buffer);
#else
rt_bool_t rtgui_graphic_driver_is_vmode(void)
{
	return RT_FALSE;
}
RTM_EXPORT(rtgui_graphic_driver_is_vmode);
#endif

/* get default driver */
struct rtgui_graphic_driver *rtgui_graphic_driver_get_default(void)
{
    return _current_driver;
}
RTM_EXPORT(rtgui_graphic_driver_get_default);

void rtgui_graphic_driver_get_rect(const struct rtgui_graphic_driver *driver, rtgui_rect_t *rect)
{
    RT_ASSERT(rect != RT_NULL);

    /* use default driver */
    if (driver == RT_NULL)
        driver = _current_driver;

    rect->x1 = rect->y1 = 0;
    rect->x2 = driver->width;
    rect->y2 = driver->height;
}
RTM_EXPORT(rtgui_graphic_driver_get_rect);

rt_err_t rtgui_graphic_set_device(rt_device_t device)
{
    rt_err_t result;
    struct rt_device_graphic_info info;
	struct rtgui_graphic_ext_ops *ext_ops;

    /* get framebuffer address */
    result = rt_device_control(device, RTGRAPHIC_CTRL_GET_INFO, &info);
    if (result != RT_EOK)
    {
        /* get device information failed */
        return -RT_ERROR;
    }

	/* if the first set graphic device */
	if (_driver.width == 0 || _driver.height == 0)
	{
		rtgui_rect_t rect;

		rtgui_get_mainwin_rect(&rect);
		if (rect.x2 == 0 || rect.y2 == 0)
		{
			rtgui_rect_init(&rect, 0, 0, info.width, info.height);
			/* re-set main-window */
			rtgui_set_mainwin_rect(&rect);
		}
	}

    /* initialize framebuffer driver */
    _driver.device = device;
    _driver.pixel_format = info.pixel_format;
    _driver.bits_per_pixel = info.bits_per_pixel;
    _driver.width = info.width;
    _driver.height = info.height;
    _driver.pitch = _driver.width * _UI_BITBYTES(_driver.bits_per_pixel);
    _driver.framebuffer = info.framebuffer;

	/* get graphic extension operations */
	result = rt_device_control(device, RTGRAPHIC_CTRL_GET_EXT, &ext_ops);
	if (result == RT_EOK)
	{
		_driver.ext_ops = ext_ops;
	}

    if (info.framebuffer != RT_NULL)
    {
        /* is a frame buffer device */
        _driver.ops = rtgui_framebuffer_get_ops(_driver.pixel_format);
    }
    else
    {
        /* is a pixel device */
        _driver.ops = rtgui_pixel_device_get_ops(_driver.pixel_format);
    }

#ifdef RTGUI_USING_HW_CURSOR
	/* set default cursor image */
	rtgui_cursor_set_image(RTGUI_CURSOR_ARROW);
#endif

#ifdef RTGUI_USING_VFRAMEBUFFER
	_graphic_driver_vmode_init();
#endif

    return RT_EOK;
}
RTM_EXPORT(rtgui_graphic_set_device);

/* screen update */
void rtgui_graphic_driver_screen_update(const struct rtgui_graphic_driver *driver, rtgui_rect_t *rect)
{
	if (driver->device != RT_NULL)
	{
	    struct rt_device_rect_info rect_info;

	    rect_info.x = rect->x1;
	    rect_info.y = rect->y1;
	    rect_info.width = rect->x2 - rect->x1;
	    rect_info.height = rect->y2 - rect->y1;
	    rt_device_control(driver->device, RTGRAPHIC_CTRL_RECT_UPDATE, &rect_info);
	}
}
RTM_EXPORT(rtgui_graphic_driver_screen_update);

void rtgui_graphic_driver_set_framebuffer(void *fb)
{
    if (_current_driver)
        _current_driver->framebuffer = fb;
    else
        _driver.framebuffer = fb;
}

/* get video frame buffer */
rt_uint8_t *rtgui_graphic_driver_get_framebuffer(const struct rtgui_graphic_driver *driver)
{
	if (driver == RT_NULL) driver = _current_driver;

    return (rt_uint8_t *)driver->framebuffer;
}
RTM_EXPORT(rtgui_graphic_driver_get_framebuffer);

#ifdef RTGUI_USING_HW_CURSOR
void rtgui_cursor_set_position(rt_uint16_t x, rt_uint16_t y)
{
	rt_uint32_t value;

	if (_current_driver->device != RT_NULL)
	{
		value = (x << 16 | y);
		rt_device_control(_driver.device, RT_DEVICE_CTRL_CURSOR_SET_POSITION, &value);
	}
}

void rtgui_cursor_set_image(enum rtgui_cursor_type type)
{
	rt_uint32_t value;

	if (_current_driver->device != RT_NULL)
	{
		value = type;
		rt_device_control(_driver.device, RT_DEVICE_CTRL_CURSOR_SET_TYPE, &value);
	}
};
#endif

