#include <board.h>
#include "drv_lcd.h"

#include <lpc_clkpwr.h>
#include <lpc_lcd.h>
#include <lpc_pinsel.h>
#include <lpc_i2c.h>
#include <lpc_timer.h>
#include <lpc_pwm.h>
#include <lpc_adc.h>

//#define FRAME_BUFFER      0xA0000000
static  rt_uint16_t * _rt_framebuffer = RT_NULL;
#define RT_HW_LCD_WIDTH		480
#define RT_HW_LCD_HEIGHT	272

static struct rt_device_graphic_info _lcd_info;
static struct rt_device  lcd;

/* LCD Config */
#define LCD_H_SIZE           480
#define LCD_H_PULSE          2
#define LCD_H_FRONT_PORCH    5
#define LCD_H_BACK_PORCH     40
#define LCD_V_SIZE           272
#define LCD_V_PULSE          2
#define LCD_V_FRONT_PORCH    8
#define LCD_V_BACK_PORCH     8


#define LCD_PIX_CLK          (8*1000000UL)

//#define LCD_CLK_PER_LINE     (LCD_H_SIZE + LCD_H_PULSE + LCD_H_FRONT_PORCH + LCD_H_BACK_PORCH)
//#define LCD_LINES_PER_FRAME  (LCD_V_SIZE + LCD_V_PULSE + LCD_V_FRONT_PORCH + LCD_V_BACK_PORCH)
//#define LCD_PIX_CLK          (LCD_CLK_PER_LINE * LCD_LINES_PER_FRAME)



static void lcd_gpio_init(void)
{
  LPC_IOCON->P2_12 	= 0x05;	// 配置P2_12为VD3, 	R0
	LPC_IOCON->P2_6 	= 0x07;	// 配置P2_6为VD4,		R1
	LPC_IOCON->P0_10 	= 0x07;	// 配置P0_10为VD5,		R2
	LPC_IOCON->P4_28	= 0x05;	// 配置P4_28为VD6,		R3
	LPC_IOCON->P4_29 = 0x05;	// 配置P4_29为VD7,		R4

	LPC_IOCON->P1_20	= 0x07;	// 配置P1_20为VD10,	G0
	LPC_IOCON->P1_21	= 0x07;	// 配置P1_21为VD11,	G1
	LPC_IOCON->P1_22	= 0x07;	// 配置P1_22为VD12,	G2
	LPC_IOCON->P1_23	= 0x07;	// 配置P1_23为VD13,	G3
	LPC_IOCON->P1_24	= 0x07;	// 配置P1_24为VD14,	G4
	LPC_IOCON->P1_25	= 0x07;	// 配置P1_25为 VD15,	G5
	
	LPC_IOCON->P2_13	= 0x07;	// 配置P2_13为VD19,	B0
	LPC_IOCON->P1_26 = 0x07;	// 配置P1_26为VD20,	B1
	LPC_IOCON->P1_27 = 0x07;	// 配置P1_27为VD21,	B2
	LPC_IOCON->P1_28 = 0x07;	// 配置P1_28为VD22,	B3
	LPC_IOCON->P1_29 = 0x07;	// 配置P1_29为VD23,	B4
	
	LPC_IOCON->P2_2	= 0x07;	  // 配置P2_2为LCD的DCLK
	LPC_IOCON->P2_0	= 0x07;	  // 配置P2_0为LCD的DSIP(power)
	LPC_IOCON->P2_5	= 0x07;	  // 配置P2_5为LCD的HSYNC
	LPC_IOCON->P2_3	= 0x07;	  // 配置P2_3为LCD的VSYNC
	
	
	LPC_IOCON->P2_1	= 0x07;	  // 配置P2_1为LCD的背光控制GPIO
	LPC_IOCON->P2_4	= 0x00;	  // 配置P2_4为LCD的DataEn
	
	LPC_GPIO2->DIR|=1<<4;     //配置P2_4为输出
	LPC_GPIO2->SET|=1<<4;     //打开LCD背光
}

static rt_uint32_t find_clock_divisor(rt_uint32_t clock) {
  rt_uint32_t Divider;
  rt_uint32_t r;

  Divider = 1;
  while (((SystemCoreClock / Divider) > clock) && (Divider <= 0x3F)) {
    Divider++;
  }
  if (Divider <= 1) {
    r = (1 << 26);  // Skip divider logic if clock divider is 1
  } else {
    Divider -= 2;
    r = 0
        | (((Divider >> 0) & 0x1F)
        | (((Divider >> 5) & 0x1F) << 27))
        ;
  }
  return r;
}
/* RT-Thread Device Interface */
static rt_err_t rt_lcd_init (rt_device_t dev)
{

	/*LCD管脚配置*/
  lcd_gpio_init();
	 /*打开LCD的控制时钟*/
	LPC_SC ->PCONP |=0x01;
	 /*禁止屏幕光标*/
	LPC_LCD->CRSR_CTRL &=~(1<<0);
	/*失能LCD控制器*/ 	
	LPC_LCD->CTRL &= ~(1UL << 0); 
	/*配置LCD工作在565模式*/
	LPC_LCD->CTRL&=~(0x07<1);     //将寄存器的显示模式清零，CTRL的[3:1]位
	LPC_LCD->CTRL|=(0x06<<1);     //565模式为110
	/*设置LCD屏幕为TFT类型*/
	LPC_LCD->CTRL |= (1<<5);
	/*设置LCD为单面板*/
	LPC_LCD->CTRL &= ~(1<<7);
	/*设置为BGR*/ 
	LPC_LCD->CTRL |= (1<<8);
	/*设置为小端字节序*/
	LPC_LCD->CTRL &= ~(1<<9);
	// little endian pix order
	LPC_LCD->CTRL &= ~(1<<10);
	// disable power
	LPC_LCD->CTRL &= ~(1<<11);

	LPC_LCD->POL=(find_clock_divisor(LCD_PIX_CLK) <<  0);
	// clock source for the LCD block is HCLK
	LPC_LCD->POL &= ~(1<<5);
	// LCDFP pin is active LOW and inactive HIGH
	LPC_LCD->POL |= (1<<11);
	// LCDLP pin is active LOW and inactive HIGH
	LPC_LCD->POL |= (1<<12);
	// data is driven out into the LCD on the falling edge
	LPC_LCD->POL &= ~(1<<13);
	// active high
	LPC_LCD->POL &= ~(1<<14);
	LPC_LCD->POL &= ~(0x3FF <<16);
	LPC_LCD->POL |= (LCD_H_SIZE-1)<<16;
	
	//配置水平同步信号时序
	LPC_LCD->TIMH = 0;                            //清空水平同步配置寄存器
	LPC_LCD->TIMH |= (LCD_H_BACK_PORCH - 1)<<24;  //设置水平同步后端
	LPC_LCD->TIMH |= (LCD_H_FRONT_PORCH - 1)<<16; //设置水平同步前端
	LPC_LCD->TIMH |= (LCD_H_PULSE - 1)<<8;        //设置水平脉冲宽度
	LPC_LCD->TIMH |= ((LCD_H_SIZE/16) - 1)<<2;    //设置水平像素点
	
	//配置垂直同步信号时序
	LPC_LCD->TIMV = 0;  //reset TIMV value before setting
	LPC_LCD->TIMV |= (LCD_V_BACK_PORCH)<<24;
	LPC_LCD->TIMV |= (LCD_V_FRONT_PORCH)<<16;
	LPC_LCD->TIMV |= (LCD_V_PULSE - 1)<<10;
	LPC_LCD->TIMV |= LCD_V_SIZE - 1;

               ;
  LPC_SC->LCD_CFG = 0x0;  // No panel clock prescaler
  //
  // Enable LCDC
  //
  LPC_LCD->UPBASE  =(rt_uint32_t)_rt_framebuffer;
  LPC_LCD->CTRL   |= (1 <<  0);                 // Enable LCD signals
  LPC_LCD->CTRL   |= (1 << 11);                 // Enable LCD power
	
	return RT_EOK;
}

static rt_err_t rt_lcd_control (rt_device_t dev, rt_uint8_t cmd, void *args)
{
	switch (cmd)
	{
	case RTGRAPHIC_CTRL_RECT_UPDATE:
		break;
	case RTGRAPHIC_CTRL_POWERON:
		break;
	case RTGRAPHIC_CTRL_POWEROFF:
		break;
	case RTGRAPHIC_CTRL_GET_INFO:
		rt_memcpy(args, &_lcd_info, sizeof(_lcd_info));
		break;
	case RTGRAPHIC_CTRL_SET_MODE:
		break;
	}

	return RT_EOK;
}




void lcd_clear(rt_uint16_t color)
{
	volatile rt_uint16_t *p = (rt_uint16_t *)_lcd_info.framebuffer;
	int x, y;
	
	for (y = 0; y <= RT_HW_LCD_HEIGHT; y++)
	{
		for (x = 0; x <= RT_HW_LCD_WIDTH; x++)
		{
			*p++ =color;  /* red */
		}
	}
}

void rt_hw_lcd_init(void)
{


	_rt_framebuffer = rt_malloc_align(sizeof(rt_uint16_t)*RT_HW_LCD_HEIGHT*RT_HW_LCD_WIDTH, 32);
  if (_rt_framebuffer == RT_NULL) return; /* no memory yet */
  //_rt_framebuffer = (rt_uint16_t *)FRAME_BUFFER;
	_lcd_info.bits_per_pixel = 16;
	_lcd_info.pixel_format = RTGRAPHIC_PIXEL_FORMAT_RGB565;
	_lcd_info.framebuffer = (void*)_rt_framebuffer;
	_lcd_info.width = RT_HW_LCD_WIDTH;
	_lcd_info.height = RT_HW_LCD_HEIGHT;
  
	/* init device structure */
	lcd.type = RT_Device_Class_Graphic;
	lcd.init = rt_lcd_init;
	lcd.open = RT_NULL;
	lcd.close = RT_NULL;
	lcd.control = rt_lcd_control;
	lcd.user_data = (void*)&_lcd_info;

	/* register lcd device to RT-Thread */
	rt_device_register(&lcd, "lcd", RT_DEVICE_FLAG_RDWR);
	
	rt_lcd_init(&lcd);
}


void lcd_test(void)
{
  lcd_clear(0xf800);
	rt_thread_delay(200);
	lcd_clear(0x07e0);
	rt_thread_delay(200);
	lcd_clear(0x001f);
	rt_thread_delay(200);
}
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(lcd_clear, lcd_clear  );
FINSH_FUNCTION_EXPORT(lcd_test, lcd_test  );
#endif
