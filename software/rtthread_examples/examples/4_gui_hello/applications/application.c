#include <board.h>
#include <rtthread.h>
#ifdef RT_USING_COMPONENTS_INIT
#include <components.h>
#endif

#include "drv_lcd.h"
#include "ui_hello.h"

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
	/* initialization RT-Thread Components */
	rt_components_init();
#endif

#ifdef RT_USING_RTGUI
	{
		rt_device_t device;

		rt_hw_lcd_init();

		device = rt_device_find("lcd");
		/* re-set graphic device */
		rtgui_graphic_set_device(device);

		/* initialize rtGUI server */
		rtgui_system_server_init();

		/* initialize keyboard and touch */
		rt_hw_key_init();
		rtgui_touch_hw_init("spi10");

		/* say hello world in screen */
		ui_hello();
	}
#endif

    /* do some thing here. */
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
