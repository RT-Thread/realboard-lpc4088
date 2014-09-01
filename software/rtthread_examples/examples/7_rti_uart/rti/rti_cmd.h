#ifndef __RTI_CMD_H__
#define __RTI_CMD_H__

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

#define RTI_CMD_MAGIC	0x7e

/* RealTime Target Insight Command Definitions */
struct rti_cmd
{
	rt_uint8_t	magic;
	rt_uint8_t	type;
	rt_uint16_t length;
};

enum rti_cmd_type
{
	/* version */
	RTI_CMD_VERSION,

	/* object information */
	RTI_CMD_OBJECT_INFO,
	
	/* list object */
	RTI_CMD_OBJECT_LIST_THREAD,
	RTI_CMD_OBJECT_LIST_TIMER,
	RTI_CMD_OBJECT_LIST_SEMAPHORE,
	RTI_CMD_OBJECT_LIST_MUTEX,
	RTI_CMD_OBJECT_LIST_MAILBOX,
	RTI_CMD_OBJECT_LIST_MESSAGEQUEUE,
	RTI_CMD_OBJECT_LIST_EVENT,

	/* object information */
	RTI_CMD_OBJECT_THREAD,
	RTI_CMD_OBJECT_TIMER,
	RTI_CMD_OBJECT_SEMAPHORE,
	RTI_CMD_OBJECT_MUTEX,
	RTI_CMD_OBJECT_MAILBOX,
	RTI_CMD_OBJECT_MESSAGEQUEUE,
	RTI_CMD_OBJECT_EVENT,
	RTI_CMD_OBJECT_MEMPOOL,

	/* memory information */
	RTI_CMD_HEAP_INFO,
	RTI_CMD_MEM_INFO,

	/* finsh shell command */
	RTI_CMD_FINSH,
	
	RTI_CMD_SYS_INFO,
};

struct rti_cmd_obj_info
{
	struct rti_cmd parent;

	rt_uint32_t obj_id;
	rt_uint32_t type;
};

struct rti_cmd_mem_info
{
	struct rti_cmd parent;
	
	rt_uint32_t addr_start;
	rt_uint32_t addr_end;
};

struct rti_cmd_finsh
{
	struct rti_cmd parent;

	rt_uint32_t length;
};

#endif
