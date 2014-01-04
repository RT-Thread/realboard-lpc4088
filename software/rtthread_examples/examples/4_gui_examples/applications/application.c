/*
   此demo用于演示rtgui examples，系统中文字库等文件需要放在tf卡的根目录
   如果触摸不准，可以删除tf卡中的setup.ini文件后重新进行触摸校准
 */
#include <board.h>
#include <rtthread.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
#endif

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#endif

#include <rtgui/calibration.h>
#include "components.h"

#include "drv_sd.h"
#include "drv_lcd.h"
#include "drv_touch.h"
#include "setup.h"

extern void rt_hw_key_init(void);
extern rt_bool_t cali_setup(void);
extern void cali_store(struct calibration_data *data);


void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
	/* initialization RT-Thread Components */
	rt_components_init();
#endif
	
	rt_hw_spi_init();

	mci_hw_init("sd0");

	
	/* Filesystem Initialization */
#ifdef RT_USING_DFS
	/* mount sd card fat partition 1 as root directory */
	if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
	{
		rt_kprintf("File System initialized!\n");
		
		#if (RT_DFS_ELM_USE_LFN != 0) && (defined RT_DFS_ELM_CODE_PAGE_FILE)
        {
                extern void ff_convert_init(void);
                ff_convert_init();
        }
		#endif
		
	}
	else 
	{	
		rt_kprintf("File System initialzation failed!\n");
	}	
#endif


#ifdef RT_USING_RTGUI
	{
		rt_device_t device;
		struct rt_device_rect_info info;
		extern void application_init(void);

		rt_hw_lcd_init();
		rt_device_init_all();
		
		device = rt_device_find("lcd");
		/* re-set graphic device */
		rtgui_graphic_set_device(device);
		
		/* 在rt_components_init时由于文件系统尚未挂载，
		中文字库还没准备好，所以待中文字库准备好后，再来一次rtgui初始化*/
		rtgui_system_server_init();
		
		rt_hw_key_init();
		rtgui_touch_hw_init("spi10");
		
		calibration_set_restore(cali_setup);
	    calibration_set_after(cali_store);
		calibration_init();		
		
		application_init();
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
