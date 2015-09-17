#ifndef __STATUS_BAR_H__
#define __STATUS_BAR_H__

#include <rtthread.h>
#include <rtgui/rtgui_app.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/box.h>
#include <rtgui/widgets/panel.h>

void statusbar_init(void);
void statusbar_show_back_button(rt_bool_t enable);
void statusbar_set_title(char *title);
#endif
