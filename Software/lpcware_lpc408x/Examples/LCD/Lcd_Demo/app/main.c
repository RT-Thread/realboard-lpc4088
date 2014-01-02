/*************************************************************************
 *
*    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : main.c
 *    Description : Main module
 *
 *    History :
 *    1. Date        : 4, August 2008
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *  This example project shows how to use the IAR Embedded Workbench for ARM
 * to develop code for the IAR LPC-1788 board. It shows basic use of the I/O,
 * the timer and the interrupt controllers.
 *  It starts by blinking USB Link LED.
 *
 * Jumpers:
 *  EXT/JLINK  - depending of power source
 *  ISP_E      - unfilled
 *  RST_E      - unfilled
 *  BDS_E      - unfilled
 *  C/SC       - SC
 *
 * Note:
 *  After power-up the controller get clock from internal RC oscillator that
 * is unstable and may fail with J-Link auto detect, therefore adaptive clocking
 * should always be used. The adaptive clock can be select from menu:
 *  Project->Options..., section Debugger->J-Link/J-Trace  JTAG Speed - Adaptive.
 *
 *    $Revision: 24636 $
 **************************************************************************/

#include "lcd_config.h"
#include "lpc_types.h"
#include "system_LPC407x_8x_177x_8x.h"
#include "lpc_clkpwr.h"
#include "lpc_timer.h"
#include "lpc_pinsel.h"
#include "lpc_lcd.h"
#include "lpc_pwm.h"
#include "lpc_adc.h"
#include "bsp.h"
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
#include "sdram_k4s561632j.h"
#elif (_CURR_USING_BRD == _QVGA_BOARD)
#include "sdram_mt48lc8m32lfb5.h"
#elif (_CURR_USING_BRD == _EA_PA_BOARD)
#include "sdram_is42s32800d.h"
#endif

/** @defgroup LCD_Demo  LCD Demo
 * @ingroup LCD_Examples
 * @{
 */

uint8_t Smb380Id, Smb380Ver;
extern uint8_t * LogoStream;
#if (LOGO_BPP == 2)
extern uint8_t * LogoPalette;
#endif

Bmp_t LogoPic =
{
  320,
  240,
  LOGO_BPP,
  BMP_BYTES_PP,
  #if (LOGO_BPP == 2)
  (uint8_t *)&LogoPalette,
  #else
  NULL,
  #endif
  (uint8_t *)&LogoStream,
  ( uint8_t *)"Logos picture"
};


void DelayMS(uint32_t dly)
{
    volatile uint32_t i = 0;

    for ( ; dly > 0; dly--)
        for (i = 0; i < 16000; i++);
}
/*************************************************************************
 * Function Name: SetBackLight
 * Parameters: level     Backlight value
 *
 * Return: none
 *
 * Description: Set LCD backlight
 *
 *************************************************************************/
void SetBackLight(uint32_t level)
{ 
#if ((_CUR_USING_LCD == _RUNNING_LCD_QVGA_TFT)||(_CUR_USING_LCD == _RUNNING_LCD_GFT035A320240Y))
  PWM_MATCHCFG_Type PWMMatchCfgDat;
  PWM_MatchUpdate(_PWM_NO_USED, _PWM_CHANNEL_NO, level, PWM_MATCH_UPDATE_NOW);
  PWMMatchCfgDat.IntOnMatch = DISABLE;
  PWMMatchCfgDat.MatchChannel = _PWM_CHANNEL_NO;
  PWMMatchCfgDat.ResetOnMatch = DISABLE;
  PWMMatchCfgDat.StopOnMatch = DISABLE;
  PWM_ConfigMatch(_PWM_NO_USED, &PWMMatchCfgDat);


  /* Enable PWM Channel Output */
  PWM_ChannelCmd(_PWM_NO_USED, _PWM_CHANNEL_NO, ENABLE);

  /* Reset and Start counter */
  PWM_ResetCounter(_PWM_NO_USED);

  PWM_CounterCmd(_PWM_NO_USED, ENABLE);

  /* Start PWM now */
  PWM_Cmd(_PWM_NO_USED, ENABLE);
#else
{
  SetPWM(level*100/0x1000);
}
#endif /*((_CURR_USING_BRD == _IAR_OLIMEX_BOARD) ||(_CURR_USING_BRD == _RUNNING_LCD_QVGA_TFT))*/
}

/*************************************************************************
 * Function Name: GetBacklightVal
 * Parameters: none
 *
 * Return: none
 *
 * Description: Get backlight value from user
 *
 *************************************************************************/
uint32_t GetBacklightVal (void) {
  uint32_t val;
  uint32_t backlight_off, pclk;

  ADC_StartCmd(LPC_ADC, ADC_START_NOW);

  while (!(ADC_ChannelGetStatus(LPC_ADC, BRD_ADC_PREPARED_CHANNEL, ADC_DATA_DONE)));

  val = ADC_ChannelGetData(LPC_ADC, BRD_ADC_PREPARED_CHANNEL);
#if ((_CUR_USING_LCD == _RUNNING_LCD_QVGA_TFT)||(_CUR_USING_LCD == _RUNNING_LCD_GFT035A320240Y))
  val = (val >> 7) & 0x3F;
  pclk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
  backlight_off = pclk/(_BACK_LIGHT_BASE_CLK*20);
  val =  val* (pclk*9/(_BACK_LIGHT_BASE_CLK*20))/0x3F;
#else
  backlight_off = 0;
#endif
  return backlight_off + val;
}
/*************************************************************************
 * Function Name: lcd_colorbars
 * Parameters: none
 *
 * Return: none
 *
 * Description: Draw color bar on screen
 *
 *************************************************************************/
void lcd_colorbars(void)
{
  LcdPixel_t color = 0;
  UNS_16 curx = 0, cury = 0, ofsx, ofsy;
  uint16_t red,green,blue;
  uint16_t maxclr;
 
  #if (LOGO_BPP == 24)
  maxclr = 256;
  #elif (LOGO_BPP == 16)
  maxclr = 32;
  #else
  maxclr = 4;
  #endif
  ofsx = (LCD_H_SIZE + maxclr - 1)/maxclr;
  ofsy = LCD_V_SIZE/3;
  
  #if (LOGO_BPP >= 8)
  // Red bar
  green = blue = 0;
  for(red = 0; red < maxclr; red++)
  {
      color = MAKE_COLOR(red,green,blue);
      LCD_FillRect (LCD_PANEL_UPPER,curx, curx+ofsx, cury, cury+ofsy, color);
      curx+=ofsx;
      if(curx>= LCD_H_SIZE)
      {
        curx = 0;
        break;
      }
  }
  
   // green bar
  red = blue = 0;
  curx = 0;
  cury += ofsy;
  for(green = 0; green < maxclr; green++)
  {
      color = MAKE_COLOR(red,green,blue);
      LCD_FillRect (LCD_PANEL_UPPER,curx, curx+ofsx, cury, cury+ofsy, color);
      curx+=ofsx;
      if(curx>= LCD_H_SIZE)
      {
        curx = 0;
        break;
      }
  }
  
  // blue bar
  green = blue = 0;
  curx = 0;
  cury += ofsy;
  for(blue = 0; blue < maxclr; blue++)
  {
      color = MAKE_COLOR(red,green,blue);
      LCD_FillRect (LCD_PANEL_UPPER,curx, curx+ofsx, cury, cury+ofsy, color);
      curx+=ofsx;
      if(curx>= LCD_H_SIZE)
      {
        curx = 0;
        break;
      }
  }
 #else
  for(color = 0; color < maxclr; color++)
  {
      LCD_FillRect (LCD_PANEL_UPPER,curx, curx+ofsx, cury, cury+LCD_V_SIZE-1, color);
      curx+=ofsx;
      if(curx>= LCD_H_SIZE)
      {
        curx = 0;
        break;
      }
  }
 #endif    
}

/*************************************************************************
 * Function Name: c_entry
 * Parameters: none
 *
 * Return: none
 *
 * Description: entry
 *
 *************************************************************************/
  void c_entry(void)
{
  volatile uint32_t i;
  uint32_t pclk;
#if LOGO_DISPLAYED
  uint32_t xs, ys;
#endif
#if ACCEL_SENSOR_USED
  AccSensor_Data_t XYZT;
#endif
  uint32_t backlight;
  PWM_TIMERCFG_Type PWMCfgDat;
  PWM_MATCHCFG_Type PWMMatchCfgDat;
  LCD_Cursor_Config_Type cursor_config;
  LCD_Config_Type lcd_config;
#if PAINT_ON_SCREEN  
  int cursor_x = 0, cursor_y = 0;
#endif  
  int draw_cursor_x, draw_cursor_y;
#if LOGO_DISPLAYED
  uint32_t start_pix_x, start_pix_y, pix_ofs;
#endif
#if TCS_USED
  TSC2046_Init_Type tsc_config;
#endif
  
  /***************/
  /** Initialize ADC */
  /***************/
  PINSEL_ConfigPin (BRD_ADC_PREPARED_CH_PORT,
                    BRD_ADC_PREPARED_CH_PIN,
                    BRD_ADC_PREPARED_CH_FUNC_NO);
  PINSEL_SetAnalogPinMode(BRD_ADC_PREPARED_CH_PORT,BRD_ADC_PREPARED_CH_PIN,ENABLE);

  ADC_Init(LPC_ADC, 400000);
  ADC_IntConfig(LPC_ADC, BRD_ADC_PREPARED_INTR, DISABLE);
  ADC_ChannelCmd(LPC_ADC, BRD_ADC_PREPARED_CHANNEL, ENABLE);

  /***************/
  /** Initialize LCD */
  /***************/
  LCD_Enable (FALSE);
    
  // SDRAM Init = check right board to avoid linking error
  SDRAMInit();
  
  lcd_config.big_endian_byte = 0;
  lcd_config.big_endian_pixel = 0;
  lcd_config.hConfig.hbp = LCD_H_BACK_PORCH;
  lcd_config.hConfig.hfp = LCD_H_FRONT_PORCH;
  lcd_config.hConfig.hsw = LCD_H_PULSE;
  lcd_config.hConfig.ppl = LCD_H_SIZE;
  lcd_config.vConfig.lpp = LCD_V_SIZE;
  lcd_config.vConfig.vbp = LCD_V_BACK_PORCH;
  lcd_config.vConfig.vfp = LCD_V_FRONT_PORCH;
  lcd_config.vConfig.vsw = LCD_V_PULSE;
  lcd_config.panel_clk   = LCD_PIX_CLK;
  lcd_config.polarity.active_high = 1;
  lcd_config.polarity.cpl = LCD_H_SIZE;
  lcd_config.polarity.invert_hsync = 1;
  lcd_config.polarity.invert_vsync = 1;
  #if (_CUR_USING_LCD ==_RUNNING_LCD_EA_REV_PB1)
  lcd_config.polarity.invert_panel_clock = 0;
  #else
  lcd_config.polarity.invert_panel_clock = 1;
  #endif
  lcd_config.lcd_panel_upper =  LCD_VRAM_BASE_ADDR_UPPER;
  lcd_config.lcd_panel_lower =  LCD_VRAM_BASE_ADDR_LOWER;
  #if (LOGO_BPP == 24)
  lcd_config.lcd_bpp = LCD_BPP_24;
  #elif (LOGO_BPP == 16)
  lcd_config.lcd_bpp = LCD_BPP_16;
  #elif (LOGO_BPP == 2)
  lcd_config.lcd_bpp = LCD_BPP_2;
  #else
  while(1);
  #endif
  lcd_config.lcd_type = LCD_TFT;
  lcd_config.lcd_palette = LogoPic.pPalette;
  lcd_config.lcd_bgr = FALSE;
  
  #if ((_CUR_USING_LCD == _RUNNING_LCD_QVGA_TFT)||(_CUR_USING_LCD == _RUNNING_LCD_EA_REV_PB1))
  LCD_Init (&lcd_config);
  InitLcdController();
  #else
  LCD_Init (&lcd_config);
  #endif

  #if TCS_USED
  tsc_config.ad_left = TOUCH_AD_LEFT;
  tsc_config.ad_right = TOUCH_AD_RIGHT;
  tsc_config.ad_top = TOUCH_AD_TOP;
  tsc_config.ad_bottom = TOUCH_AD_BOTTOM;
  tsc_config.lcd_h_size = LCD_H_SIZE;
  tsc_config.lcd_v_size = LCD_V_SIZE;
  #if (_CUR_USING_LCD == _RUNNING_LCD_QVGA_TFT)
  tsc_config.swap_xy = 1;
  #else
  tsc_config.swap_xy = 1;
  #endif
  InitTSC2046(&tsc_config);
  #endif

  LCD_SetImage(LCD_PANEL_UPPER, NULL);
  LCD_SetImage(LCD_PANEL_LOWER, NULL);
#if ((_CURR_USING_BRD == _IAR_OLIMEX_BOARD) ||(_CURR_USING_BRD == _RUNNING_LCD_QVGA_TFT)) 
   /***************/
  /* Initialize PWM */
  /***************/
  PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
  PWMCfgDat.PrescaleValue = 1;
  PWM_Init(_PWM_NO_USED, PWM_MODE_TIMER, (void *) &PWMCfgDat);

  PINSEL_ConfigPin (_PWM_PORT_NUM, _PWM_PIN_NUM, _PWM_PIN_FUNC_NUM);
  PWM_ChannelConfig(_PWM_NO_USED, _PWM_CHANNEL_NO, PWM_CHANNEL_SINGLE_EDGE);

  // Set MR0
  pclk = CLKPWR_GetCLK(CLKPWR_CLKTYPE_PER);
  PWM_MatchUpdate(_PWM_NO_USED, 0,pclk/_BACK_LIGHT_BASE_CLK, PWM_MATCH_UPDATE_NOW);
  PWMMatchCfgDat.IntOnMatch = DISABLE;
  PWMMatchCfgDat.MatchChannel = 0;
  PWMMatchCfgDat.ResetOnMatch = ENABLE;
  PWMMatchCfgDat.StopOnMatch = DISABLE;
  PWM_ConfigMatch(_PWM_NO_USED, &PWMMatchCfgDat);
#endif

  // Set backlight
  backlight = GetBacklightVal();
  SetBackLight(backlight);

  // Enable LCD
  LCD_Enable (TRUE);
  
#if LOGO_DISPLAYED
  if(LogoPic.H_Size > LCD_H_SIZE)
  {
    start_pix_x = (LogoPic.H_Size - LCD_H_SIZE)/2; 
    xs = 0;
  }
  else
  {
    start_pix_x = 0;
    xs = (LCD_H_SIZE - LogoPic.H_Size)/2;
  }
  
  if(LogoPic.V_Size > LCD_V_SIZE)
  {
     start_pix_y = (LogoPic.V_Size - LCD_V_SIZE)/2;
     ys = 0;
  }
  else 
  {
     ys = (LCD_V_SIZE - LogoPic.V_Size)/2;
     start_pix_y = 0;
  }
  
  pix_ofs = (start_pix_y * LogoPic.H_Size + start_pix_x)*LogoPic.BitsPP/8;
  LogoPic.pPicStream += pix_ofs;
  LCD_LoadPic(LCD_PANEL_UPPER,xs,ys,&LogoPic,0x00);
  
  DelayMS(2000);
#endif /*LOGO_DISPLAYED*/ 

  // Draw color bars
  lcd_colorbars();
  
  // Draw cursor
  LCD_Cursor_Enable(DISABLE, 0);

  cursor_config.baseaddress = LCD_CURSOR_BASE_ADDR;
  cursor_config.framesync = 1;
#if (CURSOR_SIZE == 64)
  cursor_config.size32 = 0;
#else
  cursor_config.size32 = 1;
#endif
  cursor_config.palette[0].Red = 0x00;
  cursor_config.palette[0].Green = 0x00;
  cursor_config.palette[0].Blue = 0x00;
  cursor_config.palette[1].Red = 0xFF;
  cursor_config.palette[1].Green = 0xFF;
  cursor_config.palette[1].Blue = 0xFF;
  LCD_Cursor_Cfg(&cursor_config);
  LCD_Cursor_SetImage((uint32_t *)Cursor, 0, sizeof(Cursor)/sizeof(uint32_t)) ;

  draw_cursor_x = (LCD_H_SIZE/2) - CURSOR_OFF_X;
  draw_cursor_y = (LCD_V_SIZE/2) - CURSOR_OFF_Y;
  LCD_Move_Cursor(draw_cursor_x, draw_cursor_y);

  LCD_Cursor_Enable(ENABLE, 0);

  /* Initialize accellation sensor */
#if ACCEL_SENSOR_USED
  AccSensor_Init();
  for(i = 0; i < 0x100000;  i++);
#endif

#if ((_CURR_USING_BRD == _IAR_OLIMEX_BOARD) && ACCEL_SENSOR_USED)
  SMB380_GetID(&Smb380Id, &Smb380Ver);
#endif
  
  while(1)
  {
    
    backlight = GetBacklightVal();
    SetBackLight(backlight);

#if TCS_USED || ACCEL_SENSOR_USED

#if TCS_USED
    {
        int16_t tmp_x = -1, tmp_y = -1;
        for(i = 0; i < 0x10000;  i++);
        GetTouchCoord((int16_t*)&tmp_x, (int16_t*)&tmp_y);
        if((tmp_x >= 0) && (tmp_y >0))
        {    
#if PAINT_ON_SCREEN            
          cursor_x = tmp_x;
          cursor_y = tmp_y;  
#endif          
          draw_cursor_x = tmp_x - CURSOR_OFF_X;
          draw_cursor_y = tmp_y - CURSOR_OFF_Y;
        }
        else
        {
            continue;
        }
        
    }
#elif ACCEL_SENSOR_USED
    for(i = 0; i < 0x10000;  i++);
    AccSensor_GetData (&XYZT);
    if((XYZT.AccX == 0) && (XYZT.AccY == 0))
      continue; 
       
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
    draw_cursor_x += XYZT.AccX/512;
    draw_cursor_y += XYZT.AccY/512;
#else
    {
       MMA7455_Orientation_t ori = MMA7455_GetOrientation(&XYZT);
       if(ori & MMA7455_XUP) draw_cursor_x++;
       if (ori & MMA7455_XDOWN) draw_cursor_x--;
       if (ori & MMA7455_YUP) draw_cursor_y++;
       if (ori & MMA7455_YDOWN) draw_cursor_y--;
    }
#endif

#endif  /*TCS_USED*/
    
    if((LCD_H_SIZE - CURSOR_OFF_X) < draw_cursor_x)
    {
      draw_cursor_x = LCD_H_SIZE - CURSOR_OFF_X;
    }

    if(-(CURSOR_OFF_X) > draw_cursor_x)
    {
      draw_cursor_x = -(CURSOR_OFF_X);
    }

    if((LCD_V_SIZE - CURSOR_OFF_Y) < draw_cursor_y)
    {
      draw_cursor_y = (LCD_V_SIZE - CURSOR_OFF_Y);
    }

    if(-(CURSOR_OFF_Y) > draw_cursor_y)
    {
      draw_cursor_y = -(CURSOR_OFF_Y);
    }
#else  /* !(TCS_USED || ACCEL_SENSOR_USED)*/ 
    for(i = 0; i < 0x1000000;  i++);
    
    draw_cursor_x += 32;
    if(draw_cursor_x >= (LCD_H_SIZE - CURSOR_OFF_X))
    {
      draw_cursor_x = 0;
      draw_cursor_y += 32;
      if(draw_cursor_y >(LCD_V_SIZE - CURSOR_OFF_Y))
        draw_cursor_y = 0;
    }
#endif  /*TCS_USED || ACCEL_SENSOR_USED*/

    LCD_Move_Cursor(draw_cursor_x, draw_cursor_y);
#if PAINT_ON_SCREEN
{
    
    LCD_PutPixel (LCD_PANEL_UPPER,cursor_x, cursor_y, 0x00) ;
}
#endif
  }
}

int main (void)
{
   c_entry();
   return 0;
}

/**
 * @}
 */
