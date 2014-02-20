#include <rtthread.h>

#ifdef RT_USING_DFS
#include <dfs_fs.h>
#include <dfs_ramfs.h>
#endif

#include "components.h"

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif
	
    /* Filesystem Initialization */
#ifdef RT_USING_DFS
#ifdef RT_USING_DFS_ELMFAT
    {
        rt_uint8_t *ramdisk;
        #define RAMDISK_SZ      (1024 * 1024 * 1)

        /* mount sd card fat partition 1 as root directory */
        if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
        {
            rt_kprintf("File System initialized!\n");
        }
        else
            rt_kprintf("File System initialzation failed!\n");

        ramdisk = (rt_uint8_t*) rt_malloc(RAMDISK_SZ);  /* malloc 1MB for ramdisk */
        if (ramdisk != RT_NULL)
        {
            struct dfs_ramfs* ramfs;

            ramfs = dfs_ramfs_create(ramdisk, RAMDISK_SZ);
            if (ramfs != RT_NULL)
            {
                /* mount ram file system */
                if (dfs_mount(RT_NULL, "/ramdisk", "ramfs", 0, ramfs) == 0)
                {
                    rt_kprintf("RAM file system initialized!\n");
                }
            }
        }
    }
#endif
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
