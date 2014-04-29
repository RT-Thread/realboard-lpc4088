#ifndef __DRV_DMA_H
#define __DRV_DMA_H
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

typedef struct 
{
 rt_uint32_t error_count;
}lpc408x_dma_t;
void rt_hw_dma_init(void);

#endif
