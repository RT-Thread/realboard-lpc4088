#include "rti.h"
#include <rthw.h>

#include "rti_pdu.h"
#include "rti_cmd.h"

#ifdef RT_USING_RTI

struct rti
{
	rt_uint8_t  chunks_refcnt[RTI_CHUNK_NUM];	/* each chunk reference count	*/
	rt_uint16_t chunk_index;		/* current chunk index			*/
	rt_uint16_t chunk_pop_index;	/* current read/pop chunk index	*/
	rt_uint16_t chunk_offset;		/* memory offset in chunk		*/

	rt_uint32_t filter;				/* hook filter					*/
	rt_uint32_t frame_sn;			/* sequence number of frame		*/
	rt_uint32_t frame_tick;			/* the begin tick of frame      */

	const struct rti_setting *setting;	/* setting of RTI			*/
};
struct rti _rti;

/* get data buffer from RTI */
rt_uint8_t *rti_get_buffer(rt_size_t length)
{
	rt_uint8_t* result;
	rt_uint32_t level;

	result = RT_NULL;

	level = rt_hw_interrupt_disable();
	if (_rti.chunk_offset + length > _rti.setting->chunk_size)
	{
		rt_uint16_t next_index;
		struct rti_frame* frame;

		/* current frame: set current chunk size */
		frame = (struct rti_frame*)(_rti.setting->chunk_memory + _rti.chunk_index * _rti.setting->chunk_size);
		frame->magic = RTI_MAGIC;
		frame->length = RT_ALIGN(_rti.chunk_offset, 4);
		frame->seq = _rti.frame_sn++;

		/* switch to next chunk */
		next_index = _rti.chunk_index + 1;
		if (next_index >= _rti.setting->chunk_num) next_index = 0;

		/* check queue is full */
		if (next_index != _rti.chunk_pop_index)
		{
			/* new frame: set new chunk index and offset */
			_rti.chunk_index = next_index;
			_rti.frame_tick  = rti_get_tick(); /* get the tick of beginning frame */

			/* skip frame header */
			_rti.chunk_offset = sizeof(struct rti_frame);
			result = _rti.setting->chunk_memory + _rti.chunk_index*_rti.setting->chunk_size + _rti.chunk_offset;
			/* increase reference counter */
			_rti.chunks_refcnt[_rti.chunk_index] ++;
			/* set offset */
			_rti.chunk_offset += length;
		}
	}
	else
	{
		/* get buffer from current chunk */
		result = _rti.setting->chunk_memory + _rti.chunk_index*_rti.setting->chunk_size + _rti.chunk_offset;
		/* increase reference counter */
		_rti.chunks_refcnt[_rti.chunk_index] ++;
		/* set offset */
		_rti.chunk_offset += length;
	}
	rt_hw_interrupt_enable(level);

	// rt_kprintf("rti buf: 0x%08x\n", result);
	return result;
}

void rti_put_chunk(rt_uint8_t *chunk)
{
	rt_uint32_t index, level;

	index = ((rt_uint32_t)chunk - (rt_uint32_t)_rti.setting->chunk_memory)/_rti.setting->chunk_size;

	level = rt_hw_interrupt_disable();
	_rti.chunks_refcnt[index] --;
	rt_hw_interrupt_enable(level);
	
	rti_has_buffer();
}
 
void rti_put_next_chunk()
{
	rt_uint32_t level;
	rt_uint32_t next_index;
	struct rti_data_dummy* dummy;
	struct rti_frame* frame;

	/* made a dummy data */
	dummy = (struct rti_data_dummy*)rti_get_buffer(sizeof(struct rti_data_dummy));
	if (dummy == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_DUMMY_INIT(dummy, rt_thread_self());
	rti_put_chunk((rt_uint8_t*)dummy);

	level = rt_hw_interrupt_disable();

	/* current frame: set current chunk size */
	frame = (struct rti_frame*)(_rti.setting->chunk_memory + _rti.chunk_index * _rti.setting->chunk_size);
	frame->magic = RTI_MAGIC;
	frame->length = RT_ALIGN(_rti.chunk_offset, 4);
	frame->seq = _rti.frame_sn++;

	/* switch to next chunk */
	next_index = _rti.chunk_index + 1;
	if (next_index >= _rti.setting->chunk_num) next_index = 0;

	/* check queue is full */
	if (next_index != _rti.chunk_pop_index)
	{
		/* new frame: set new chunk index and offset */
		_rti.chunk_index = next_index;
		_rti.frame_tick  = rti_get_tick(); /* get the tick of beginning frame */

		/* skip frame header */
		_rti.chunk_offset = sizeof(struct rti_frame);
	}
	rt_hw_interrupt_enable(level);
}

/* get a data chunk from RTI */
rt_uint8_t *rti_get_chunk()
{
	rt_uint8_t *result = RT_NULL;

	/* rti queue is not empty */
	if ((_rti.chunk_pop_index != _rti.chunk_index) && 
		(_rti.chunks_refcnt[_rti.chunk_pop_index] == 0))
	{
		result = _rti.setting->chunk_memory + _rti.chunk_pop_index * _rti.setting->chunk_size;
	}

	return result;
}

void rti_chunk_next()
{
	rt_uint32_t level;

	level = rt_hw_interrupt_disable();
	_rti.chunk_pop_index ++;
	if (_rti.chunk_pop_index >= _rti.setting->chunk_num)
	{
		_rti.chunk_pop_index = 0;
	}
	rt_hw_interrupt_enable(level);
}

rt_uint32_t rti_get_filter()
{
	return _rti.filter;
}

void rti_chunk_flush(void)
{
	struct rti_frame* frame;

	/* get rti data chunk */
	frame = (struct rti_frame*)rti_get_chunk();
	while (frame != RT_NULL)
	{
		rti_tx((rt_uint8_t*)frame, frame->length);
		rti_chunk_next();
		
		/* get next chunk */
		frame = (struct rti_frame*)rti_get_chunk();
	}
}

/* responces for some command */
void rti_get_sysinfo()
{
	struct rti_data_sysinfo* sysinfo;
	sysinfo = (struct rti_data_sysinfo*)rti_get_buffer(sizeof(struct rti_data_sysinfo));
	if (sysinfo == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_SYSINFO_INIT(sysinfo, 
		RT_NAME_MAX, 
		_rti.setting->frame_time/_rti.setting->time_unit, 
		_rti.setting->time_unit);
	rti_put_chunk((rt_uint8_t*)sysinfo);
}


/* ---- OS Hook ---- */
/*
 * idle hook function will check whether rti data is available,
 * if rti data is available, this function will send it to the
 * host machine.
 */
static void rti_thread_idle_hook()
{
	struct rti_frame* frame;

	/* get rti data chunk */
	frame = (struct rti_frame*)rti_get_chunk();
	if (frame != RT_NULL)
	{
		rti_tx((rt_uint8_t*)frame, frame->length);
		rti_chunk_next();
	}
	else
	{
		/* the maximal time of frame is 1 second */
		if (rti_get_tick() - _rti.frame_tick > _rti.setting->frame_time) 
		{
			rti_put_next_chunk();
		}
	}
}

static void rti_scheduler_hook(struct rt_thread* from, struct rt_thread* to)
{
	struct rti_data_switch* thread_switch;
	if (_rti.filter & (1 << RTI_DATA_SWITCH)) return;

	
	thread_switch = (struct rti_data_switch*)rti_get_buffer(sizeof(struct rti_data_switch));
	if (thread_switch == RT_NULL)
	{
		rti_no_buffer();
		return;
	}
  
	RTI_DATA_SWITCH_INIT(thread_switch, to, from);
	rti_put_chunk((rt_uint8_t*)thread_switch);
}

static void rti_malloc_hook(void *ptr, rt_size_t size)
{
	struct rti_data_malloc* data_malloc;

	if (_rti.filter & (1 << RTI_DATA_MALLOC)) return;

	data_malloc = (struct rti_data_malloc*)rti_get_buffer(sizeof(struct rti_data_malloc));
	if (data_malloc == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_MALLOC_INIT(data_malloc, ptr, size);
	rti_put_chunk((rt_uint8_t*)data_malloc);
}

static void rti_free_hook(void *ptr)
{
	struct rti_data_free* data_free;

	if (_rti.filter & (1 << RTI_DATA_FREE)) return;

	data_free = (struct rti_data_free*)rti_get_buffer(sizeof(struct rti_data_free));
	if (data_free == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_FREE_INIT(data_free, ptr);
	rti_put_chunk((rt_uint8_t*)data_free);
}

static void rti_mp_alloc_hook(struct rt_mempool* mp, void *ptr)
{
	struct rti_data_mp_alloc* mp_alloc;

	if (_rti.filter & (1 << RTI_DATA_MP_ALLOC)) return;

	mp_alloc = (struct rti_data_mp_alloc*)rti_get_buffer(sizeof(struct rti_data_mp_alloc));
	if (mp_alloc == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_MP_ALLOC_INIT(mp_alloc, mp, ptr);
	rti_put_chunk((rt_uint8_t*)mp_alloc);
}

static void rti_mp_free_hook(struct rt_mempool* mp, void *ptr)
{
	struct rti_data_mp_free* mp_free;

	if (_rti.filter & (1 << RTI_DATA_MP_FREE)) return;

	mp_free = (struct rti_data_mp_free*)rti_get_buffer(sizeof(struct rti_data_mp_free));
	if (mp_free == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_MP_FREE_INIT(mp_free, mp, ptr);
	rti_put_chunk((rt_uint8_t*)mp_free);
}

static void rti_object_attach_hook(struct rt_object* object)
{
	struct rti_data_object_attached* data_object;
	
	if (_rti.filter & (1 << RTI_DATA_OBJECT_ATTACH)) return;

	data_object = (struct rti_data_object_attached*)rti_get_buffer(sizeof(struct rti_data_object_attached));
	if (data_object == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_OBJECT_ATTACH_INIT(data_object, object->type, object, object->name);
	rti_put_chunk((rt_uint8_t*)data_object);
}

static void rti_object_detach_hook(struct rt_object* object)
{
	struct rti_data_object* data_object;

	if (_rti.filter & (1 << RTI_DATA_OBJECT_DETACH)) return;

	data_object = (struct rti_data_object*)rti_get_buffer(sizeof(struct rti_data_object));
	if (data_object == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_OBJECT_DETACH_INIT(data_object, object->type, object->name);
	rti_put_chunk((rt_uint8_t*)data_object);
}

static void rti_object_trytake_hook(struct rt_object* object)
{
	struct rti_data_object* data_object;

	if (_rti.filter & (1 << RTI_DATA_OBJECT_TRYTAKE)) return;

	data_object = (struct rti_data_object*)rti_get_buffer(sizeof(struct rti_data_object));
	if (data_object == RT_NULL)
	{
		rti_no_buffer();
		return;
	}
  
	RTI_DATA_OBJECT_TRYTAKE_INIT(data_object, object->type, object->name);
	rti_put_chunk((rt_uint8_t*)data_object);
}

static void rti_object_take_hook(struct rt_object* object)
{
	struct rti_data_object* data_object;

	if (_rti.filter & (1 << RTI_DATA_OBJECT_TAKE)) return;

	data_object = (struct rti_data_object*)rti_get_buffer(sizeof(struct rti_data_object));
	if (data_object == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_OBJECT_TAKE_INIT(data_object, object->type, object->name);
	rti_put_chunk((rt_uint8_t*)data_object);
}

static void rti_object_put_hook(struct rt_object* object)
{
	struct rti_data_object* data_object;

	if (_rti.filter & (1 << RTI_DATA_OBJECT_PUT)) return;

	data_object = (struct rti_data_object*)rti_get_buffer(sizeof(struct rti_data_object));
	if (data_object == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_OBJECT_PUT_INIT(data_object, object->type, object->name);
	rti_put_chunk((rt_uint8_t*)data_object);
}

static void rti_timer_timeout_hook(struct rt_timer* timer)
{
	struct rti_data_timer_timeout* data_timer;

	if (_rti.filter & (1 << RTI_DATA_TIMER_TIMEOUT)) return;

	data_timer = (struct rti_data_timer_timeout*)rti_get_buffer(sizeof(struct rti_data_timer_timeout));
	if (data_timer == RT_NULL)
	{
		rti_no_buffer();
		return;
	}

	RTI_DATA_TIMER_TIMEOUT_INIT(data_timer, timer, timer->timeout_func);
	rti_put_chunk((rt_uint8_t*)data_timer);
}

void rti_init(const struct rti_setting *setting)
{
	struct rti_frame* frame;

	/* initial rti structure */
	rt_memset(&_rti, 0, sizeof(_rti));

	/* set rti setting */
	_rti.setting = setting;
	_rti.filter  = setting->filter;

	/* set first frame */
	frame = (struct rti_frame*)_rti.setting->chunk_memory;
	frame->magic = RTI_MAGIC;
	_rti.chunk_offset = sizeof(struct rti_frame);	

	/* set each hook function */
	/* object hook */
	rt_object_attach_sethook(rti_object_attach_hook);
	rt_object_detach_sethook(rti_object_detach_hook);
	rt_object_trytake_sethook(rti_object_trytake_hook);
	rt_object_take_sethook(rti_object_take_hook);
	rt_object_put_sethook(rti_object_put_hook);


	/* memory pool hook */
	rt_mp_alloc_sethook(rti_mp_alloc_hook);
	rt_mp_free_sethook(rti_mp_free_hook);

	/* heap hook */
	rt_malloc_sethook(rti_malloc_hook);
	rt_free_sethook(rti_free_hook);


	/* scheduler hook */
	rt_scheduler_sethook(rti_scheduler_hook);

	/* timer timeout hook */
	rt_timer_timeout_sethook(rti_timer_timeout_hook);

	/* idle hook */
	rt_thread_idle_sethook(rti_thread_idle_hook);

#ifdef RTI_USING_SERVER
	rti_server_init(_rti.setting->device);
#endif
}
#endif

#include <finsh.h>
void rti()
{
	int i;

	rt_kprintf("index %d, pop %d\n", _rti.chunk_index, _rti.chunk_pop_index);
	rt_kprintf("offset %d, frame sn: %d\n", _rti.chunk_offset, _rti.frame_sn);

	rt_kprintf("ref:");
	for (i = 0; i < RTI_CHUNK_NUM; i++)
	{
		rt_kprintf("%d ", _rti.chunks_refcnt[i]);
	}
	rt_kprintf("\n");
}
FINSH_FUNCTION_EXPORT(rti, dump rti);
