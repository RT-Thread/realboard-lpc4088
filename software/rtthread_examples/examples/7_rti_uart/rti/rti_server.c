#include <rtthread.h>
#include "rti.h"
#include "rti_pdu.h"
#include "rti_cmd.h"

static rt_device_t rti_device = RT_NULL;

/* RealTime Target Insight Server */
#ifdef RTI_USING_SERVER
struct rt_semaphore rti_sem;
static rt_uint8_t rti_srv_stack[512];
static struct rt_thread rti_srv_thread;

#define rt_list_entry(node, type, member) \
    ((type *)((char *)(node) - (unsigned long)(&((type *)0)->member)))
extern struct rt_object_information rt_object_container[];

/* rti command */
void rti_list_thread()
{
	struct rt_object *obj;
	struct rt_list_node *list, *node;
	struct rti_data_object_info *obj_info;

	rt_enter_critical();
	list = &rt_object_container[RT_Object_Class_Thread].object_list;
	for (node = list->next; node != list; node = node->next)
	{
		obj = (struct rt_object*)(rt_list_entry(node, struct rt_object, list));
		obj_info = (struct rti_data_object_info*)rti_get_buffer(sizeof(struct rti_data_object_info));
		if (obj_info == RT_NULL)
		{
			rti_no_buffer();
			rt_exit_critical();
			return;
		}

		RTI_DATA_OBJECT_INFO_INIT(obj_info, RT_Object_Class_Thread, obj, obj->name);
		rti_put_chunk((rt_uint8_t*)obj_info);
	}
	rt_exit_critical();
}

void rti_list_timer()
{
	struct rt_object *obj;
	struct rt_list_node *list, *node;
	struct rti_data_object_info *obj_info;

	rt_enter_critical();
	list = &rt_object_container[RT_Object_Class_Timer].object_list;
	for (node = list->next; node != list; node = node->next)
	{
		obj = (struct rt_object*)(rt_list_entry(node, struct rt_object, list));
		obj_info = (struct rti_data_object_info*)rti_get_buffer(sizeof(struct rti_data_object_info));
		if (obj_info == RT_NULL)
		{
			rti_no_buffer();
			rt_exit_critical();
			return;
		}

		RTI_DATA_OBJECT_INFO_INIT(obj_info, RT_Object_Class_Timer, obj, obj->name);
		rti_put_chunk((rt_uint8_t*)obj_info);
	}
	rt_exit_critical();
}

void rti_list_semaphore()
{
	struct rti_data_sem *data_sem;
	struct rt_list_node *list, *node;
	struct rt_semaphore *sem;

	rt_enter_critical();
	list = &rt_object_container[RT_Object_Class_Semaphore].object_list;
	for (node = list->next; node != list; node = node->next)
	{
		sem = (struct rt_semaphore *)(rt_list_entry(node, struct rt_object, list));
		data_sem = (struct rti_data_sem*)rti_get_buffer(sizeof(struct rti_data_sem));
		if (data_sem == RT_NULL)
		{
			rti_no_buffer();
			rt_exit_critical();
			return;
		}

		RTI_DATA_SEM_INIT(data_sem,sem,sem->value,sem->parent.parent.name);
		rti_put_chunk((rt_uint8_t*)data_sem);
	}
	rt_exit_critical();
}

void rti_list_mutex()
{
	struct rt_mutex *mutex;
	struct rt_list_node *list, *node;
	struct rti_data_mutex *data_mutex;

	rt_enter_critical();
	list = &rt_object_container[RT_Object_Class_Mutex].object_list;
	for (node = list->next; node != list; node = node->next)
	{
		mutex = (struct rt_mutex*)(rt_list_entry(node, struct rt_object, list));
		data_mutex = (struct rti_data_mutex*)rti_get_buffer(sizeof(struct rti_data_mutex));
		if (data_mutex== RT_NULL)
		{
			rti_no_buffer();
			rt_exit_critical();
			return;
		}

		RTI_DATA_MUTEX_INIT(data_mutex, mutex, mutex->value,mutex->hold,mutex->parent.parent.name,mutex->owner->name);
		rti_put_chunk((rt_uint8_t*)data_mutex);
	}
	rt_exit_critical();
}

void rti_list_mailbox()
{
	struct rt_mailbox *mb;
	struct rt_list_node *list, *node;
	struct rti_data_mailbox *rti_mb;

	rt_enter_critical();
	list = &rt_object_container[RT_Object_Class_MailBox].object_list;
	for (node = list->next; node != list; node = node->next)
	{
		mb = (struct rt_mailbox*)(rt_list_entry(node, struct rt_object, list));
		rti_mb = (struct rti_data_mailbox*)rti_get_buffer(sizeof(struct rti_data_mailbox));
		if (rti_mb == RT_NULL)
		{
			rti_no_buffer();
			rt_exit_critical();
			return;
		}
		RTI_DATA_MAILBOX_INIT(rti_mb,mb,mb->entry,mb->size, mb->parent.parent.name);
		rti_put_chunk((rt_uint8_t*)rti_mb);
	}
	rt_exit_critical();
}

void rti_list_messagequeue()
{
	struct rt_messagequeue *mq;
	struct rt_list_node *list, *node;
	struct rti_data_msqueue *rti_mq;
	rt_enter_critical();
	list = &rt_object_container[RT_Object_Class_MessageQueue].object_list;
	for (node = list->next; node != list; node = node->next)
	{
		mq = (struct rt_messagequeue*)(rt_list_entry(node, struct rt_object, list));
		rti_mq = (struct rti_data_msqueue*)rti_get_buffer(sizeof(struct rti_data_msqueue));
		if (rti_mq == RT_NULL)
		{
			rti_no_buffer();
			rt_exit_critical();
			return;
		}

		RTI_DATA_MSQUEUE_INIT(rti_mq, mq, mq->entry, mq->parent.parent.name);
		rti_put_chunk((rt_uint8_t*)rti_mq);
	}
	rt_exit_critical();
}

void rti_list_event()
{
	struct rt_event *event;
	struct rt_list_node *list, *node;
	struct rti_data_event *rti_event;

	rt_enter_critical();
	list = &rt_object_container[RT_Object_Class_Event].object_list;
	for (node = list->next; node != list; node = node->next)
	{
		event = (struct rt_event*)(rt_list_entry(node, struct rt_object, list));
		rti_event = (struct rti_data_event *)rti_get_buffer(sizeof(struct rti_data_event));
		if (rti_event == RT_NULL)
		{
			rti_no_buffer();
			rt_exit_critical();
			return;
		}
		
		RTI_DATA_EVENT_INIT(rti_event, event, event->set, event->parent.parent.name);
		rti_put_chunk((rt_uint8_t*)rti_event);
	}
	rt_exit_critical();
}

void rti_version()
{
	struct rti_data_version* version;

	if (rti_get_filter() & (1 << RTI_DATA_VERSION)) return;

	version = (struct rti_data_version*)rti_get_buffer(sizeof(struct rti_data_version));
	if (version != RT_NULL)
	{
		RTI_DATA_VERSION_INIT(version, 1, 0);
	}
	rti_put_chunk((rt_uint8_t*)version);
}

void rti_heap_info()
{
	struct rti_data_heapinfo* info;
	rt_uint32_t total, max, used;

	if (rti_get_filter() & (1 << RTI_DATA_HEAPINFO)) return;

	info = (struct rti_data_heapinfo*)rti_get_buffer(sizeof(struct rti_data_heapinfo));
	if (info != RT_NULL)
	{
		rt_memory_info(&total, &max, &used);
		RTI_DATA_HEAPINFO_INIT(info, total, max, used);
		rti_put_chunk((rt_uint8_t*)info);
	}
}

void rti_mem_info(rt_uint32_t begin, rt_uint32_t end)
{
	rt_uint32_t length;
	struct rti_data_memdump* mem;

	if (rti_get_filter() & (1 << RTI_DATA_MEMDUMP)) return;

	/* take a alignment */
	begin = RT_ALIGN(begin, RT_ALIGN_SIZE);
	end   = RT_ALIGN(end, RT_ALIGN_SIZE);
	/* get length */
	length = begin - end;

	mem = (struct rti_data_memdump*)rti_get_buffer(length + 
		sizeof(struct rti_data_memdump));
	if (mem != RT_NULL)
	{
		RTI_DATA_MEMDUMP_INIT(mem, begin, length);
		rti_put_chunk((rt_uint8_t*)mem);
	}
}

void rti_object_information(rt_uint32_t obj, rt_uint32_t type)
{
	struct rt_object* obj_ptr;
	struct rt_list_node *list, *node;
	struct rti_data_object_info *obj_info;

	/* remove static attribute */	
	if (type & RT_Object_Class_Static) type = type & ~(RT_Object_Class_Static);

	/* parameter check */
	if ((obj == 0) || (type > RT_Object_Class_Unknown)) return;

	rt_enter_critical();
	list = &rt_object_container[type].object_list;
	for (node = list->next; node != list; node = node->next)
	{
		obj_ptr = (struct rt_object*)(rt_list_entry(node, struct rt_object, list));
		if (obj_ptr == (struct rt_object*)obj)
		{
			/* find object, get its information */
			obj_info = (struct rti_data_object_info*)rti_get_buffer(sizeof(struct rti_data_object_info));
			if (obj_info == RT_NULL)
			{
				rt_exit_critical();
				rti_no_buffer();
				return;
			}

			RTI_DATA_OBJECT_INFO_INIT(obj_info, obj_ptr->type, obj_ptr, obj_ptr->name);
			rti_put_chunk((rt_uint8_t*)obj_info);
			rt_exit_critical();
			return;
		}
	}
	rt_exit_critical();

	/* can't find object */
	return;
}


struct rti_cmd* rti_rx_cmd()
{
	rt_uint8_t ch;
	rt_size_t index;
	struct rti_cmd* cmd;

	static rt_uint8_t cmd_rx_buffer[32];
	static rt_uint8_t cmd_rx_offset = 0;

	cmd = (struct rti_cmd*)&cmd_rx_buffer[0];
  
	/* fix: read byte one by one */
	while (rt_device_read(rti_device, 0, &ch, 1) == 1)
	{
		cmd_rx_buffer[cmd_rx_offset++] = ch;
		if (cmd_rx_offset >= sizeof(cmd_rx_buffer))
		{
			/* reset command buffer */
			cmd_rx_offset = 0;
		}
	}
  rt_kprintf("magic:0x%2X,type:%d,len:%d\n",cmd->magic,cmd->type,cmd->length);
	if ((cmd->magic != RTI_CMD_MAGIC) && cmd_rx_offset != 0)
	{
		rt_kprintf("drop command\n");

		/* drop the wrong command */
		for (index = 0; index < cmd_rx_offset; index ++)
		{
			if (cmd_rx_buffer[index] == RTI_CMD_MAGIC)
			{
				rt_size_t reset_index = 0;

				for (reset_index = 0; index < cmd_rx_offset; reset_index ++, index++)
					cmd_rx_buffer[reset_index] = cmd_rx_buffer[index];
  
				/* reset rx offset */
				cmd_rx_offset = reset_index;
				goto __return;
			}
		}

		/* all data are wrong */
		cmd_rx_offset = 0;
	}
	else if ((cmd->magic == RTI_CMD_MAGIC) && cmd_rx_offset > sizeof(struct rti_cmd))
	{
		
		/* command header wrong, drop all data */
		if (cmd->length > sizeof(cmd_rx_buffer)) cmd_rx_offset = 0;
	}
__return:
	if (cmd_rx_offset >= (sizeof(struct rti_cmd)-1) && (cmd->length  <= cmd_rx_offset))
	{
		rt_size_t cmd_length;

		cmd_length = cmd->length;

		/* allocate new command buffer */
		cmd = (struct rti_cmd*) rt_malloc(cmd_length);
		if (cmd != RT_NULL)
		{
			/* copy to the new buffer */
			rt_memcpy(cmd, cmd_rx_buffer, cmd_length);

			/* remove the copied command buffer */
			for (index = 0; index < cmd_rx_offset - cmd_length; index ++)
				cmd_rx_buffer[index] = cmd_rx_buffer[cmd_length + index];

			cmd_rx_offset = index;
		}

		return cmd;
	}

	return RT_NULL;
}

void rti_server(void* parameter)
{
	struct rti_cmd* cmd;

	/* output realtime target insight version firstly */
	rti_version();

	while (1)
	{
		/* get rti command */
		rt_sem_take(&rti_sem, RT_WAITING_FOREVER);
		cmd = rti_rx_cmd();
		if (cmd != RT_NULL)
		{
			rt_kprintf("cmd: %d\n", cmd->type);
			switch (cmd->type)
			{
			case RTI_CMD_VERSION:
				rti_version();
				rt_thread_delay(1);
				rti_list_thread();
				rt_thread_delay(1);
				rti_list_timer();
				rt_thread_delay(1);
				rti_list_semaphore();
				rt_thread_delay(1);
				rti_list_mutex();
				rt_thread_delay(1);
				rti_list_mailbox();
				rt_thread_delay(1);
				rti_list_messagequeue();
				rt_thread_delay(1);
				rti_list_event();
				break;

			case RTI_CMD_OBJECT_INFO:
				{
				struct rti_cmd_obj_info* obj_info;
				
				obj_info = (struct rti_cmd_obj_info*)cmd;
				rti_object_information(obj_info->obj_id, obj_info->type);
				}
				break;

			case RTI_CMD_OBJECT_LIST_THREAD:
				rti_list_thread();
				break;
			case RTI_CMD_OBJECT_LIST_TIMER:
				rti_list_timer();
				break;
			case RTI_CMD_OBJECT_LIST_SEMAPHORE:
				rti_list_semaphore();
				break;
			case RTI_CMD_OBJECT_LIST_MUTEX:
				rti_list_mutex();
				break;
			case RTI_CMD_OBJECT_LIST_MAILBOX:
				rti_list_mailbox();
				break;
			case RTI_CMD_OBJECT_LIST_MESSAGEQUEUE:
				rti_list_messagequeue();
				break;
			case RTI_CMD_OBJECT_LIST_EVENT:
				rti_list_event();
				break;

			/* object information */
			case RTI_CMD_OBJECT_THREAD:
				break;
			case RTI_CMD_OBJECT_TIMER:
				break;
			case RTI_CMD_OBJECT_SEMAPHORE:
				break;
			case RTI_CMD_OBJECT_MUTEX:
				break;
			case RTI_CMD_OBJECT_MAILBOX:
				break;
			case RTI_CMD_OBJECT_MESSAGEQUEUE:
				break;
			case RTI_CMD_OBJECT_EVENT:
				break;
			case RTI_CMD_OBJECT_MEMPOOL:
				break;

			/* memory information */
			case RTI_CMD_HEAP_INFO:
				rti_heap_info();
				break;
			case RTI_CMD_MEM_INFO:
				{
				struct rti_cmd_mem_info* info;

				info = (struct rti_cmd_mem_info*) cmd;
				rti_mem_info(info->addr_start, info->addr_end);
				}
				break;
			
			case RTI_CMD_SYS_INFO:
				rti_get_sysinfo();
				break;
			}
			rt_free(cmd);
		}
	}
}

#ifdef RT_USING_FINSH
#include <finsh.h>
void list_rti(void)
{
	rti_version();
	rt_thread_delay(1);
	rti_list_thread();
	rt_thread_delay(1);
	rti_list_timer();
	rt_thread_delay(1);
	rti_list_semaphore();
	rt_thread_delay(1);
	rti_list_mutex();
	rt_thread_delay(1);
	rti_list_mailbox();
	rt_thread_delay(1);
	rti_list_messagequeue();
	rt_thread_delay(1);
	rti_list_event();
}
FINSH_FUNCTION_EXPORT(list_rti, list rti version);
#endif

static rt_err_t cmd_rx_ind(rt_device_t dev, rt_size_t size)
{
	RT_ASSERT(dev != RT_NULL);
  rt_kprintf("recv data\n");
	/* release semaphore to let finsh thread rx data */
	rt_sem_release(&rti_sem);

	return RT_EOK;
}

void rti_server_init(const char* device)
{
	rt_err_t result;

	/* initialize semaphore */
	result = rt_sem_init(&rti_sem, "rti", 0, RT_IPC_FLAG_FIFO);
	//RT_ASSERT(result != RT_EOK)

	/* initialize RTI output device */
	rti_device = rt_device_find(device);
	if (rti_device != RT_NULL)
	{
		/* open device */
		rt_device_open(rti_device, RT_DEVICE_OFLAG_RDWR|RT_DEVICE_FLAG_INT_RX);

		/* set rx indication */
		rt_device_set_rx_indicate(rti_device, cmd_rx_ind);
	}

	result = rt_thread_init(&rti_srv_thread, "rti",
		rti_server, RT_NULL,
		&rti_srv_stack[0], sizeof(rti_srv_stack),
		20, 5);
	if (result == RT_EOK)
	{
		rt_thread_startup(&rti_srv_thread);
	}
	else
	{
		/* detach semaphore */
		rt_sem_detach(&rti_sem);
	}
}
#endif

void rti_tx(rt_uint8_t* ptr, rt_uint32_t size)
{
	if (rti_device != RT_NULL)
	{
		rt_device_write(rti_device, 0, ptr, size);
	}
}
