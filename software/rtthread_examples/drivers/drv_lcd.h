#ifndef __DRV_LCD_H__
#define __DRV_LCD_H__

void rt_hw_lcd_init(void);
void cursor_set_position(rt_uint32_t x, rt_uint32_t y);
void cursor_display(rt_bool_t enable);
#endif

