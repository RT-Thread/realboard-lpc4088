#include <rtthread.h>
#include <rtdevice.h> /* For completion. */

#include <wget.h>
#include <dfs_fs.h>
#include <dfs_posix.h>
#include <json.h>

#define startup_log rt_kprintf

static rt_bool_t filesystem_checking(void)
{
    DIR *r;

    startup_log("checking file system...\n");
    r = opendir("/");
    if (r == RT_NULL)
        return RT_FALSE;

    closedir(r);
    return RT_TRUE;
}

static rt_bool_t filesystem_format(void)
{
    int rc;

    startup_log("try to format disk...");
    rc = dfs_mkfs("elm", "nand0");
    if (rc != 0)
        return RT_FALSE;

    if (dfs_mount("nand0", "/", "elm", 0, 0) == 0)
    {
        startup_log("Mount Root done");
        return RT_TRUE;
    }
    else
    {
        startup_log("Mount Root failed");
        return RT_FALSE;
    }
}

static const char* _resource_paths[] =
{
    "/bin"               ,

    "/SD"                ,

    "/picture"           ,

    "/programs"          ,
    "/programs/calc"     ,

    "/resource"          ,
    "/resource/setting"  ,
    "/resource/apps"     ,
    "/resource/statusbar",

    RT_NULL              ,
};

struct resource_item
{
    const char *URL;
    const char *local_URL;
    const char *local_fn;
};
#define URL_ROOT "http://www.rt-thread.com/download/realboard/rootfs"
#define URL_LOC  "/SD/rootfs"
static const struct resource_item _resource_items[] =
{
    {URL_ROOT"/resource/uni2gbk.tbl"            , URL_LOC"/resource/uni2gbk.tbl"            , "/resource/uni2gbk.tbl"            },
    {URL_ROOT"/resource/gbk2uni.tbl"            , URL_LOC"/resource/gbk2uni.tbl"            , "/resource/gbk2uni.tbl"            },
    {URL_ROOT"/resource/hzk12.fnt"              , URL_LOC"/resource/hzk12.fnt"              , "/resource/hzk12.fnt"              },
    {URL_ROOT"/resource/hzk16.fnt"              , URL_LOC"/resource/hzk16.fnt"              , "/resource/hzk16.fnt"              },
    {URL_ROOT"/resource/bg_image.jpg"           , URL_LOC"/resource/bg_image.jpg"           , "/resource/bg_image.jpg"           },
    {URL_ROOT"/resource/ycircle.png"            , URL_LOC"/resource/ycircle.png"            , "/resource/ycircle.png"            },
    {URL_ROOT"/resource/gcircle.png"            , URL_LOC"/resource/gcircle.png"            , "/resource/gcircle.png"            },

    {URL_ROOT"/resource/apps/alarm.png"         , URL_LOC"/resource/apps/alarm.png"         , "/resource/apps/alarm.png"         },
    {URL_ROOT"/resource/apps/filelist.png"      , URL_LOC"/resource/apps/filelist.png"      , "/resource/apps/filelist.png"      },
    {URL_ROOT"/resource/apps/leds.png"          , URL_LOC"/resource/apps/leds.png"          , "/resource/apps/leds.png"          },
    {URL_ROOT"/resource/apps/music.png"         , URL_LOC"/resource/apps/music.png"         , "/resource/apps/music.png"         },
    {URL_ROOT"/resource/apps/network.png"       , URL_LOC"/resource/apps/network.png"       , "/resource/apps/network.png"       },
    {URL_ROOT"/resource/apps/note.png"          , URL_LOC"/resource/apps/note.png"          , "/resource/apps/note.png"          },
    {URL_ROOT"/resource/apps/picture.png"       , URL_LOC"/resource/apps/picture.png"       , "/resource/apps/picture.png"       },
    {URL_ROOT"/resource/apps/setting.png"       , URL_LOC"/resource/apps/setting.png"       , "/resource/apps/setting.png"       },
    {URL_ROOT"/resource/apps/snake.png"         , URL_LOC"/resource/apps/snake.png"         , "/resource/apps/snake.png"         },
    {URL_ROOT"/resource/apps/status.png"        , URL_LOC"/resource/apps/status.png"        , "/resource/apps/status.png"        },
    {URL_ROOT"/resource/apps/terminal.png"      , URL_LOC"/resource/apps/terminal.png"      , "/resource/apps/terminal.png"      },
    {URL_ROOT"/resource/apps/tetris.png"        , URL_LOC"/resource/apps/tetris.png"        , "/resource/apps/tetris.png"        },
    {URL_ROOT"/resource/apps/time.png"          , URL_LOC"/resource/apps/time.png"          , "/resource/apps/time.png"          },
    {URL_ROOT"/resource/apps/update.png"        , URL_LOC"/resource/apps/update.png"        , "/resource/apps/update.png"        },

    {URL_ROOT"/resource/statusbar/logo.hdc"     , URL_LOC"/resource/statusbar/logo.hdc"     , "/resource/statusbar/logo.hdc"     },
    {URL_ROOT"/resource/statusbar/back.hdc"     , URL_LOC"/resource/statusbar/back.hdc"     , "/resource/statusbar/back.hdc"     },
    {URL_ROOT"/resource/statusbar/linkup.hdc"   , URL_LOC"/resource/statusbar/linkup.hdc"   , "/resource/statusbar/linkup.hdc"   },
    {URL_ROOT"/resource/statusbar/linkdown.hdc" , URL_LOC"/resource/statusbar/linkdown.hdc" , "/resource/statusbar/linkdown.hdc" },

    {URL_ROOT"/picture/0.jpg"                   , URL_LOC"/picture/0.jpg"                   , "/picture/0.jpg"                   },
    {URL_ROOT"/picture/1.jpg"                   , URL_LOC"/picture/1.jpg"                   , "/picture/1.jpg"                   },
    {URL_ROOT"/picture/2.jpg"                   , URL_LOC"/picture/2.jpg"                   , "/picture/2.jpg"                   },
    {URL_ROOT"/picture/3.jpg"                   , URL_LOC"/picture/3.jpg"                   , "/picture/3.jpg"                   },
    {URL_ROOT"/picture/4.jpg"                   , URL_LOC"/picture/4.jpg"                   , "/picture/4.jpg"                   },
    {URL_ROOT"/picture/5.jpg"                   , URL_LOC"/picture/5.jpg"                   , "/picture/5.jpg"                   },
    {URL_ROOT"/picture/6.jpg"                   , URL_LOC"/picture/6.jpg"                   , "/picture/6.jpg"                   },
    {URL_ROOT"/picture/7.jpg"                   , URL_LOC"/picture/7.jpg"                   , "/picture/7.jpg"                   },
    {URL_ROOT"/picture/8.jpg"                   , URL_LOC"/picture/8.jpg"                   , "/picture/8.jpg"                   },
    {URL_ROOT"/picture/9.jpg"                   , URL_LOC"/picture/9.jpg"                   , "/picture/9.jpg"                   },

    {URL_ROOT"/programs/calc/calc.png"          , URL_LOC"/programs/calc/calc.png"          , "/programs/calc/calc.png"          },
    {URL_ROOT"/programs/calc/calc.mo"           , URL_LOC"/programs/calc/calc.mo"           , "/programs/calc/calc.mo"           },

    {RT_NULL, RT_NULL},
};

#define BUF_SZ  4096
static rt_err_t copyfile(const char *src,  const char *dst)
{
    struct dfs_fd src_fd, dst_fd;
    rt_uint8_t *block_ptr;
    rt_int32_t read_bytes;

    block_ptr = rt_malloc(BUF_SZ);
    if (block_ptr == RT_NULL)
    {
        return -RT_ENOMEM;
    }

    if (dfs_file_open(&src_fd, src, DFS_O_RDONLY) < 0)
    {
        rt_free(block_ptr);
        return -RT_ERROR;
    }
    if (dfs_file_open(&dst_fd, dst, DFS_O_WRONLY | DFS_O_CREAT) < 0)
    {
        rt_free(block_ptr);
        dfs_file_close(&src_fd);
        return -RT_ERROR;
    }

    do {
        read_bytes = dfs_file_read(&src_fd, block_ptr, BUF_SZ);
        if (read_bytes > 0)
        {
            dfs_file_write(&dst_fd, block_ptr, read_bytes);
        }
    } while (read_bytes > 0);

    dfs_file_close(&src_fd);
    dfs_file_close(&dst_fd);
    rt_free(block_ptr);

    return RT_EOK;
}

static void resource_checking(void)
{
    rt_uint32_t index;
    char *msg = RT_NULL;

    startup_log("checking resource...");

    index = 0;
    while (1)
    {
        DIR *dir;
        const char *path;

        path = _resource_paths[index];
        if (path == RT_NULL)
            break;

        dir = opendir(path);
        if (dir == RT_NULL)
        {
            mkdir(path, 0);
            /* Handle /SD specially. We want the SD card to be mounted asap. */
            if (strcmp(path, "/SD") == 0)
            {
                if (dfs_mount("sd0", "/SD", "elm", 0, 0) == 0)
                    rt_kprintf("/SD mounted\n");
                else
                    rt_kprintf("/SD mount failed\n");
            }
        }
        else
            closedir(dir);

        index++;
    }

    index = 0;
    while (1)
    {
        int fd;
        const struct resource_item *ritem;

        ritem = &_resource_items[index];
        if (ritem->URL == RT_NULL)
            break;

        fd = open(ritem->local_fn, O_RDONLY, 0);
        if (fd < 0)
        {
            if (!msg)
            {
                msg = (char *)rt_malloc(128);
                rt_memset(msg, 0x0, 128);
            }

            rt_snprintf(msg, 128 - 1, "getting %s...", ritem->local_fn);
            startup_log(msg);
            if (copyfile(ritem->local_URL, ritem->local_fn) != RT_EOK)
                http_down(ritem->local_fn, ritem->URL);
        }
        else
            close(fd);

        index ++;
    }

    if (msg)
        rt_free(msg);

    startup_log("checking done!");
}

void network_checking(void)
{
    int fd;
    
	http_down("/files.json", "http://lab.rt-thread.org/realboard/lpc4088/files.json");
    fd = open("/files.json", O_RDONLY, 0);
    if (fd >= 0)
    {
        int length;
        char *ptr;
        
        length = lseek(fd, 0, SEEK_END);
        if (length > 0)
        {
            lseek(fd, 0, SEEK_SET);
            
            ptr = (char*)rt_malloc(length);
            if (ptr != RT_NULL)
            {
                struct json_tree* tree;
                
                length = read(fd, ptr, length);
                tree = json_tree_parse(ptr, length);
                if (tree != RT_NULL)
                {
                    int index = 0;
                    struct json_node* node;

                    while (1)
                    {
                        const char *url;
                        const char *local_url;
                        
                        node = json_node_get_array(&(tree->root), index, RT_NULL);
                        if (node == RT_NULL) break;
                        index ++;
                        
                        url = json_node_get_string(node, "url", RT_NULL);
                        local_url = json_node_get_string(node, "local_url", RT_NULL);
                        
                        rt_kprintf("%s=>%s\n", local_url, url);
                    }
                    
                    json_tree_destroy(tree);
                }
                
                rt_free(ptr);
            }
        }
        
        close(fd);
    }
}

void startup_checking(struct rt_completion *cmp)
{
    if (filesystem_checking() == RT_FALSE)
    {
        filesystem_format();
    }

    resource_checking();

	network_checking();

    if (cmp)
        rt_completion_done(cmp);
}

#include <finsh.h>
int res_check(int argc, char** argv)
{
    struct rt_completion done;
    rt_completion_init(&done);

    rt_kprintf("Resource checking program!\n");
    startup_checking(&done);
    rt_completion_wait(&done, RT_WAITING_FOREVER);
    rt_kprintf("\nChecking resource files done!!\n");
    
    return 0;
}
MSH_CMD_EXPORT(res_check, resource files checking and download);
