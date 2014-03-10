#include <rtthread.h>
#include <dfs_fs.h>

#ifdef RT_USING_NFTL
#include "nftl.h"
#endif

int mnt_init(void)
{
	nftl_attach("nand0");
	if (dfs_mount("nand0", "/", "elm", 0, 0) == 0)
	{
		rt_kprintf("Mount FatFs file system to root, Done!\n");
	}
	else
	{
		rt_kprintf("Mount FatFs file system failed.\n");
	}

	/* initilize sd card */
	mci_hw_init("sd0");
	/* mount sd card fat partition 1 as root directory */
	if (dfs_mount("sd0", "/SD", "elm", 0, 0) == 0)
		rt_kprintf("File System initialized!\n");
	else
		rt_kprintf("File System init failed!\n");
}
INIT_ENV_EXPORT(mnt_init);
