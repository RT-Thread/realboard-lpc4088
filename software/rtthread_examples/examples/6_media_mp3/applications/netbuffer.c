#include <rthw.h>
#include <rtthread.h>

#include "netbuffer.h"

#define MP3_DECODE_MP_CNT   4
#define MP3_DECODE_MP_BLOCK_SZ    512*1024
#define MP3_DECODE_MP_SZ         (MP3_DECODE_MP_BLOCK_SZ*2+4)*4
static rt_uint8_t *mempool; // 256k x 4
static struct rt_mempool _mp;
static rt_bool_t is_inited = RT_FALSE;

rt_size_t sbuf_get_size()
{
    return MP3_DECODE_MP_BLOCK_SZ * 2;
}

void sbuf_init()
{
	  mempool=rt_malloc(MP3_DECODE_MP_SZ);
    rt_mp_init(&_mp, "mp3", mempool, MP3_DECODE_MP_SZ, MP3_DECODE_MP_BLOCK_SZ * 2);
}

void* sbuf_alloc()
{
	if (is_inited == RT_FALSE)
	{
		sbuf_init();
		is_inited = RT_TRUE;
	}

    return (rt_uint16_t*)rt_mp_alloc(&_mp, RT_WAITING_FOREVER);
}

void sbuf_release(void* ptr)
{
    rt_mp_free(ptr);
}
