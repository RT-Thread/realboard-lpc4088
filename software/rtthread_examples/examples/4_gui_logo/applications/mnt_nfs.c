#include <rtthread.h>
#include <components.h>

int mountnfs(const char * path, const char* export_dir)
{
    const char * mountpath = "/";
    if (path != RT_NULL) mountpath = path;

    rt_kprintf("mount nfs:%s to %s...", export_dir, mountpath);
    if (dfs_mount(RT_NULL, mountpath, "nfs", 0, export_dir) == 0)
    {
        rt_kprintf("[ok]\n");
        return 0;
    }
    else
    {
        rt_kprintf("[failed!]\n");
        return -1;
    }
}
FINSH_FUNCTION_EXPORT(mountnfs, mount nfs);

/* mountnfs, exported to msh */
int cmd_mountnfs(int argc, char** argv)
{
    if (argc < 3)
    {
        rt_kprintf("Usage:%s remote_export path\n", argv[0]);
        return 0;
    }

    return mountnfs(argv[1], argv[2]);
}
FINSH_FUNCTION_EXPORT_ALIAS(cmd_mountnfs, __cmd_mountnfs, mount NFS file system.);
