/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009 RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      first implementation
 * 2010-02-04     Magicoe      ported to LPC17xx
 * 2010-05-02     Aozima       update CMSIS to 130
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "drv_uart.h"
#include "drv_lcd.h"
#ifdef LPC_EXT_SDRAM
#include "drv_sdram.h"
#include "drv_mpu.h"
#endif

#ifdef RT_USING_COMPONENTS_INIT
#include <components.h>
#endif

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * This function will initial LPC17xx board.
 */
void rt_hw_board_init()
{
    /* NVIC Configuration */
#define NVIC_VTOR_MASK              0x3FFFFF80
#ifdef  VECT_TAB_RAM
    /* Set the Vector Table base location at 0x10000000 */
    SCB->VTOR  = (0x10000000 & NVIC_VTOR_MASK);
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x00000000 */
    SCB->VTOR  = (0x00000000 & NVIC_VTOR_MASK);
#endif

    /* init systick */
    SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND - 1);
    /* set pend exception priority */
    NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

    /*init uart device*/
    rt_hw_uart_init();
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);

#if LPC_EXT_SDRAM == 1
    lpc_sdram_hw_init();
    mpu_init();
#endif

#ifdef RT_USING_COMPONENTS_INIT
    /* initialization board with RT-Thread Components */
    rt_components_board_init();
#endif
}

#ifdef RT_USING_RTGUI
#include "lpc_eeprom.h"
#include <rtgui/driver.h>
#include "drv_lcd.h"

/* initialize for gui driver */
int rtgui_lcd_init(void)
{
    rt_device_t device;

    rt_hw_lcd_init();

    device = rt_device_find("lcd");
    /* set graphic device */
    rtgui_graphic_set_device(device);

    return 0;
}
INIT_DEVICE_EXPORT(rtgui_lcd_init);

#include <rtgui/touch.h>
#include <rtgui/calibration.h>
#include "drv_touch.h"

#define CALIBRATION_DATA_MAGIC  0x55
#define CALIBRATION_DATA_PAGE   0x00
#define CALIBRATION_DATA_OFFSET 0x01

static rt_bool_t calibration_data_save(calculate_data_t *data)
{
    rt_uint8_t count;
    rt_uint8_t *buf;
    count = sizeof(calculate_data_t) + 1;
    buf = (rt_uint8_t *)rt_malloc(count);
    buf[0] = CALIBRATION_DATA_MAGIC;
    rt_memcpy(&buf[1], data, count - 1);
    /* power up the eeprom */
    EEPROM_PowerDown(DISABLE);
    /* erase the page for touch data */
    EEPROM_Erase(CALIBRATION_DATA_PAGE);
    EEPROM_Write(0, CALIBRATION_DATA_PAGE, buf, MODE_8_BIT, count);
    rt_free(buf);
    return RT_TRUE;
}

/* initialize for touch & calibration */
int touch_calibration_init(void)
{
    rt_uint8_t magic = 0;
    calculate_data_t data;
    struct rtgui_calibration_ops *ops;

    ops = calibration_get_ops();

    /* initialization the eeprom  on chip  */
    EEPROM_Init();
    /* initialization the touch driver */
    rtgui_touch_hw_init("spi10");
    /* set callback to save calibration data */
    calibration_set_after(calibration_data_save);
    /* initialization rtgui tonch server */
    rtgui_touch_init(ops);
    /* restore calibration data */
    EEPROM_Read(0, CALIBRATION_DATA_PAGE, &magic, MODE_8_BIT, 1);
    if (CALIBRATION_DATA_MAGIC != magic)
    {
        rt_kprintf("touch is not calibration,now calibration it please.\n");
        calibration_init();
    }
    else
    {
        EEPROM_Read(CALIBRATION_DATA_OFFSET, CALIBRATION_DATA_PAGE, &data, MODE_8_BIT, sizeof(calculate_data_t));
        calibration_set_data(&data);
    }
    /* power down the EEPROM */
    EEPROM_PowerDown(ENABLE);
    return 0;
}
INIT_APP_EXPORT(touch_calibration_init);
#endif

/* initialization for system heap */
int rt_hw_board_heap_init(void)
{
#ifdef RT_USING_HEAP
#if LPC_EXT_SDRAM
    rt_system_heap_init((void *)LPC_EXT_SDRAM_BEGIN, (void *)LPC_EXT_SDRAM_END);
    sram_init();
#else
    rt_system_heap_init((void *)HEAP_BEGIN, (void *)HEAP_END);
#endif
#endif

    return 0;
}

void MemManage_Handler(void)
{
    extern void HardFault_Handler(void);

    rt_kprintf("Memory Fault!\n");
    HardFault_Handler();
}
