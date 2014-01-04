
#ifndef _CURSOR_H_
#define _CURSOR_H_
#include "bsp.h"

#if (_CUR_USING_LCD == _RUNNING_LCD_GFT035A320240Y)
#define CURSOR_SIZE		64
//Cursor 64x64 pixels
#define CURSOR_H_SIZE       (64)
#define CURSOR_V_SIZE       (64)
#define CURSOR_OFF_X       (CURSOR_H_SIZE/2)
#define CURSOR_OFF_Y       (CURSOR_V_SIZE/2)
#else /*(_CUR_USING_LCD != _RUNNING_LCD_GFT035A320240Y)*/
#define CURSOR_SIZE		32
//Cursor 64x64 pixels
#define CURSOR_H_SIZE       (32)
#define CURSOR_V_SIZE       (32)
#define CURSOR_OFF_X       (6)
#define CURSOR_OFF_Y       (0)
#endif

extern const unsigned char Cursor[(CURSOR_H_SIZE/4)*CURSOR_H_SIZE];
#endif /*_CURSOR_H_*/
