#include <board.h>
#include <rtthread.h>
#include <rtgui/rtgui_system.h>
#include <shell.h>

#include <drv_key.h>
#include <drv_spi.h>
#include <drv_touch.h>
#include "ui_hello.h"

void rt_init_thread_entry(void* parameter)
{
	/* initialize spi driver */
		rt_hw_spi_init();
	/* initialize LCD drv for GUI */
	rtgui_lcd_init();
	/* initialize GUI system */
	rtgui_system_server_init();
	/* initialize keyboard */
	rt_hw_key_init();
	/* initialize touch */
	touch_calibration_init();

	/* say hello world in screen */
	ui_hello();
	
	finsh_system_init();
}

int rt_application_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);//
    if (tid != RT_NULL)
        rt_thread_startup(tid);
	
    return 0;
}
