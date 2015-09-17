#ifndef __RTI_H__
#define __RTI_H__

#include <rtthread.h>

#define RTI_USING_SERVER

/*
 * RealTime Target Information
 */
#ifndef RTI_CHUNK_NUM
#define RTI_CHUNK_NUM	64
#endif

struct rti_setting
{
	const char* device;		/* rti output device name	*/

	rt_uint16_t chunk_size;		/* chunk size in rti		*/
	rt_uint8_t  chunk_num;		/* number of chunk in rti	*/
	rt_uint8_t  name_size;		/* size of object name		*/
	rt_uint8_t* chunk_memory;	/* chunk memory buffer		*/

	rt_uint32_t time_unit;		/* unit time of rti in ms	*/
	rt_uint32_t frame_time;		/* time length of one frame */
	rt_uint32_t filter;			/* record filter			*/
};

void rti_init(const struct rti_setting *setting);
#ifdef RTI_USING_SERVER
void rti_server_init(const char* device);
#endif

rt_uint32_t rti_get_tick(void);
rt_uint32_t rti_get_filter(void);

rt_uint8_t *rti_get_buffer(rt_size_t length);
void rti_put_chunk(rt_uint8_t *chunk);

void rti_no_buffer(void);
void rti_has_buffer(void);

void rti_get_sysinfo(void);

/* rti command */
void rti_tx(rt_uint8_t* ptr, rt_uint32_t size);

#endif
