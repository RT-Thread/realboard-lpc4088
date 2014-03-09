#include <rtthread.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/rtgui_app.h>
#include <calibration.h>
#include <dfs_posix.h>
#include "appmgr.h"
#include "statusbar.h"

#define SETUP_FILE "/setup.bin"

static calculate_data_t *setup_data;

rt_bool_t cali_setup(void)
{
    int fd, length;
    fd = open(SETUP_FILE, O_RDONLY , 0);
    if (fd >= 0)
    {
        length = read(fd, setup_data, sizeof(calibration_typedef));
        close(fd);
        if (length > 0)
        {   
					  calibration_set_data(setup_data);
            return RT_TRUE;
        }
        else
        {
            return RT_FALSE;
        }
    }
    close(fd);
    return RT_FALSE;
}

rt_bool_t cali_store(calibration_typedef *data)
{
     int fd, length;
    fd = open(SETUP_FILE, O_WRONLY | O_CREAT, 0);
    if (fd >= 0)
    {
        length = write(fd, setup_data, sizeof(calibration_typedef));
        close(fd);
        if (length > 0)
        {
            return RT_TRUE;
        }
        else
        {
            return RT_FALSE;
        }
    }
    close(fd);
    return RT_FALSE;
}

void realtouch_ui_init(void)
{
    rt_device_t device;    
    struct rt_device_rect_info info;    

    device = rt_device_find("lcd");    
    if (device != RT_NULL)    
    {        
        info.width = 480;        
        info.height = 272;        
        /* set graphic resolution */        
        rt_device_control(device, RTGRAPHIC_CTRL_SET_MODE, &info);    
    }    

    /* re-set graphic device */    
    rtgui_graphic_set_device(device);       

    app_mgr_init();
    rt_thread_delay(10);
    setup_data=(calculate_data_t *)rt_malloc(sizeof(calibration_typedef));
		calibration_set_restore(cali_setup);
		calibration_set_after(cali_store);
		calibration_set_data(setup_data);
		if(cali_setup()!=RT_TRUE)
			{
       calibration_init();
      }
}

