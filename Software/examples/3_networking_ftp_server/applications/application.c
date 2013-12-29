/*
   此demo用于演示ftp服务器
 */
#include <board.h>
#include <rtthread.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
#endif


#include "components.h"

extern void lpc_emac_hw_init(void);

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_LWIP
	/* initialize eth interface */
	lpc_emac_hw_init();
#endif
	
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
		
			#if (RT_DFS_ELM_USE_LFN != 0) && (defined RT_DFS_ELM_CODE_PAGE_FILE)
			{
                extern void ff_convert_init(void);
                ff_convert_init();
			}
			#endif
		
		}
        else
            rt_kprintf("File System initialzation failed!\n");
#endif
    }
#endif
	
    /* do some thing here. */
#if defined(RT_USING_DFS) && defined(RT_USING_LWIP)
	/* start ftp server */
	rt_kprintf("ftp server begin...\n");
	ftpd_start();
	rt_kprintf("ftp server started!!\n");
#endif
    
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
