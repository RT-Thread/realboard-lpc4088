/*
 * File      : board.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-09-22     Bernard      add board.h to this bsp
 * 2010-02-04     Magicoe      add board.h to LPC176x bsp
 * 2013-12-18     Bernard      porting to LPC4088 bsp
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include "LPC407x_8x_177x_8x.h"
#include <rtthread.h>

/* disable SDRAM in default */
#ifndef LPC_EXT_SDRAM
#define LPC_EXT_SDRAM       0
#endif

// <RDTConfigurator URL="http://www.rt-thread.com/eclipse">

// <integer name="LPC_EXT_SDRAM" description="Begin Address of External SDRAM" default="0xA0000000" />
#define LPC_EXT_SDRAM_BEGIN    0xA0000000
// <integer name="LPC_EXT_SDRAM_END" description="End Address of External SDRAM" default="0xA2000000" />
#define LPC_EXT_SDRAM_END      0xA2000000

// <bool name="RT_USING_UART0" description="Using UART0" default="true" />
#define RT_USING_UART0
// <bool name="RT_USING_UART1" description="Using UART1" default="true" />
//#define RT_USING_UART1
// <bool name="RT_USING_UART2" description="Using UART2" default="true" />
#define RT_USING_UART2
// <string name="RT_CONSOLE_DEVICE_NAME" description="The name of console device" default="" />
#define RT_CONSOLE_DEVICE_NAME	"uart0"

// </RDTConfigurator>

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN  ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define HEAP_BEGIN  (__segment_end("HEAP"))
#else
extern int __bss_end;
#define HEAP_BEGIN  ((void *)&__bss_end)
#endif
#define HEAP_END    (void*)(0x10000000 + 0x10000)

void rt_hw_board_init(void);

#endif
