#ifndef __APPMGR_H__
#define __APPMGR_H__

#include <rtgui/rtgui_app.h>
#include <rtgui/image.h>
#include <rtgui/widgets/window.h>
#include <rtgui/widgets/label.h>
#include <rtgui/widgets/box.h>
#include <rtgui/widgets/panel.h>

struct app_item
{
    char *name;
    char *text;
    rtgui_image_t *icon;
    void (*app_starter)(void *);
    void *parameter;
    int is_external;
};

#define ITEM_MAX           32

void app_mainui_init(void);

void mainmenu_register_internal_app(char *name, char *text, rtgui_image_t *image,
                                    void (*app_starter)(void *), void *p);
void mainmenu_unregister_app(char *name);

void tasklist_show(void *p);

#ifdef RT_USING_MODULE 
void mainmenu_register_app(char *name);
#endif

#endif
