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

//#include <log_trace.h>
//#include "nftl.h"
//#include "drv_nand.h"

/* thread phase init */
void rt_init_thread_entry(void *parameter)
{
// #ifdef RT_USING_I2C
//     rt_i2c_core_init();
//     rt_hw_i2c_init();
// #endif

    rt_hw_spi_init();
    rt_system_module_init();
    /* Filesystem Initialization */
#ifdef RT_USING_DFS
    {
        extern rt_err_t mci_hw_init(const char *device_name);
        /* initilize sd card */
        mci_hw_init("sd0");
        /* init the device filesystem */
        dfs_init();

        /* init the elm FAT filesystam*/
        elm_init();
        /* mount sd card fat partition 1 as root directory */
        if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
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
        /* re-init device driver */
        rt_device_init_all();

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
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t led_stack[ 512 ];
static struct rt_thread led_thread;
static void led_thread_entry(void* parameter)
{
    unsigned int count=0;
    rt_device_t led_dev=rt_device_find("led");
    rt_uint8_t led_value=0;
    while (1)
    {
        /* led1 on */
#ifndef RT_USING_FINSH
        rt_kprintf("led on, count : %d\r\n",count);
#endif
        count++;
        led_value=1;
        led_dev->write(led_dev,count%4,&led_value,1);
        rt_thread_delay( RT_TICK_PER_SECOND/2 ); /* sleep 0.5 second and switch to other thread */

        /* led1 off */
#ifndef RT_USING_FINSH
        rt_kprintf("led off\r\n");
#endif
        led_value=0;
        led_dev->write(led_dev,count%4,&led_value,1);
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}
int rt_application_init(void)
{
    rt_thread_t tid;
    rt_err_t  result;
    tid = rt_thread_create("init",
                           rt_init_thread_entry, RT_NULL,
                           2048, RT_THREAD_PRIORITY_MAX / 3, 20);
    if (tid != RT_NULL) rt_thread_startup(tid);

    /* init led thread */
//    result = rt_thread_init(&led_thread,
//                            "led",
//                            led_thread_entry,
//                            RT_NULL,
//                            (rt_uint8_t*)&led_stack[0],
//                            sizeof(led_stack),
//                            20,
//                            5);
//    if (result == RT_EOK)
//    {
//        rt_thread_startup(&led_thread);
//    }
    return 0;
}

typedef int (*func_t)(void);
int mtest(void)
{
    func_t ptr;

    ptr = (func_t)0xA0000000;
    ptr();

    return 0;
}
FINSH_FUNCTION_EXPORT(mtest, mpu test);
