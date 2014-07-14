/*
   此demo用于演示网络部分
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

	
#ifdef RT_USING_COMPONENTS_INIT
	/* initialization RT-Thread Components */
	rt_components_init();
#endif
#ifdef RT_USING_LWIP
	/* initialize eth interface */
	rt_hw_wifi_init("spi01");
#endif
//netio_init();
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
