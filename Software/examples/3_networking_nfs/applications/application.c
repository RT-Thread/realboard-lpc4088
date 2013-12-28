/*
   此demo用于演示网络文件系统
 */
#include <board.h>
#include <rtthread.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
#endif


#include "components.h"

extern struct netif * netif_list;
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
	
	while( !(netif_list->flags & NETIF_FLAG_UP)) 
	{	/*等待网络准备好*/
		rt_thread_delay( RT_TICK_PER_SECOND);
				
	}	
	rt_kprintf(" link ok \r\n");
    list_if();
    /* do some thing here. */
#if defined(RT_USING_DFS) && defined(RT_USING_LWIP) && defined(RT_USING_DFS_NFS)
	{
		/* NFSv3 Initialization */
		rt_kprintf("begin init NFSv3 File System ...\n");
		if (dfs_mount(RT_NULL, "/", "nfs", 0, RT_NFS_HOST_EXPORT) == 0)
			rt_kprintf("NFSv3 File System initialized!\n");
		else
			rt_kprintf("NFSv3 File System initialzation failed!\n");
	}
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
