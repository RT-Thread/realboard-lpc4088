#include "rti.h"
#include "rti_pdu.h"
#include "rti_cmd.h"
#include <board.h>

#define LPC_SC_PCONP_PCTIM0 0x02UL
#define LPC_SC_PCONP_PCTIM1 0x04UL
#define LPC_SC_PCONP_PCTIM2 (0x01UL<<22)
#define LPC_SC_PCONP_PCTIM3 (0x01UL<<23)
/* register B definitions */
/* IR */
# define TIM_IR_MR0         0x01UL
# define TIM_IR_MR1         0x02UL
# define TIM_IR_MR2         0x04UL
# define TIM_IR_MR3         0x08UL
# define TIM_IR_CR0         0x10UL
# define TIM_IR_CR1         0x20UL
/* TCR */
# define TIM_TCR_CNT_EN     0x01UL
# define TIM_TCR_CNT_RST    0x02UL
/* MCR */
# define TIM_MCR_MR0I       0x01UL
# define TIM_MCR_MR0R       0x02UL
# define TIM_MCR_MR0S       0x04UL
# define TIM_MCR_MR1I       0x08UL
# define TIM_MCR_MR1R       0x10UL
# define TIM_MCR_MR1S       0x20UL
# define TIM_MCR_MR2I       0x40UL
# define TIM_MCR_MR2R       0x80UL
# define TIM_MCR_MR2S       0x100UL
# define TIM_MCR_MR3I       0x200UL
# define TIM_MCR_MR3R       0x400UL
# define TIM_MCR_MR3S       0x800UL


#ifdef RT_USING_RTI

struct rti_setting setting;
static rt_uint32_t ts_tick=0;



void TIMER3_IRQHandler(void)
{
	 rt_interrupt_enter();
   ts_tick ++;
   LPC_TIM3->IR |= TIM_IR_MR0;         /* clear interrupt */
	 rt_interrupt_leave();
}
/* get time tick for rti */
rt_uint32_t rti_get_tick(void)
{
	return ts_tick;
}

void rti_no_buffer(void)
{
	LPC_GPIO4->CLR=0x01<<16;
}

void rti_has_buffer(void)
{
	LPC_GPIO4->SET=0x01<<16;
}

void rti_stub_init(void)
{

	/* TIM3 configuration */
	{
	
    LPC_SC->PCONP |= LPC_SC_PCONP_PCTIM3;           /* power on timer */
    LPC_TIM3->TCR = TIM_TCR_CNT_RST;                /* reset counter */
    LPC_TIM3->TCR = 0;                              /* release reset */
    LPC_TIM3->TCR = TIM_TCR_CNT_EN;                 /* enable counter */
    LPC_TIM3->IR = 0x3F;                            /* clear interrupts */
    LPC_TIM3->PR = 0;
    LPC_TIM3->TC = 0;
    LPC_TIM3->PC = 0;
    LPC_TIM3->MR0 =  PeripheralClock/10000-1;
    LPC_TIM3->MCR |= TIM_MCR_MR0I | TIM_MCR_MR0R;   /* enable match interrupt + reset */

    NVIC_SetPriority(TIMER3_IRQn, (0x01 < 3) | (0x01));   /* set priority in NVIC */
    NVIC_EnableIRQ(TIMER3_IRQn);                    /* enable interrupt in NVIC */
	}

	    /* init rti . */
	 {
		 setting.chunk_num    = RTI_CHUNK_NUM;
		 setting.chunk_size   = 2048;
		 setting.chunk_memory = rt_malloc(setting.chunk_num * setting.chunk_size);

		 setting.device       = "uart2";
		 setting.time_unit    = 10000;
		 setting.name_size    = 8;
		 setting.frame_time   = 10000;
		 setting.filter       = 0;

		 rti_init(&setting);
   }
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(rti_stub_init, startup realtime target insight);

void ts_test(void)
{
	rt_uint32_t tick;

	tick = rti_get_tick();
	rt_thread_delay(1);
	tick = rti_get_tick() - tick;
	rt_kprintf("1 tick = %d rti tick\n", tick);

	tick = rti_get_tick();
	rt_thread_delay(10);
	tick = rti_get_tick() - tick;
	rt_kprintf("10 tick = %d rti tick\n", tick);

	tick = rti_get_tick();
	rt_thread_delay(100);
	tick = rti_get_tick() - tick;
	rt_kprintf("100 tick = %d rti tick\n", tick);
}
FINSH_FUNCTION_EXPORT(ts_test, startup realtime target insight);
#endif
#endif
