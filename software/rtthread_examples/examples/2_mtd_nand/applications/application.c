/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#include <rtthread.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
#endif
#ifdef RT_USING_SPI
#include "drv_spi.h"
#include "drv_gd5f1g.h"
#endif
#include "drv_hy27uf081g.h"

void rt_init_thread_entry(void* parameter)
{
    /* initialize MTD nand device */
#ifdef RT_USING_SPI
    rt_hw_spi_init();
    gd5f1g_init();
#endif
    nand_hy27uf_hw_init();

	/* initialize finsh shell Component */
    finsh_system_init();
}

int rt_application_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
