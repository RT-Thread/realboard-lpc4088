#include "statusbar.h"
#include <rtgui/dc.h>
#include <rtgui/image.h>
#include "xpm/start.xpm"

static const rtgui_color_t _status_bar_pixels[] = 
{
//	RTGUI_RGB(228,228,228),
//	RTGUI_RGB(92,158,200),
	RTGUI_RGB(30,116,175),
	RTGUI_RGB(29,114,173),
	RTGUI_RGB(29,113,171),
	RTGUI_RGB(28,110,168),
	RTGUI_RGB(27,108,166),
	RTGUI_RGB(26,106,163),
	RTGUI_RGB(25,103,160),
	RTGUI_RGB(25,101,158),
	RTGUI_RGB(24,98,155),
	RTGUI_RGB(23,97,153),
	RTGUI_RGB(23,95,152),
	RTGUI_RGB(22,94,150),
	RTGUI_RGB(22,93,148),
	RTGUI_RGB(21,91,146),
	RTGUI_RGB(20,90,143),
	RTGUI_RGB(20,88,141),
	RTGUI_RGB(19,86,138),
	RTGUI_RGB(47,91,135),
	RTGUI_RGB(48,93,133),
	RTGUI_RGB(49,95,130),
	RTGUI_RGB(50,98,128),
	RTGUI_RGB(51,100,126),
	RTGUI_RGB(52,102,124),
	RTGUI_RGB(255,255,255),
};

void dc_draw_bar(struct rtgui_dc* dc, const rtgui_color_t *bar_pixel, struct rtgui_rect *rect, int style)
{
	rt_uint32_t index;
	struct rtgui_gc *gc;
	rtgui_color_t fg;

	gc = rtgui_dc_get_gc(dc);
	fg = gc->foreground;

	if (style == RTGUI_HORIZONTAL)
	{
		/* horizontal */
		for (index = rect->y1; index < rect->y2; index ++)
		{
			gc->foreground = bar_pixel[index - rect->y1];
			rtgui_dc_draw_hline(dc, rect->x1, rect->x2, index);
		}
	}
	else 
	{
		/* vertical */
		for (index = rect->x1; index < rect->x2; index ++)
		{
			gc->foreground = bar_pixel[index - rect->x1];
			rtgui_dc_draw_vline(dc, index, rect->y1, rect->y2);
		}
	}
	gc->foreground = fg;
}

rt_bool_t statusbar_event_handler(struct rtgui_object* object, struct rtgui_event* event)
{
	switch (event->type)
	{
	case RTGUI_EVENT_PAINT:
		{
			struct rtgui_dc *dc;
			struct rtgui_rect rect;
			struct rtgui_rect time_rect;
			//struct rtgui_image *image;

			/* create start image */
//			image = rtgui_image_create_from_mem("xpm", (const rt_uint8_t*)start_xpm, sizeof(start_xpm), RT_FALSE);
			rtgui_widget_get_rect(RTGUI_WIDGET(object), &rect);

			dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(object));
			dc_draw_bar(dc, _status_bar_pixels, &rect, RTGUI_HORIZONTAL);
			time_rect.x1=0;
			time_rect.y1=0;
			time_rect.x2=40;
			time_rect.y2=16;
			RTGUI_DC_FC(dc)=RTGUI_RGB(255,255,255);
			rtgui_rect_moveto_align(&rect,&time_rect,RTGUI_ALIGN_CENTER);
      rtgui_dc_draw_text(dc,"12:00",&time_rect);
//			rect.x1 += 15;
//			rtgui_image_blit(image, dc, &rect);

			/* dispatch event */
			rtgui_container_dispatch_event(RTGUI_CONTAINER(object), event);

			rtgui_dc_end_drawing(dc);
		//rtgui_image_destroy(image);
		}
		break;

	case RTGUI_EVENT_MOUSE_BUTTON:
		{
			struct rtgui_event_mouse* emouse = (struct rtgui_event_mouse*)event;
			struct rtgui_rect start_rect;

			rtgui_widget_get_extent(RTGUI_WIDGET(object), &start_rect);
			start_rect.x1 += 15;
			start_rect.x2 = start_rect.x1 + 48;

			/* it's not this widget event, clean status */
			if (rtgui_rect_contains_point(&start_rect, emouse->x, emouse->y) == RT_EOK &&
				emouse->button & (RTGUI_MOUSE_BUTTON_UP))
			{
				rtgui_app_activate(rtgui_app_self());
				break;
			}

			return RT_TRUE;
		}

	default:
		return rtgui_win_event_handler(object, event);
	}

	return RT_FALSE;
}

void statusbar_init(void)
{
	rtgui_rect_t rect;
	struct rtgui_win* win;

	/* get scree rect */
	rtgui_get_screen_rect(&rect);
//	rect.x1=0;
//	rect.x2=480;
//	rect.y1=0;
	rect.y2 = rect.y1 + 24;

	/* create status bar window */
	win = rtgui_win_create(RT_NULL, "StatusBar", &rect, RTGUI_WIN_STYLE_NO_BORDER |
		RTGUI_WIN_STYLE_NO_TITLE | RTGUI_WIN_STYLE_ONTOP);
	rtgui_object_set_event_handler(RTGUI_OBJECT(win), statusbar_event_handler);

	rtgui_get_screen_rect(&rect);
//	rect.x2=480;
//	rect.y2=272;
	rect.y1 = 24;
	/* set the rect information of main window */
	rtgui_set_mainwin_rect(&rect);

	rtgui_win_show(win, RT_FALSE);	
}

