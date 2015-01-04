#include "mainmenu.h"
#include <rtgui/widgets/notebook.h>
#include <rtgui/widgets/list_view.h>
#include <rtgui/calibration.h>
#include <stdio.h>
#include <time.h>

void filelist_app_create(void*);
void picture_app_create(void *param);
void _upgrade_fw_dia(void*);

#define APPS_RESOURCE_PATH "/resource/apps"
void internal_app_init(void)
{
    mainmenu_register_internal_app("picture", "ͼƬ",
                                   rtgui_image_create(APPS_RESOURCE_PATH"/picture.png", RT_FALSE),
                                   picture_app_create, RT_NULL);
}

