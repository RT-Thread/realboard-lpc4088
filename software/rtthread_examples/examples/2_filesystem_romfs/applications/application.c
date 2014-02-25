#include <rtthread.h>
#include <dfs_init.h>
#include <dfs_fs.h>
#include <dfs_romfs.h>
#include <shell.h>

void rt_init_thread_entry(void* parameter)
{
	/* initialize the device filesystem */
	dfs_init();

	dfs_romfs_init();
	
    /* mount rom file system */
    if (dfs_mount(RT_NULL, "/", "rom", 0, DFS_ROMFS_ROOT) == 0)
    {
        rt_kprintf("ROM file system initializated!\n");
    }

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

