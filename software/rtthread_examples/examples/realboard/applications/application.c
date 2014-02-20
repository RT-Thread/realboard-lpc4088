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
#ifdef RT_USING_LWIP
#include <drv_emac.h>
#endif
#ifdef RT_USING_RTGUI
#include "drv_lcd.h"
#endif
#ifdef RT_USING_I2C
#include "drivers/i2c.h"
#endif
#ifdef RT_USING_SPI
#include "drv_spi.h"
#include "drivers/spi.h"
#endif

#ifdef RT_USING_NFTL
#include "nftl.h"
#endif

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
    rt_hw_spi_init();
    rt_system_module_init();

    /* Filesystem Initialization */
#ifdef RT_USING_DFS
    {
        extern rt_err_t mci_hw_init(const char *device_name);

        /* init the device filesystem */
        dfs_init();

        /* init the elm FAT filesystam*/
        elm_init();
#ifdef RT_USING_NFTL
		nand_hy27uf_hw_init();
		nftl_attach("nand0");
		if (dfs_mount("nand0", "/", "elm", 0, 0) == 0)
		{
			rt_kprintf("Mount FatFs file system to root, Done!\n");
		}
		else
		{
			rt_kprintf("Mount FatFs file system failed.\n");
		}
#endif

        /* initilize sd card */
        mci_hw_init("sd0");
        /* mount sd card fat partition 1 as root directory */
        if (dfs_mount("sd0", "/SD", "elm", 0, 0) == 0)
            rt_kprintf("File System initialized!\n");
        else
            rt_kprintf("File System init failed!\n");
    }
#endif
    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    {
        extern void lwip_sys_init(void);

        eth_system_device_init();

        /* register ethernetif device */
        lpc_emac_hw_init();
        /* init all device */
        rt_device_init_all();

        /* init lwip system */
        lwip_sys_init();
        rt_kprintf("TCP/IP initialized!\n");
    }
#endif

#ifdef RT_USING_RTGUI
    {
        extern void realtouch_ui_init(void);
        extern void rt_hw_key_init(void);
        rt_device_t lcd;
        /* init lcd */
        rt_hw_lcd_init();

        /* find lcd device */
        lcd = rt_device_find("lcd");
        if (lcd != RT_NULL)
        {
            /* set lcd device as rtgui graphic driver */
            rtgui_graphic_set_device(lcd);

            /* init rtgui system server */
            rtgui_system_server_init();
            rt_thread_delay(5);
            rt_hw_key_init();
            rtgui_touch_hw_init("spi10");
            /* startup rtgui realtouch ui */
            realtouch_ui_init();
        }
    }
#endif

#ifdef RT_USING_FINSH
    /* initialize finsh */
    finsh_system_init();
#endif
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
