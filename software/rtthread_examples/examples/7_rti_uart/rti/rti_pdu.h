/*
 * Copyright(c) 2010 RT-Thread Development Studio. All rights reserved.
 *
 */
#ifndef __RTI_PDU_H__
#define __RTI_PDU_H__

#define RTI_NAME_MAX	8

#ifdef _MSC_VER
	#ifndef __RT_THREAD_DATA_H__
	typedef signed char    rt_int8_t;
	typedef unsigned char  rt_uint8_t;
	typedef signed short   rt_int16_t;
	typedef unsigned short rt_uint16_t;
	typedef signed long    rt_int32_t;
	typedef unsigned long  rt_uint32_t;
	typedef unsigned long  rt_size_t;
	#endif
#else
#include <rtthread.h>
#endif

/* RealTime Target Insight Data Definitions */

enum rti_data_type
{
	RTI_DATA_UNKNOWN = 0,		/* 0 */
	RTI_DATA_VERSION,			/* 1 */
	RTI_DATA_SWITCH,			/* 2 */
	RTI_DATA_MALLOC,			/* 3 */
	RTI_DATA_FREE,				/* 4 */
	RTI_DATA_MP_ALLOC,			/* 5 */
	RTI_DATA_MP_FREE,			/* 6 */
	RTI_DATA_OBJECT_ATTACH,		/* 7 */
	RTI_DATA_OBJECT_DETACH,		/* 8 */
	RTI_DATA_OBJECT_TRYTAKE,	/* 9 */
	RTI_DATA_OBJECT_TAKE,		/* 10 */
	RTI_DATA_OBJECT_PUT,		/* 11 */
	RTI_DATA_TIMER_TIMEOUT,		/* 12 */
	RTI_DATA_OBJECT,			/* 13 */
	RTI_DATA_THREAD,			/* 14 */
	RTI_DATA_HEAPINFO,			/* 15 */
	RTI_DATA_MEMDUMP,			/* 16 */
	RTI_DATA_LOG,				/* 17 */
	RTI_DATA_DUMMY,				/* 18 */
	RTI_DATA_SYSINFO,           /* 19 */
	RTI_DATA_SEM,           /* 20 */
	RTI_DATA_MUTEX,           /* 21 */
	RTI_DATA_EVENT,           /* 22 */
  RTI_DATA_MAILBOX,           /* 23 */
  RTI_DATA_MSQUEUE,           /* 24 */
  RTI_DATA_TIMER,           /* 25 */
};

#define RTI_MAGIC	0x7e7e
struct rti_frame
{
	rt_uint16_t magic;
	rt_uint16_t length;
	rt_uint32_t seq;
};

struct rti_data
{
	/* rti basic data object */
	rt_uint16_t type;
	rt_uint16_t length;

	rt_uint32_t tick;
};
#define RTI_DATA_INIT(d, t, l)	\
	do { (d)->type = t; (d)->length = l; (d)->tick = rti_get_tick(); } while(0)


struct rti_data_version
{
	struct rti_data parent;
	rt_uint16_t major, minor;
};
#define RTI_DATA_VERSION_INIT(d, version_major, version_minor)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_VERSION, sizeof(struct rti_data_version)); \
		d->major = version_major; d->minor = version_minor; \
	} while (0)

struct rti_data_switch
{
	struct rti_data parent;

	char switch_to[8], switch_out[8];
};
#define RTI_DATA_SWITCH_INIT(d, to, out)	\
	do \
	{  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_SWITCH, sizeof(struct rti_data_switch)); \
		rt_strncpy(d->switch_to,to->name,8); rt_strncpy(d->switch_out,out->name,8); \
	} \
	while (0)

struct rti_data_malloc
{
	struct rti_data parent;

	rt_uint32_t ptr;
	rt_uint32_t size;
};
#define RTI_DATA_MALLOC_INIT(d, mem_ptr, mem_size)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_MALLOC, sizeof(struct rti_data_malloc));	\
		d->ptr = (rt_uint32_t)mem_ptr; d->size = (rt_uint32_t)mem_size;					\
	} while (0)

struct rti_data_free
{
	struct rti_data parent;

	rt_uint32_t ptr;
};
#define RTI_DATA_FREE_INIT(d, mem_ptr)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_FREE, sizeof(struct rti_data_free)); \
		d->ptr = (rt_uint32_t)mem_ptr; \
	} while (0)

struct rti_data_mp_alloc
{
	struct rti_data parent;

	rt_uint32_t mp_id;
	rt_uint32_t ptr;
};
#define RTI_DATA_MP_ALLOC_INIT(d, id, block)	\
	do \
	{  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_MP_ALLOC, sizeof(struct rti_data_mp_alloc)); \
		d->mp_id = (rt_uint32_t)id; d->ptr = (rt_uint32_t)block; \
	} \
	while (0)

struct rti_data_mp_free
{
	struct rti_data parent;

	rt_uint32_t mp_id;
	rt_uint32_t ptr;
};
#define RTI_DATA_MP_FREE_INIT(d, id, block)	\
	do \
	{  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_MP_FREE, sizeof(struct rti_data_mp_free)); \
		d->mp_id = (rt_uint32_t)id; d->ptr = (rt_uint32_t)block; \
	} \
	while (0)

struct rti_data_timer_timeout
{
	struct rti_data parent;

	rt_uint32_t id;
	rt_uint32_t timeout;
};
#define RTI_DATA_TIMER_TIMEOUT_INIT(d, timer_id, timer_timeout) \
	do { \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_TIMER_TIMEOUT, sizeof(struct rti_data_timer_timeout)); \
		d->id = (rt_uint32_t)timer_id; d->timeout = (rt_uint32_t)timer_timeout; \
	} while (0)

struct rti_data_object_attached
{
	struct rti_data parent;

	rt_uint32_t type;
	rt_uint32_t id;
	rt_uint8_t  name[RTI_NAME_MAX];
};
#define RTI_DATA_OBJECT_ATTACH_INIT(d, object_type, object_id, object_name)		\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_OBJECT_ATTACH, sizeof(struct rti_data_object_attached));		\
		d->type = (rt_uint32_t)object_type; d->id = (rt_uint32_t)object_id; \
		rt_strncpy((char*)d->name, (char*)object_name, RTI_NAME_MAX); \
	} while (0)

struct rti_data_object
{
	struct rti_data parent;

	rt_uint32_t type;
	//rt_uint32_t id;
	rt_uint8_t  name[RTI_NAME_MAX];
};
#define RTI_DATA_OBJECT_INIT(d, t, object_type, object_name)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), t, sizeof(struct rti_data_object));		\
		d->type = (rt_uint32_t)object_type; \
		rt_strncpy((char*)d->name, (char*)object_name, RTI_NAME_MAX); \
	} while (0)

#define RTI_DATA_OBJECT_DETACH_INIT(d, object_type, object_name)			\
	RTI_DATA_OBJECT_INIT(d, RTI_DATA_OBJECT_DETACH, object_type, object_name)
#define RTI_DATA_OBJECT_TRYTAKE_INIT(d, object_type, object_name)			\
	RTI_DATA_OBJECT_INIT(d, RTI_DATA_OBJECT_TRYTAKE, object_type, object_name)
#define RTI_DATA_OBJECT_TAKE_INIT(d, object_type, object_name)			\
	RTI_DATA_OBJECT_INIT(d, RTI_DATA_OBJECT_TAKE, object_type, object_name)
#define RTI_DATA_OBJECT_PUT_INIT(d, object_type, object_name)				\
	RTI_DATA_OBJECT_INIT(d, RTI_DATA_OBJECT_PUT, object_type, object_name)

struct rti_data_object_info
{
	struct rti_data parent;

	rt_uint32_t type;
	rt_uint32_t id;
	rt_uint8_t  name[RTI_NAME_MAX];
};
#define RTI_DATA_OBJECT_INFO_INIT(d, object_type, object_id, object_name)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_OBJECT, sizeof(struct rti_data_object_info)); \
		d->type = object_type; d->id = (rt_uint32_t)object_id; \
		rt_strncpy((char*)d->name, (char*)object_name, RTI_NAME_MAX); \
	} while (0)

struct rti_data_thread
{
	struct rti_data parent;

	rt_uint32_t id;
	rt_uint32_t stack_begin, stack_end;
	rt_uint32_t stack_ptr, stack_max;

	rt_uint8_t priority, status;
};
#define RTI_DATA_THREAD_INIT(d, id, stack_begin, stack_end, stack_ptr, stack_max, priority, status)	\
	do{  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_THREAD, sizeof(struct rti_data_thread)); \
		d->tid = id; d->stack_begin = stack_begin; d->stack_end = stack_end; \
		d->stack_ptr = stack_ptr; d->stack_max = stack_max; d->priority = pritority; \
		d->status = status; \
	} while (0)

struct rti_data_log
{
	struct rti_data parent;
};
#define RTI_DATA_LOG_INIT(d, log, length)	\
	do{  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_LOG, length + sizeof(struct rti_data_log)); \
		rt_strncpy((char*)(d + 1), log, length); \
	} while (0)

struct rti_data_heapinfo
{
	struct rti_data parent;

	rt_uint32_t mem_total;
	rt_uint32_t max_used;
	rt_uint32_t current_used;
};
#define RTI_DATA_HEAPINFO_INIT(d, total, max, used)	\
	do{  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_HEAPINFO, sizeof(struct rti_data_heapinfo)); \
		d->mem_total = total; d->max_used = max; d->current_used = used; \
	} while (0)

struct rti_data_memdump
{
	struct rti_data parent;
};
#define RTI_DATA_MEMDUMP_INIT(d, mem, length)	\
	do{  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_MEMDUMP, length + sizeof(struct rti_data_memdump)); \
		rt_memcpy((void*)(d + 1), (void*)mem, length); \
	} while (0)

struct rti_data_dummy
{
	struct rti_data parent;
	rt_uint32_t tid;
};
#define RTI_DATA_DUMMY_INIT(d, id)	\
	do{  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_DUMMY, sizeof(struct rti_data_dummy)); \
		d->tid = (rt_uint32_t)(id); \
	} while (0)

struct rti_data_sysinfo
{
	struct rti_data parent;

	rt_uint16_t name_size;
	rt_uint16_t frame_time;
	rt_uint32_t time_unit;
};
#define RTI_DATA_SYSINFO_INIT(d, nm, ft, tu)	\
	do{  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_SYSINFO, sizeof(struct rti_data_sysinfo)); \
		d->name_size = nm; \
		d->frame_time = ft; \
		d->time_unit  = tu; \
	} while (0)

	struct rti_data_sem
{
	struct rti_data parent;

	rt_uint32_t id;
	rt_uint32_t value;
	
	rt_uint8_t  name[RTI_NAME_MAX];
};
#define RTI_DATA_SEM_INIT(d, sem_id,sem_value, sem_name)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_SEM, sizeof(struct rti_data_sem)); \
		d->id = (rt_uint32_t)sem_id; d->value = (rt_uint32_t)sem_value; \
		rt_strncpy((char*)d->name, (char*)sem_name, RTI_NAME_MAX); \
	} while (0)
	
	
		struct rti_data_mutex
{
	struct rti_data parent;

	rt_uint32_t id;
	rt_uint32_t value;
	rt_uint32_t hold;
	rt_uint8_t  name[RTI_NAME_MAX];
	rt_uint8_t  owner[RTI_NAME_MAX];
};
#define RTI_DATA_MUTEX_INIT(d, mutex_id, mutex_value, mutex_hold, mutex_name, mutex_owner)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_MUTEX, sizeof(struct rti_data_mutex)); \
		d->id = (rt_uint32_t)mutex_id; d->value = (rt_uint32_t)mutex_value; d->hold=(rt_uint32_t)mutex_hold;\
		rt_strncpy((char*)d->name, (char*)mutex_name, RTI_NAME_MAX); \
		rt_strncpy((char*)d->owner, (char*)mutex_owner, RTI_NAME_MAX); \
	} while (0)

struct rti_data_event
{
	struct rti_data parent;
	rt_uint32_t id;
	rt_uint32_t value;
	rt_uint8_t  name[RTI_NAME_MAX];
	
};
#define RTI_DATA_EVENT_INIT(d, event_id, event_value, event_name)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_EVENT, sizeof(struct rti_data_event)); \
		d->id = (rt_uint32_t)event_id; d->value = (rt_uint32_t)event_value; \
		rt_strncpy((char*)d->name, (char*)event_name, RTI_NAME_MAX); \
	} while (0)
	
struct rti_data_mailbox
{
	struct rti_data parent;

	rt_uint32_t id;
	rt_uint32_t value;
	rt_uint32_t size;
	rt_uint8_t  name[RTI_NAME_MAX];
};
#define RTI_DATA_MAILBOX_INIT(d, mb_id, mb_value, mb_size, mb_name)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_MAILBOX, sizeof(struct rti_data_mailbox)); \
		d->id = (rt_uint32_t)mb_id; d->value = (rt_uint32_t)mb_value; d->size=(rt_uint32_t)mb_size;\
		rt_strncpy((char*)d->name, (char*)mb_name, RTI_NAME_MAX); \
	} while (0)
	

struct rti_data_msqueue
{
	struct rti_data parent;
	rt_uint32_t id;
	rt_uint32_t value;
	rt_uint8_t  name[RTI_NAME_MAX];
};
#define RTI_DATA_MSQUEUE_INIT(d, mq_id, mq_value, mq_name)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_MSQUEUE, sizeof(struct rti_data_msqueue)); \
		d->id = (rt_uint32_t)mq_id; d->value = (rt_uint32_t)mq_value; \
		rt_strncpy((char*)d->name, (char*)mq_name, RTI_NAME_MAX); \
	} while (0)
	
	struct rti_data_timer
{
	struct rti_data parent;

	rt_uint32_t id;
	rt_uint32_t periodic;
	rt_int32_t timeout;
	rt_int32_t flag;
	rt_uint8_t  name[RTI_NAME_MAX];
};
#define RTI_DATA_TIMER_INIT(d, tim_id, tim_per,tim_tout,tim_flag, tim_name)	\
	do {  \
		RTI_DATA_INIT(&(d->parent), RTI_DATA_TIMER, sizeof(struct rti_data_msqueue)); \
		d->id = (rt_uint32_t)tim_id; d->periodic = (rt_uint32_t)tim_per; d->timeout = (rt_uint32_t)tim_tout;d->flag = (rt_uint32_t)tim_flag;\
		rt_strncpy((char*)d->name, (char*)tim_name, RTI_NAME_MAX); \
	} while (0)
#endif	
	
