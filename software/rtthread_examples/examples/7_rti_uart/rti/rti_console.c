#include <rtthread.h>
#include "rti.h"
#include "rti_pdu.h"

static rt_size_t rti_console_device_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
	struct rti_data_log* log;

	if (rti_get_filter() & (1 << RTI_DATA_LOG)) return 0;

	log = (struct rti_data_log*)rti_get_buffer(size + 
		sizeof(struct rti_data_log));
	if (log != RT_NULL)
	{
		RTI_DATA_LOG_INIT(log, buffer, size);
		rti_put_chunk((rt_uint8_t*)log);
	}

	return size;
}

static struct rt_device _rti_device;
void rti_console_device_init()
{
    /* register rti device */
    _rti_device.type    = RT_Device_Class_Char;
    _rti_device.init    = RT_NULL;
    _rti_device.open    = RT_NULL;
    _rti_device.close   = RT_NULL;
    _rti_device.read 	= RT_NULL;
    _rti_device.write   = rti_console_device_write;
    _rti_device.control = RT_NULL;
    /* no private */
    _rti_device.user_data = RT_NULL;

    rt_device_register(&_rti_device, "rti",
                       RT_DEVICE_FLAG_WRONLY | RT_DEVICE_FLAG_STANDALONE);	
}
