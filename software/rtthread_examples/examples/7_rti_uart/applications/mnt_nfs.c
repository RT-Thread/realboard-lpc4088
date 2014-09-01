#include <components.h>

int mountnfs(const char * path)
{
    const char * mountpath = "/";
    if (path != NULL) mountpath = path;

    rt_kprintf("mount nfs to %s...", mountpath);
    if (dfs_mount(RT_NULL, mountpath, "nfs", 0, RT_NFS_HOST_EXPORT) == 0)
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
    if (argc == 1)
    {
        rt_kprintf("%s path\n", argv[0]);
        return 0;
    }

    return mountnfs(argv[1]);
}
FINSH_FUNCTION_EXPORT_ALIAS(cmd_mountnfs, __cmd_mountnfs, mount NFS file system.);
