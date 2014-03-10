/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-05-02     Aozima       add led function
 */

#include <rtthread.h>

#include <board.h>
#include <components.h>

int mountnfs(const char * path)
{
    const char * mountpath = "/";
    if (path != NULL) mountpath = path;

    rt_kprintf("mount nfs to %s...", mountpath);
    if (dfs_mount(RT_NULL, mountpath, "nfs", 0, RT_NFS_HOST_EXPORT) == 0)
    {
        rt_kprintf("[ok]\n");
        return 0;
    }
    else
    {
        rt_kprintf("[failed!]\n");
        return -1;
    }
}
FINSH_FUNCTION_EXPORT(mountnfs, mount nfs);

/* mountnfs, exported to msh */
int cmd_mountnfs(int argc, char** argv)
{
    if (argc == 1)
    {
        rt_kprintf("%s path\n", argv[0]);
        return 0;
    }

    return mountnfs(argv[1]);
}
FINSH_FUNCTION_EXPORT_ALIAS(cmd_mountnfs, __cmd_mountnfs, mount NFS file system.);

/* thread phase init */
void rt_init_thread_entry(void *parameter)
{
	rt_components_init();

	/* startup rtgui realtouch ui */
	realtouch_ui_init();
}

int rt_application_init(void)
{
    rt_thread_t tid;
    tid = rt_thread_create("init",
                           rt_init_thread_entry, RT_NULL,
                           2048, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);

    return 0;
}
