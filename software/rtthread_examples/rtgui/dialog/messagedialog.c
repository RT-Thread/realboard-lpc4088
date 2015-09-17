#include <rtgui/rtgui_object.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_theme.h>
#include <rtgui/rtgui_app.h>

#include <rtgui/list.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/box.h>
#include <rtgui/widgets/panel.h>
#include <rtgui/widgets/textview.h>
#include <rtgui/widgets/iconbox.h>
#include "messagedialog.h"

#define MARGIN 5
static rtgui_panel_t  *button_group;
static rtgui_messagedialog_t *messagedialog;

static void messagedialog_onbutton(struct rtgui_object *object, struct rtgui_event *event)
{
    rtgui_app_exit(rtgui_app_self(), (rt_uint16_t)RTGUI_WIDGET(object)->user_data);
}
static rt_bool_t rtgui_messagedialog_event_handler(struct rtgui_object *object, struct rtgui_event *event)
{
    rtgui_win_t *win = RTGUI_WIN(object);
    switch (event->type)
    {
    case RTGUI_EVENT_SHOW:
        rtgui_win_event_handler(RTGUI_OBJECT(object), event);
        /* focus the first button */
        if (RTGUI_CONTAINER(button_group)->children.next->next != RT_NULL)
        {
            rtgui_widget_focus(rtgui_list_entry(RTGUI_CONTAINER(button_group)->children.next->next,
                                                rtgui_widget_t, sibling));
        }
        break;
    case RTGUI_EVENT_KBD:
    {
        struct rtgui_event_kbd *ekbd = (struct rtgui_event_kbd *)event;
        if (ekbd->type == RTGUI_KEYDOWN)
        {
            if (ekbd->key == RTGUIK_TAB)
            {
                if (win->focused_widget != RT_NULL && win->focused_widget->sibling.next != RT_NULL)
                {
                    rtgui_widget_focus(rtgui_list_entry(win->focused_widget->sibling.next, rtgui_widget_t, sibling));
                }
                else
                {
                    rtgui_widget_focus(rtgui_list_entry(RTGUI_CONTAINER(button_group)->children.next->next,
                                                        rtgui_widget_t, sibling));
                }
                rtgui_widget_update(RTGUI_WIDGET(win));
                return RT_TRUE;
            }
        }
        rtgui_win_event_handler(RTGUI_OBJECT(object), event);
        break;
    }
    default:
        return rtgui_win_event_handler(RTGUI_OBJECT(object), event);
    }

    return RT_FALSE;
}

rtgui_messagedialog_t *rtgui_messagedialog_create(rtgui_win_t *parent, const char *text, const char *tile, rtgui_rect_t *rect, rtgui_dialog_flag_t flag, rtgui_image_t *icon)
{
    rtgui_button_t *button;
    rtgui_box_t *box, *main_box, *btn_box;
    rtgui_panel_t *main_panel, *panel;

    messagedialog = (rtgui_messagedialog_t *)rtgui_malloc(sizeof(rtgui_messagedialog_t));
    messagedialog->win = rtgui_win_create(parent, tile,
                                          rect, RTGUI_WIN_STYLE_DEFAULT | RTGUI_WIN_STYLE_DESTROY_ON_CLOSE);
    messagedialog->flag = flag;
    rtgui_object_set_event_handler(RTGUI_OBJECT(messagedialog->win), rtgui_messagedialog_event_handler);
    box = rtgui_box_create(RTGUI_VERTICAL, MARGIN);
    rtgui_container_set_box(RTGUI_CONTAINER(messagedialog->win), box);
    /* create main panel */
    main_panel = rtgui_panel_create(RTGUI_BORDER_NONE);
    RTGUI_WIDGET_ALIGN(RTGUI_WIDGET(main_panel)) = RTGUI_ALIGN_EXPAND | RTGUI_ALIGN_STRETCH;
    main_box = rtgui_box_create(RTGUI_HORIZONTAL, MARGIN);
    rtgui_container_set_box(RTGUI_CONTAINER(main_panel), main_box);
    /* create icon box */
    if (icon != RT_NULL)
    {
        messagedialog->iconbox = rtgui_iconbox_create(icon, "", RTGUI_ICONBOX_NOTEXT);
        rtgui_widget_set_minwidth(RTGUI_WIDGET(messagedialog->iconbox), 48);
        rtgui_widget_set_minheight(RTGUI_WIDGET(messagedialog->iconbox), 48);
        RTGUI_WIDGET_ALIGN(RTGUI_WIDGET(messagedialog->iconbox)) = RTGUI_ALIGN_CENTER;
        rtgui_container_add_child(RTGUI_CONTAINER(main_panel), RTGUI_WIDGET(messagedialog->iconbox));
    }
    else
    {
        messagedialog->iconbox = RT_NULL;
    }
    /* create textview */
    messagedialog->text_view = rtgui_textview_create(text, rect);
    RTGUI_WIDGET_ALIGN(RTGUI_WIDGET(messagedialog->text_view)) = RTGUI_ALIGN_EXPAND | RTGUI_ALIGN_STRETCH;
    rtgui_container_add_child(RTGUI_CONTAINER(main_panel), RTGUI_WIDGET(messagedialog->text_view));

    button_group = rtgui_panel_create(RTGUI_BORDER_NONE);
    rtgui_widget_set_minheight(RTGUI_WIDGET(button_group), 40);
    RTGUI_WIDGET_ALIGN(button_group) = RTGUI_ALIGN_EXPAND;
    btn_box = rtgui_box_create(RTGUI_HORIZONTAL, MARGIN);
    rtgui_container_set_box(RTGUI_CONTAINER(button_group), btn_box);
    panel = rtgui_panel_create(RTGUI_BORDER_NONE);
    RTGUI_WIDGET_ALIGN(RTGUI_WIDGET(panel)) = RTGUI_ALIGN_EXPAND | RTGUI_ALIGN_STRETCH;
    rtgui_container_add_child(RTGUI_CONTAINER(button_group), RTGUI_WIDGET(panel));
    if (flag & RTGUI_MB_FLAG_OK)
    {
        button = rtgui_button_create("Ok");
        button->parent.parent.user_data = RTGUI_DR_OK;
        rtgui_widget_set_minwidth(RTGUI_WIDGET(button), 60);
        rtgui_widget_set_minheight(RTGUI_WIDGET(button), 24);
        RTGUI_WIDGET_ALIGN(button) = RTGUI_ALIGN_RIGHT;
        rtgui_button_set_onbutton(button, messagedialog_onbutton);
        rtgui_container_add_child(RTGUI_CONTAINER(button_group), RTGUI_WIDGET(button));
    }
    if (flag & RTGUI_MB_FLAG_YES)
    {
        button = rtgui_button_create("Yes");
        button->parent.parent.user_data = RTGUI_DR_YES;
        rtgui_widget_set_minwidth(RTGUI_WIDGET(button), 60);
        rtgui_widget_set_minheight(RTGUI_WIDGET(button), 24);
        RTGUI_WIDGET_ALIGN(button) = RTGUI_ALIGN_RIGHT;
        rtgui_button_set_onbutton(button, messagedialog_onbutton);
        rtgui_container_add_child(RTGUI_CONTAINER(button_group), RTGUI_WIDGET(button));
    }

    if (flag & RTGUI_MB_FLAG_RETRY)
    {
        button = rtgui_button_create("Retry");
        button->parent.parent.user_data = RTGUI_DR_RETRY;
        rtgui_widget_set_minwidth(RTGUI_WIDGET(button), 60);
        rtgui_widget_set_minheight(RTGUI_WIDGET(button), 24);
        RTGUI_WIDGET_ALIGN(button) = RTGUI_ALIGN_RIGHT;
        rtgui_button_set_onbutton(button, messagedialog_onbutton);
        rtgui_container_add_child(RTGUI_CONTAINER(button_group), RTGUI_WIDGET(button));
    }
    if (flag & RTGUI_MB_FLAG_NO)
    {
        button = rtgui_button_create("No");
        button->parent.parent.user_data = RTGUI_DR_NO;
        rtgui_widget_set_minwidth(RTGUI_WIDGET(button), 60);
        rtgui_widget_set_minheight(RTGUI_WIDGET(button), 24);
        RTGUI_WIDGET_ALIGN(button) = RTGUI_ALIGN_RIGHT;
        rtgui_button_set_onbutton(button, messagedialog_onbutton);
        rtgui_container_add_child(RTGUI_CONTAINER(button_group), RTGUI_WIDGET(button));
    }
    if (flag & RTGUI_MB_FLAG_CANCEL)
    {
        button = rtgui_button_create("Cancel");
        button->parent.parent.user_data = RTGUI_DR_CANCEL;
        rtgui_widget_set_minwidth(RTGUI_WIDGET(button), 60);
        rtgui_widget_set_minheight(RTGUI_WIDGET(button), 24);
        RTGUI_WIDGET_ALIGN(button) = RTGUI_ALIGN_RIGHT;
        rtgui_button_set_onbutton(button, messagedialog_onbutton);
        rtgui_container_add_child(RTGUI_CONTAINER(button_group), RTGUI_WIDGET(button));
    }
    /* add child to win */
    rtgui_container_add_child(RTGUI_CONTAINER(messagedialog->win), RTGUI_WIDGET(main_panel));
    rtgui_container_add_child(RTGUI_CONTAINER(messagedialog->win), RTGUI_WIDGET(button_group));
    /* box layout */
    rtgui_container_layout(RTGUI_CONTAINER(messagedialog->win));

    return messagedialog;
}

rtgui_dialog_result_t rtgui_messagedialog_show(rtgui_messagedialog_t *msgbox)
{
    rtgui_dialog_result_t ret;
    ret = (rtgui_dialog_result_t)rtgui_win_show(msgbox->win, RT_TRUE);
    return ret;
}
void rtgui_messagedialog_destroy(rtgui_messagedialog_t *msgbox)
{
    rtgui_free(msgbox);
}
