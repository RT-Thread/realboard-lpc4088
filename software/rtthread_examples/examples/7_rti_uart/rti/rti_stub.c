#include "rti.h"

/* get time tick for rti */
rt_uint32_t rti_get_tick()
{
	return rt_tick_get();
}

void rti_stub_init()
{
}

void rti_has_buffer()
{
}

void rti_no_buffer()
{
}
