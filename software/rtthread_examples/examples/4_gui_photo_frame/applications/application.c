/*
   此demo类似数码相框，请将HDC或BMP格式图片放在TF卡更目录进行显示，K2键用于切换图片
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

extern int picture_init(void);

void rt_init_thread_entry(void* parameter)
{
	/* initialize spi driver */
	rt_hw_spi_init();
	
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
  /* initialize lcd driver */
	rtgui_lcd_init();
	/* initialize GUI system */
	rtgui_system_server_init();
	/* initialize keyboard */
	rt_hw_key_init();
	/* initialize touch and calibration */
	touch_calibration_init();

  /* picture viwer initializtion */
	picture_init();

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
