#include "rti.h"

/* transmit rti information */
void rti_tx(rt_uint8_t* ptr, rt_uint32_t size)
{
}

/* get time tick for rti */
rt_uint32_t rti_get_tick()
{
	return rt_tick_get();
}

void rti_stub_init()
{
}
