#ifndef __APPS_LIST_H__
#define __APPS_LIST_H__

#include <rtgui/event.h>
#include <rtgui/rtgui_object.h>
#include <rtgui/widgets/panel.h>

rt_bool_t apps_list_event_handler(struct rtgui_object *object, struct rtgui_event *event);
rtgui_win_t *tasklist_win_create(rtgui_win_t *parent);
struct rtgui_app *rtgui_app_find(const char *app_name);

#endif
