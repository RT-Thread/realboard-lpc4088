#ifndef _RTI_DATA_OBJECT_H_
#define _RTI_DATA_OBJECT_H_

#include "../rti_pdu.h"

#define RTI_OBJECT_NAME_SIZE	32

class rti_object
{
public:
	static rti_object* parse(rt_uint8_t* data, rt_uint16_t size);

public:
	rt_uint16_t type;
	rt_uint16_t size;
	rt_uint32_t tick;
};

class rti_object_version : public rti_object
{
public:
	rt_uint16_t major, minor;
};

class rti_object_switch : public rti_object
{
public:
	rt_uint32_t from;
	char frome_name[RTI_OBJECT_NAME_SIZE];
	rt_uint32_t to;
	char to_name[RTI_OBJECT_NAME_SIZE];
};

class rti_object_malloc : public rti_object
{
public:
	rt_uint32_t ptr;
	rt_uint32_t size;
};

class rti_object_free : public rti_object
{
public:
	rt_uint32_t ptr;
};

class rti_object_mp_alloc : public rti_object
{
public:
	rt_uint32_t mp_id;
	char mp_name[RTI_OBJECT_NAME_SIZE];

	rt_uint32_t ptr;
};

class rti_object_mp_free : public rti_object
{
public:
	rt_uint32_t mp_id;
	char mp_name[RTI_OBJECT_NAME_SIZE];

	rt_uint32_t ptr;
};

class rti_object_attached : public rti_object
{
public:
	rt_uint32_t type;
	rt_uint32_t id;
	rt_uint8_t  name[RTI_OBJECT_NAME_SIZE];
};

class rti_object_detach : public rti_object
{
public:
	rt_uint32_t type;

	rt_uint32_t id;
	rt_uint8_t  name[RTI_OBJECT_NAME_SIZE];
};

class rti_object_trytake : public rti_object
{
public:
	rt_uint32_t type;

	rt_uint32_t id;
	rt_uint8_t  name[RTI_OBJECT_NAME_SIZE];
};

class rti_object_take : public rti_object
{
public:
	rt_uint32_t type;

	rt_uint32_t id;
	rt_uint8_t  name[RTI_OBJECT_NAME_SIZE];
};

class rti_object_put : public rti_object
{
public:
	rt_uint32_t type;

	rt_uint32_t id;
	rt_uint8_t  name[RTI_OBJECT_NAME_SIZE];
};

class rti_object_timeout : public rti_object
{
public:
	rt_uint32_t id;
	rt_uint8_t  name[RTI_OBJECT_NAME_SIZE];
	rt_uint32_t timeout;
};

class rti_object_info : public rti_object
{
public:
	rt_uint32_t type;
	rt_uint32_t id;
	rt_uint8_t  name[RTI_OBJECT_NAME_SIZE];
};

class rti_object_thread : public rti_object
{
public:
	rt_uint32_t id;
	rt_uint8_t  name[RTI_OBJECT_NAME_SIZE];
	rt_uint32_t stack_begin, stack_end;
	rt_uint32_t stack_ptr, stack_max;

	rt_uint8_t priority, status;
};

class rti_object_heapinfo : public rti_object
{
public:
	rt_uint32_t mem_total;
	rt_uint32_t max_used;
	rt_uint32_t current_used;
};

class rti_object_memdump : public rti_object
{
public:
	rt_uint8_t *mem;
	rt_uint32_t size;
};

class rti_object_log : public rti_object
{
public:
	rt_uint8_t *log;
	rt_uint32_t size;
};

#endif
