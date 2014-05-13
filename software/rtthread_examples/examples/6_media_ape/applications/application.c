/*
  ��demo������ʾelmfs������tf�����ص�ϵͳ��Ŀ¼
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

#include "components.h"



void rt_init_thread_entry(void* parameter)
{
	
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif
	
	{
		extern rt_err_t mci_hw_init(const char *device_name);
		mci_hw_init("sd0");
	}	

    /* Filesystem Initialization */
#ifdef RT_USING_DFS
    {
#ifdef RT_USING_DFS_ELMFAT
        /* mount sd card fat partition 1 as root directory */
        if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
        {
            rt_kprintf("File System initialized!\n");
        }
        else
            rt_kprintf("File System initialzation failed!\n");
#endif
    }
#endif
	rt_hw_i2c_init();
	codec_hw_init("i2c1");
//		while(1){
//		wav("/test.wav");
//		}
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
