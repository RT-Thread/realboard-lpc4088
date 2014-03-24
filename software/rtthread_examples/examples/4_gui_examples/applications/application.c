/*
   此demo用于演示rtgui examples，系统中文字库等文件需要放在tf卡的根目录
   如果触摸不准，可以删除tf卡中的setup.ini文件后重新进行触摸校准
 */
#include <board.h>
#include <rtthread.h>
#include <rtgui/rtgui_system.h>
#include <dfs_fs.h>
#include <dfs_init.h>
#include <dfs_elm.h>
#include <shell.h>

#include <drv_key.h>
#include <drv_spi.h>
#include <drv_touch.h>
#include <drv_sd.h>

extern void application_init(void);
void rt_init_thread_entry(void* parameter)
{

	/* Filesystem Initialization */
	mci_hw_init("sd0");
	/* initialize the device file system */
	dfs_init();
	/* initialize the elm chan FatFS file system*/
	elm_init();

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

	/* initialize keyboard */
	rt_hw_key_init();
	/* initialize touch */
	rt_hw_spi_init();
	/* initialize LCD drv for GUI */
	rtgui_lcd_init();

	/* initialize GUI system */
	 rtgui_system_server_init();
    /* initialize touch */
    touch_calibration_init();

    /* GUI examples initializtion */
	 application_init();
	
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
