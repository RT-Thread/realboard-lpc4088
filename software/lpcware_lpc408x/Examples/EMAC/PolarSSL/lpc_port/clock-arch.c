#include "clock-arch.h"
#include "LPC407x_8x_177x_8x.h"
#include "lpc_clkpwr.h"

__IO clock_time_t Ticks;

#define LED_PIN     (1<<13)

/* SysTick timer interrupt  handler */
void SysTick_Handler (void)
{
    ++Ticks;
    if (!(Ticks & 0x07)){
        LPC_GPIO0->PIN ^= LED_PIN;
    }
}

/* Timer init */
void clock_init(void)
{
    Ticks = 0;
    // NXP: Initialize System tick timer
    // Generate interrupt each SYSTICK_PERIOD microsecond
    if (SysTick_Config((SystemCoreClock/CLOCK_CONF_SECOND))){
        // Capture error
        while (1);
    }
}

/* returned The current clock time, measured in system ticks */
clock_time_t clock_time(void)
{
  return(Ticks);
}
