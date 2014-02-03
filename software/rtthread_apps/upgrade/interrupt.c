#include <inttypes.h>

#if defined ( __GNUC__ ) /*------------------ GNU Compiler ---------------------*/

/* GNU gcc specific functions */

/** \brief  Enable IRQ Interrupts

  This function enables IRQ interrupts by clearing the I-bit in the CPSR.
  Can only be executed in Privileged modes.
 */
void rt_hw_interrupt_enable(uint32_t flag)
{
    __asm volatile ("MSR primask, %0" : : "r" (flag) : "memory");
}

/** \brief  Disable IRQ Interrupts

  This function disables IRQ interrupts by setting the I-bit in the CPSR.
  Can only be executed in Privileged modes.
 */
uint32_t rt_hw_interrupt_disable(void)
{
    uint32_t result;

    __asm volatile ("MRS %0, primask" : "=r" (result) );
    __asm volatile ("cpsid i" : : : "memory");

    return result;
}

#endif
