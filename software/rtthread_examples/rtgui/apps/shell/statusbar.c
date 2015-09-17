#include <stdio.h>
#include <time.h>

#include "statusbar.h"
#include <rtgui/dc.h>
#include <rtgui/image.h>

#ifdef RT_USING_LWIP
#include "netif/ethernetif.h"
#endif

#define RESOURCE_PATH "/resource/statusbar"
#define TIME_POS  370
#define BGCOLOR   RTGUI_RGB(0,0,0)
static rtgui_win_t *statusbar;
static struct rtgui_image *logo_image, *back_image, *linkup_image, *linkdown_image;
static rtgui_rect_t title_rect = {0, 0, 64, 16};
static rtgui_rect_t time_rect = {TIME_POS, 0, TIME_POS + 40, 12};
static void statusbar_draw(void);
static void update_time(void);
static void update_network_status(void);
static char *title = RT_NULL;
static rt_bool_t backstatus = RT_FALSE;

rt_bool_t statusbar_event_handler(struct rtgui_object *object, struct rtgui_event *event)
{
    switch (event->type)
    {
    case RTGUI_EVENT_PAINT:
    {
        statusbar_draw();
        /* dispatch event */
        rtgui_container_dispatch_event(RTGUI_CONTAINER(object), event);
    }
    break;

    case RTGUI_EVENT_MOUSE_BUTTON:
    {
        struct rtgui_event_mouse *emouse = (struct rtgui_event_mouse *)event;
        struct rtgui_rect start_rect;

        rtgui_widget_get_extent(RTGUI_WIDGET(object), &start_rect);
        start_rect.x1 += 0;
        start_rect.x2 = start_rect.x1 + 80;

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

static void statusbar_draw(void)
{
    struct rtgui_rect rect;
    struct rtgui_dc *dc;
    rtgui_widget_get_rect(RTGUI_WIDGET(statusbar), &rect);
    dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(statusbar));
    RTGUI_DC_BC(dc) = BGCOLOR;
    rtgui_dc_fill_rect(dc, &rect);
    rtgui_dc_end_drawing(dc);
    update_time();
    update_network_status();
    statusbar_set_title(title);
    statusbar_show_back_button(backstatus);
}

/* hh:mm */
static char _time_text[6];
static void update_time(void)
{
    struct rtgui_rect rect;
    struct rtgui_dc *dc;
    const time_t now = time(0);
    struct tm *timeinfo;

    timeinfo = localtime(&now);

    rt_snprintf(_time_text, sizeof(_time_text), "%02d:%02d",
                timeinfo->tm_hour, timeinfo->tm_min);

    rtgui_widget_get_rect(RTGUI_WIDGET(statusbar), &rect);
    dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(statusbar));
    RTGUI_DC_FC(dc) = RTGUI_RGB(255, 255, 255);
    RTGUI_DC_BC(dc) = BGCOLOR;
    rtgui_dc_fill_rect(dc, &time_rect);
    rtgui_dc_draw_text(dc, _time_text, &time_rect);
    rtgui_dc_end_drawing(dc);
}

#ifdef RT_USING_LWIP
static void update_network_status(void)
{
    struct rtgui_rect rect;
    struct rtgui_dc *dc;
    struct rtgui_rect image_rect;
    rtgui_widget_get_rect(RTGUI_WIDGET(statusbar), &rect);
    dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(statusbar));
    rtgui_image_get_rect(linkup_image, &image_rect);
    rtgui_rect_moveto_align(&rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
    image_rect.x1 = rect.x2 - image_rect.x2 - 20;
    image_rect.x2 = rect.x2 - 20;
    RTGUI_DC_BC(dc) = BGCOLOR;
    rtgui_dc_fill_rect(dc, &image_rect);
    if (netif_default->flags & NETIF_FLAG_LINK_UP)
    {
        rtgui_image_blit(linkup_image, dc, &image_rect);
    }
    else
    {
        rtgui_image_blit(linkdown_image, dc, &image_rect);
    }
    rtgui_dc_end_drawing(dc);

}
#else
static void update_network_status(void)
{}
#endif

static rt_uint8_t time_count = 0;
static void timer_timeout(struct rtgui_timer *timer, void *parameter)
{

    time_count++;
    if (time_count == 12)
    {
        update_time();
    }
    update_network_status();
}

void statusbar_show_back_button(rt_bool_t enable)
{
    struct rtgui_dc *dc;
    struct rtgui_rect rect;
    struct rtgui_rect image_rect;

    image_rect.x1 = 5;
    image_rect.x2 = image_rect.x1 + back_image->w;
    image_rect.y1 = 0;
    image_rect.y2 = back_image->h;
    backstatus = enable;
    rtgui_widget_get_rect(RTGUI_WIDGET(statusbar), &rect);
    rtgui_rect_moveto_align(&rect, &image_rect, RTGUI_ALIGN_CENTER_VERTICAL);
    dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(statusbar));
    if (enable)
    {
        rtgui_image_blit(back_image, dc, &image_rect);
    }
    else
    {
        rtgui_image_blit(logo_image, dc, &image_rect);
    }
    rtgui_dc_end_drawing(dc);
}

void statusbar_set_title(char *text)
{
    struct rtgui_dc *dc;
    dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(statusbar));
    RTGUI_DC_BC(dc) = BGCOLOR;
    rtgui_dc_fill_rect(dc, &title_rect);
    RTGUI_DC_FC(dc) = RTGUI_RGB(255, 255, 255);
    title = text;
    if (title != RT_NULL)
    {
        rtgui_dc_draw_text(dc, text, &title_rect);
    }
    else
    {
        rtgui_dc_draw_text(dc, "×ÀÃæ", &title_rect);
    }
    rtgui_dc_end_drawing(dc);
}

void statusbar_init(void)
{
    rtgui_rect_t rect;
    rtgui_timer_t *timer;

    /* get scree rect */
    rtgui_get_screen_rect(&rect);
    rect.y2 = rect.y1 + 24;
    rtgui_rect_moveto_align(&rect, &title_rect, RTGUI_ALIGN_CENTER);
    rtgui_rect_moveto_align(&rect, &time_rect, RTGUI_ALIGN_CENTER_VERTICAL);
    /* create status bar window */
    statusbar = rtgui_win_create(RT_NULL, "StatusBar", &rect, RTGUI_WIN_STYLE_NO_BORDER |
                                 RTGUI_WIN_STYLE_NO_TITLE | RTGUI_WIN_STYLE_ONTOP);
    rtgui_object_set_event_handler(RTGUI_OBJECT(statusbar), statusbar_event_handler);
    /* create start image */
    logo_image  = rtgui_image_create(RESOURCE_PATH"/logo.hdc", RT_FALSE);
    back_image  = rtgui_image_create(RESOURCE_PATH"/back.hdc", RT_FALSE);
    linkup_image  = rtgui_image_create(RESOURCE_PATH"/linkup.hdc", RT_FALSE);
    linkdown_image  = rtgui_image_create(RESOURCE_PATH"/linkdown.hdc", RT_FALSE);

    rtgui_get_screen_rect(&rect);
    rect.y1 = 24;
    /* set the rect information of main window */
    rtgui_set_mainwin_rect(&rect);

    timer = rtgui_timer_create(RT_TICK_PER_SECOND * 5,
                               RT_TIMER_FLAG_PERIODIC, timer_timeout,
                               RT_NULL);
    rtgui_timer_start(timer);

    rtgui_win_show(statusbar, RT_FALSE);
}

