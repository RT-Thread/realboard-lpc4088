#include <rtthread.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_app.h>
#include <dfs_posix.h>
#include "appmgr.h"
#include "statusbar.h"


void realtouch_ui_init(void)
{
    app_mgr_init();
    rt_thread_delay(10);
}
