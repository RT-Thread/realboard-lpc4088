#include <rtthread.h>
#include <dfs_fs.h>

#include "drv_sd.h"

int mnt_init(void)
{
	/* initilize sd card */
	mci_hw_init("sd0");
	/* mount sd card fat partition 1 as root directory */
	if (dfs_mount("sd0", "/", "elm", 0, 0) == 0)
		rt_kprintf("File System initialized!\n");
	else
		rt_kprintf("File System init failed!\n");
	return 0;
}
INIT_ENV_EXPORT(mnt_init);
