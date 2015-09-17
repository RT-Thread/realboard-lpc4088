#include "mainmenu.h"
#include <rtgui/widgets/notebook.h>
#include <rtgui/widgets/list_view.h>
#include <rtgui/calibration.h>
#include <stdio.h>
#include <time.h>

#include "statusbar.h"

/* Example Apps. */
#include "apps_list.h"

#define hw_driver               (rtgui_graphic_driver_get_default())
#define STATUSBAR_HEIGHT 24
#define PAGE_MARK_MARGIN 2
#define PAGE_MARK_ITEM_WIDTH  10
#define PAGE_MARK_ITEM_HEIGHT 10

#define LIST_MARGIN        5

typedef void (*on_select_func)(struct app_item *item);

struct app_list_view
{
    /*app list rect*/
    rtgui_rect_t rect, view_rect, pm_rect;
    /* background display point and list view display point */
    struct rtgui_point bgdisp_point, listdisp_point;
    /* list item */
    struct app_item *items;
    /* background buffer*/
    struct rtgui_dc *bg_buffer;
    /* list wiew buffer */
    struct rtgui_dc *view_buffer;
    /* page make buffer */
    struct rtgui_dc *pm_buffer;
    /* total number of items */
    rt_uint16_t items_count;
    /* total number of page */
    rt_uint16_t page_count;
    /* the number of item in a page */
    rt_uint16_t page_items;
    /* current item */
    rt_int16_t current_item;
    /* old page */
    rt_int16_t old_page;
    /* current page */
    rt_int16_t current_page;
    /* icon layout */
    rt_uint8_t row_items, col_items;
    /* */
    on_select_func on_select;
};

static struct app_list_view *app_list;

static rtgui_win_t *win;
static struct app_item items[ITEM_MAX];
static struct rtgui_image *ycircle_image, *gcircle_image;

static void on_draw(rtgui_widget_t *widget);
static void next_page(rtgui_widget_t *widget);
static void priv_page(rtgui_widget_t *widget);
static void app_list_draw_pagemark(struct app_list_view *view);
static void app_list_pagemark_change(struct app_list_view *view);

static rt_bool_t app_list_view_onmouse(struct app_list_view *view,
                                       struct rtgui_event_mouse *emouse);

static rt_bool_t event_handler(struct rtgui_object *object, rtgui_event_t *event)
{
    rt_bool_t result;

    RT_ASSERT(object != RT_NULL);
    RT_ASSERT(event  != RT_NULL);

    result = RT_TRUE;
    switch (event->type)
    {
    case RTGUI_EVENT_APP_CREATE:
    case RTGUI_EVENT_APP_DESTROY:
        return apps_list_event_handler(object, event);
    case RTGUI_EVENT_WIN_ACTIVATE:
        result = rtgui_app_event_handler(object, event);
        break;
    case RTGUI_EVENT_MOUSE_BUTTON:
        result = rtgui_app_event_handler(object, event);
        break;
    default:
        /* invoke parent event handler */
        result = rtgui_app_event_handler(object, event);
        break;
    }

    return result;
}

static rt_bool_t mainmenu_event_handler(struct rtgui_object *object,
                                        rtgui_event_t *event)
{
    switch (event->type)
    {
    case RTGUI_EVENT_PAINT:
        on_draw(RTGUI_WIDGET(object));
        break;
    case RTGUI_EVENT_KBD:
        {
            struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd *)event;
            if ((ekbd->key == RTGUIK_RIGHT) && RTGUI_KBD_IS_UP(ekbd))
            {
                next_page(RTGUI_WIDGET(object));
            }
            else if ((ekbd->key == RTGUIK_LEFT) && RTGUI_KBD_IS_UP(ekbd))
            {
                priv_page(RTGUI_WIDGET(object));
            }
            else
            {
                return rtgui_win_event_handler(object, event);
            }
        }
        break;
    case RTGUI_EVENT_MOUSE_BUTTON:
        {
            struct rtgui_event_mouse *emouse;

            emouse = (struct rtgui_event_mouse *)event;
            return app_list_view_onmouse(app_list, emouse);
        }
    case RTGUI_EVENT_GESTURE:
        {
            struct rtgui_event_gesture *gestrure_event =
                (struct rtgui_event_gesture *)event;
            switch (gestrure_event->type)
            {
            case RTGUI_GESTURE_RIGHT:
                priv_page(RTGUI_WIDGET(object));
                break;
            case RTGUI_GESTURE_LEFT:
                next_page(RTGUI_WIDGET(object));
                break;
            }
        }
        break;
    case RTGUI_EVENT_WIN_ACTIVATE:
        statusbar_set_title(RT_NULL);
        statusbar_show_back_button(RT_FALSE);
        return rtgui_win_event_handler(object, event);
    default:
        return rtgui_win_event_handler(object, event);
    }
    return RT_FALSE;
}

extern void picture_app_create(void);
extern void filelist_app_create(void);
static void exec_internal_app(struct app_item *item)
{
    struct rtgui_app *app;

    if (!item)
        return;
    if (item->app_starter == RT_NULL)
    {
        rt_kprintf("can not start the app,app_starter is NULL!\n");
        return;
    }
    statusbar_set_title(item->text);
    if (item->name != RT_NULL)
    {
        app = rtgui_app_find(item->name);
        if (app == RT_NULL)
        {
            item->app_starter(item->parameter);
        }
        else
        {
            rtgui_app_activate(app);
            statusbar_show_back_button(RT_TRUE);
        }
        statusbar_show_back_button(RT_TRUE);
    }
    else
    {
        item->app_starter(item->parameter);
    }
}

#ifdef RT_USING_MODULE 
static void launch_ext_mo(void *param)
{
    rt_kprintf("try open mo %s\n", param);
    rt_module_open((char *)param);
}

void mainmenu_register_app(char *name)
{
    int i;
    char *path_buf;

    for (i = 0; i < ITEM_MAX; i++)
    {
        if (!items[i].text)
            break;
        if (!items[i].is_external)
            continue;
        if (!items[i].name)
            continue;
        if (strcmp(items[i].name, name) == 0)
            return;
    }
    if (i == ITEM_MAX)
    {
        rt_kprintf("app list is full\n");
        return;
    }

    items[i].name = rt_strdup(name);
    if (!items[i].name)
        return;

    path_buf = rtgui_malloc(DFS_PATH_MAX);
    if (!path_buf)
    {
        goto _err;
    }

    items[i].text = rt_strdup(name);
    if (!items[i].text)
        goto _err;

    rt_snprintf(path_buf, DFS_PATH_MAX, "/programs/%s/%s.png", name, name);
    path_buf[DFS_PATH_MAX - 1] = '\0';
    items[i].icon = rtgui_image_create(path_buf, RT_FALSE);
    if (!items[i].icon)
        goto _err;

    items[i].app_starter = launch_ext_mo;
    rt_snprintf(path_buf, DFS_PATH_MAX, "/programs/%s/%s.mo", name, name);
    items[i].parameter = path_buf;
    items[i].is_external = 1;

    rt_kprintf("mo %s registered\n", name);
    return;

_err:
    items[i].app_starter = RT_NULL;
    if (items[i].name)
    {
        rt_free(items[i].name);
        items[i].name = RT_NULL;
    }
    if (items[i].text)
    {
        rt_free(items[i].text);
        items[i].text = RT_NULL;
    }
    if (items[i].icon)
    {
        rtgui_image_destroy(items[i].icon);
        items[i].icon = RT_NULL;
    }
    if (path_buf)
        rtgui_free(path_buf);
}

static void mainmenu_scan_apps(void)
{
    DIR *dir;
    struct dirent *dirent;
    char *path_buf;

    path_buf = rt_malloc(DFS_PATH_MAX);
    if (!path_buf)
        return;

    dir = opendir("/programs/");
    if (dir == RT_NULL)
        goto _ret;

    do
    {
        dirent = readdir(dir);
        if (dirent == RT_NULL)
            break;

        if (strcmp(dirent->d_name, ".") == 0)
            continue;
        /* readdir is not guaranteed to return "..". So we should deal with
         * it specially. */
        if (strcmp(dirent->d_name, "..") == 0)
            continue;

        mainmenu_register_app(dirent->d_name);
    }
    while (dirent != RT_NULL);
    closedir(dir);

_ret:
    rt_free(path_buf);
}
#endif

void mainmenu_register_internal_app(char *name, char *text, rtgui_image_t *image,
                                    void (*app_starter)(void *), void *p)
{
    int i;

    for (i = 0; i < ITEM_MAX; i++)
    {
        if (!items[i].text)
            break;
        if (!items[i].name)
            continue;
        if (strcmp(items[i].name, name) == 0)
            return;
    }
    if (i == ITEM_MAX)
    {
        rt_kprintf("app list is full\n");
        return;
    }

    items[i].name = rt_strdup(name);
    if (!items[i].name)
        return;

    items[i].text = rt_strdup(name);
    if (!items[i].text)
        goto _err;

    items[i].icon = image;

    items[i].app_starter = app_starter;
    items[i].parameter = p;
    items[i].is_external = 1;

    return;

_err:
    items[i].app_starter = RT_NULL;
    if (items[i].name)
    {
        rt_free(items[i].name);
        items[i].name = RT_NULL;
    }
    if (items[i].text)
    {
        rt_free(items[i].text);
        items[i].text = RT_NULL;
    }
    if (items[i].icon)
    {
        rtgui_image_destroy(items[i].icon);
        items[i].icon = RT_NULL;
    }
}

void mainmenu_unregister_app(char *name)
{
    int i;

    for (i = 0; i < ITEM_MAX; i++)
    {
        if (!items[i].is_external)
            continue;
        if (strcmp(items[i].name, name) == 0)
        {
            rt_free(items[i].name);
            items[i].name = RT_NULL;
            rt_free(items[i].text);
            items[i].text = RT_NULL;
            rtgui_image_destroy(items[i].icon);
            items[i].icon = RT_NULL;
            items[i].app_starter = RT_NULL;
            rtgui_free(items[i].parameter);
        }
    }
}

static void next_page(rtgui_widget_t *widget)
{
    struct rtgui_dc *dc;

    if (app_list->current_page == (app_list->page_count - 1))
    {
        return;
    }
    app_list->current_page++;

    while ((app_list->current_page * 480) > app_list->listdisp_point.x)
    {
        app_list->listdisp_point.x += 40;
        app_list->bgdisp_point.x += 10;
        dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(widget));
        rtgui_dc_blit(app_list->bg_buffer, &app_list->bgdisp_point,
                      dc, &app_list->rect);
        rtgui_dc_blit(app_list->view_buffer, &app_list->listdisp_point,
                      dc, &app_list->view_rect);
        rtgui_dc_blit(app_list->pm_buffer, RT_NULL, dc, &app_list->pm_rect);
        rtgui_dc_end_drawing(dc);
    }
    app_list_pagemark_change(app_list);
    dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(widget));
    rtgui_dc_blit(app_list->pm_buffer, RT_NULL, dc, &app_list->pm_rect);
    rtgui_dc_end_drawing(dc);
}

static void priv_page(rtgui_widget_t *widget)
{
    struct rtgui_dc *dc;
    if (app_list->current_page == 0)
    {
        return;
    }
    app_list->current_page--;
    while ((app_list->current_page * 480) < app_list->listdisp_point.x)
    {

        app_list->listdisp_point.x -= 40;
        app_list->bgdisp_point.x -= 10;
        dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(widget));
        rtgui_dc_blit(app_list->bg_buffer, &app_list->bgdisp_point,
                      dc, &app_list->rect);
        rtgui_dc_blit(app_list->view_buffer, &app_list->listdisp_point,
                      dc, &app_list->view_rect);
        rtgui_dc_blit(app_list->pm_buffer, RT_NULL, dc, &app_list->pm_rect);
        rtgui_dc_end_drawing(dc);
    }
    app_list_pagemark_change(app_list);
    dc = rtgui_dc_begin_drawing(RTGUI_WIDGET(widget));
    rtgui_dc_blit(app_list->pm_buffer, RT_NULL, dc, &app_list->pm_rect);
    rtgui_dc_end_drawing(dc);
}

static rt_bool_t app_list_view_onmouse(struct app_list_view *view,
                                       struct rtgui_event_mouse *emouse)
{
    if (rtgui_rect_contains_point(&view->view_rect,
                                  emouse->x, emouse->y) == RT_EOK)
    {
        rt_uint16_t index;
        rt_uint16_t old_item;

        /* get old item */
        old_item = view->current_item;

        {
            rt_uint16_t x, y;
            rt_uint16_t item_height, item_width;


            item_width = (rtgui_rect_width(view->view_rect) - 2 * LIST_MARGIN) /
                         view->col_items;
            item_height = (rtgui_rect_height(view->view_rect) - 4) / view->row_items;
            x = emouse->x - view->view_rect.x1;
            y = emouse->y - view->view_rect.y1;

            index = (y / item_height * view->col_items) + x / item_width;

            if ((index + (view->current_page * view->page_items) <
                    view->items_count))
            {

                if (emouse->button & RTGUI_MOUSE_BUTTON_DOWN)
                {
                    view->current_item = index + (view->current_page *
                                                  view->page_items);
                    /* down event */
                    // rtgui_list_view_update_icon(view, old_item);
                }
                else
                {
                    if (view->current_item == index + (view->current_page *
                                                       view->page_items))
                    {
                        rt_kprintf("mouse up,current item:%d\n", view->current_item);
                        /* up event */
                        if (view->on_select != RT_NULL)
                        {
                            view->on_select(&view->items[view->current_item]);
                        }
                    }
                }

            }
        }


        return RT_TRUE;
    }

    return RT_FALSE;
}

static struct app_list_view *app_list_create(rtgui_rect_t *rect,
                                             struct app_item *items,
                                             rt_uint16_t item_count,
                                             rt_uint8_t row_items,
                                             rt_uint8_t col_items,
                                             rtgui_image_t *bg_image)
{
    struct app_list_view *view = RT_NULL;
    view = (struct app_list_view *)rtgui_malloc(sizeof(struct app_list_view));
    view->rect = *rect;
    view->view_rect.x1 = rect->x1;
    view->view_rect.x2 = rect->x2;
    view->view_rect.y1 = rect->y1 + 4 * LIST_MARGIN;
    view->view_rect.y2 = rect->y2 - 6 * LIST_MARGIN;
    view->col_items = col_items;
    view->row_items = row_items;
    view->current_page = 0;
    view->old_page = 0;
    view->page_items = col_items * row_items;
    view->page_count = (item_count + view->page_items - 1) / view->page_items;
    view->current_item = -1;
    view->items = items;
    view->items_count = item_count;
    if (bg_image != RT_NULL)
    {
        rtgui_rect_t bg_rect;

        RTGUI_RECT(bg_rect, 0, 0, bg_image->w, bg_image->h);
        /* create background dc buffer */
        view->bg_buffer = rtgui_dc_buffer_create(bg_image->w, bg_image->h);
        /* draw background image to buffer */
        rtgui_image_blit(bg_image, view->bg_buffer, &bg_rect);
    }
    /* create the main menu dc buffer */
    view->view_buffer =
        rtgui_dc_buffer_create_pixformat(RTGRAPHIC_PIXEL_FORMAT_ARGB888,
                                         rtgui_rect_width(view->view_rect) *
                                         view->page_count,
                                         rtgui_rect_height(view->view_rect));
    RTGUI_DC_BC(view->view_buffer) = RTGUI_ARGB(0, 0, 0, 0);

    view->pm_rect.x1 = 0;
    view->pm_rect.y1 = rect->y2 - 6 * LIST_MARGIN;
    view->pm_rect.x2 = PAGE_MARK_ITEM_WIDTH * view->page_count + PAGE_MARK_MARGIN *
                       (view->page_count - 1);
    view->pm_rect.y2 = view->pm_rect.y1 + PAGE_MARK_ITEM_HEIGHT;

    view->pm_buffer =
        rtgui_dc_buffer_create_pixformat(RTGRAPHIC_PIXEL_FORMAT_ARGB888,
                                         rtgui_rect_width(view->pm_rect) ,
                                         rtgui_rect_height(view->pm_rect));
    rtgui_rect_moveto_align(rect, &view->pm_rect, RTGUI_ALIGN_CENTER_HORIZONTAL);

    RTGUI_DC_BC(view->pm_buffer) = RTGUI_ARGB(0, 0, 0, 0);

    view->bgdisp_point.x = 0;
    view->bgdisp_point.y = 0;
    view->listdisp_point.x = 0;
    view->listdisp_point.y = 0;
    view->on_select = RT_NULL;
    return view;
}

static void app_list_pagemark_change(struct app_list_view *view)
{
    rtgui_rect_t rect = {0, 0, PAGE_MARK_ITEM_WIDTH, PAGE_MARK_ITEM_HEIGHT};
    if (view->current_page != view->old_page)
    {
        /* draw the current page mark */
        rect.x1 = view->current_page * (PAGE_MARK_ITEM_WIDTH + PAGE_MARK_MARGIN);
        rect.x2 = rect.x1 + PAGE_MARK_ITEM_WIDTH;
        RTGUI_DC_BC(view->pm_buffer) = RTGUI_ARGB(0, 0, 0, 0);
        rtgui_dc_fill_rect(view->pm_buffer, &rect);
        rtgui_image_blit(ycircle_image, view->pm_buffer, &rect);
        /* draw the old page mark */
        rect.x1 = view->old_page * (PAGE_MARK_ITEM_WIDTH + PAGE_MARK_MARGIN);
        rect.x2 = rect.x1 + PAGE_MARK_ITEM_WIDTH;
        RTGUI_DC_BC(view->pm_buffer) = RTGUI_ARGB(0, 0, 0, 0);
        rtgui_dc_fill_rect(view->pm_buffer, &rect);
        rtgui_image_blit(gcircle_image, view->pm_buffer, &rect);
        view->old_page = view->current_page;
    }
}

static void app_list_draw_pagemark(struct app_list_view *view)
{
    rt_uint8_t index;
    rtgui_rect_t rect = {0, 0, PAGE_MARK_ITEM_WIDTH, PAGE_MARK_ITEM_HEIGHT};
    for (index = 0; index < view->page_count; index++)
    {
        rect.x1 = index * (PAGE_MARK_ITEM_WIDTH + PAGE_MARK_MARGIN);
        rect.x2 = rect.x1 + PAGE_MARK_ITEM_WIDTH;
        if (index == view->current_page)
        {
            rtgui_image_blit(ycircle_image, view->pm_buffer, &rect);
        }
        else
        {
            rtgui_image_blit(gcircle_image, view->pm_buffer, &rect);
        }

    }
}

static void app_list_draw(struct app_list_view *view)
{
    struct rtgui_rect item_rect, drawing_rect;
    rt_ubase_t p, c, r, item_index; /* col and row index */
    rt_ubase_t item_width, item_height;
    rtgui_image_t *image;
    rt_uint16_t width, height;

    if (view->items_count == 0)
        return;

    width = rtgui_rect_width(view->view_rect);
    height = rtgui_rect_height(view->view_rect);
    item_index = (view->current_item / view->page_items) * view->page_items;
    item_width = (width - 2 * LIST_MARGIN) / view->col_items;
    item_height = (height - 4) / view->row_items;
    image = view->items[0].icon;
    for (p = 0; p < view->page_count; p++)
    {
        for (r = 0; r < view->row_items; r++)
        {
            for (c = 0; c < view->col_items; c++)
            {
                if (item_index < view->items_count)
                {
                    item_rect.y1 =  r * item_height;
                    item_rect.x1 =  p * width + LIST_MARGIN + c * item_width;
                    item_rect.x2 = item_rect.x1 + item_width;
                    item_rect.y2 = item_rect.y1 + item_height;
                    if (item_index == view->current_item)
                    {
                        //TO DO
                    }
                    if (view->items[item_index].icon)
                    {
                        drawing_rect.x1 = drawing_rect.y1 = 0;
                        drawing_rect.x2 = image->w;
                        drawing_rect.y2 = image->h;
                        rtgui_rect_moveto_align(&item_rect, &drawing_rect,
                                                RTGUI_ALIGN_CENTER_HORIZONTAL);
                        drawing_rect.y1 += 5;
                        drawing_rect.y2 += 5;
                        rtgui_image_blit(view->items[item_index].icon,
                                         view->view_buffer,
                                         &drawing_rect);
                    }
                    if (view->items[item_index].text)
                    {
                        item_rect.y1 = drawing_rect.y2 + LIST_MARGIN;
                        item_rect.x1 += 3;
                        item_rect.x2 -= 3;
                        rtgui_font_get_metrics(RTGUI_WIDGET_FONT(win),
                                               view->items[item_index].text,
                                               &drawing_rect);
                        rtgui_rect_moveto_align(&item_rect, &drawing_rect,
                                                RTGUI_ALIGN_CENTER_HORIZONTAL);
                        rtgui_dc_draw_text(view->view_buffer,
                                           view->items[item_index].text,
                                           &drawing_rect);
                    }
                    item_index++;
                }
                else
                    break;
            }
        }
    }
}

static void on_draw(rtgui_widget_t *widget)
{
    struct rtgui_dc *dc;
    dc = rtgui_dc_begin_drawing(widget);
    rtgui_dc_blit(app_list->bg_buffer, &app_list->bgdisp_point,
                  dc, &app_list->rect);
    rtgui_dc_blit(app_list->view_buffer, &app_list->listdisp_point,
                  dc, &app_list->view_rect);
    rtgui_dc_blit(app_list->pm_buffer, RT_NULL, dc, &app_list->pm_rect);
    rtgui_dc_end_drawing(dc);
}

static void app_mainmenu_init(void)
{
    struct rtgui_image *bg_image;
    rtgui_rect_t rect;

    /* create main window of Application Manager */
    win = rtgui_mainwin_create(RT_NULL, "mainmenu", RTGUI_WIN_STYLE_MAINWIN);
    if (win != RT_NULL)
    {
        rtgui_object_set_event_handler(RTGUI_OBJECT(win), mainmenu_event_handler);
        rtgui_widget_get_rect(RTGUI_WIDGET(win), &rect);

#ifdef RT_USING_MODULE 
        mainmenu_scan_apps();
#endif
        /* create background image */
        bg_image = rtgui_image_create("/resource/bg_image.jpg", RT_TRUE);
        if (bg_image == RT_NULL)
        {
            rt_kprintf("open \"/resource/bg_image.jpg\" failed\n");
        }
        ycircle_image = rtgui_image_create("/resource/ycircle.png", RT_TRUE);
        gcircle_image = rtgui_image_create("/resource/gcircle.png", RT_TRUE);
        app_list = app_list_create(&rect, items, ITEM_MAX, 2, 5, bg_image);
        rtgui_image_destroy(bg_image);
        app_list_draw(app_list);
        app_list_draw_pagemark(app_list);
        app_list->on_select = exec_internal_app;
        rtgui_win_show(win, RT_FALSE);
        /* set as main window */
        rtgui_app_set_main_win(rtgui_app_self(), win);
    }
}

static rtgui_win_t *tasklist_win;

void tasklist_show(void *p)
{
    rtgui_win_show(tasklist_win, RT_TRUE);
}

static void app_mainmenu_entry(void *parameter)
{
    struct rtgui_app *application;

    application = rtgui_app_create("menu");
    if (application != RT_NULL)
    {
        /* set as window manager */
        rtgui_app_set_as_wm(application);

        /* initialize status bar */
        statusbar_init();
        app_mainmenu_init();
        /* set our event handler */
        rtgui_object_set_event_handler(RTGUI_OBJECT(application),
                                       event_handler);

        tasklist_win = tasklist_win_create(RT_NULL);

        rtgui_app_run(application);
        rtgui_app_destroy(application);
    }
}

void app_mainui_init(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("menu", app_mainmenu_entry, RT_NULL, 4096, 20, 20);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
}

