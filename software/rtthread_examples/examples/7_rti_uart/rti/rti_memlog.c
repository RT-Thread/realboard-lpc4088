#include <rthw.h>
#include <rtthread.h>
#include <stdio.h>

#define RTI_MEMLOG_CHUNK_NUM    16
#define RTI_MEMLOG_LINE_SIZE	80

static rt_uint8_t  rti_memlog_chunk[RTI_MEMLOG_CHUNK_NUM][RTI_MEMLOG_LINE_SIZE];
static rt_uint16_t rti_memlog_index = 0;
static rt_uint32_t rti_memlog_cnt = 0;

void rti_mem_log(const char* msg)
{
    rt_uint32_t level;
    rt_uint8_t* memlog_ptr;

	/* get mem log line buffer */
	level = rt_hw_interrupt_disable();
    memlog_ptr = &rti_memlog_chunk[rti_memlog_index][0];
    rti_memlog_cnt ++;

    rti_memlog_index ++;
    if (rti_memlog_index >= RTI_MEMLOG_CHUNK_NUM)
        rti_memlog_index = 0;
	rt_hw_interrupt_enable(level);

	sprintf ((char*)memlog_ptr, "%02x ", rti_memlog_cnt & 0xff);
	memlog_ptr += 3;

	rt_memcpy(memlog_ptr, msg, rt_strlen(msg));
}

void rti_mem_printf(const char* fmt, ...)
{
	va_list args;
    rt_uint32_t level;
    rt_uint8_t* memlog_ptr;

	va_start(args, fmt);

    /* get mem log line buffer */
	level = rt_hw_interrupt_disable();
    memlog_ptr = &rti_memlog_chunk[rti_memlog_index][0];
    rti_memlog_cnt ++;

    rti_memlog_index ++;
    if (rti_memlog_index >= RTI_MEMLOG_CHUNK_NUM)
        rti_memlog_index = 0;
	rt_hw_interrupt_enable(level);

	sprintf ((char*)memlog_ptr, "%02x ", rti_memlog_cnt & 0xff);
	memlog_ptr += 3;
	vsnprintf((char*)memlog_ptr, RTI_MEMLOG_LINE_SIZE - 3, fmt, args);

	va_end(args);
}

void rti_mem_dump()
{
	rt_uint32_t index;
	
	/* memlog dump */
	rt_kprintf("RealTime Target Insight Memlog Dump\n");
	for (index = rti_memlog_index; index < RTI_MEMLOG_CHUNK_NUM; index ++)
	{
		rt_kprintf("%s", &rti_memlog_chunk[index][0]);
	}
	
	for (index = 0; index < rti_memlog_index; index ++)
	{
		rt_kprintf("%s", &rti_memlog_chunk[index][0]);
	}
}
