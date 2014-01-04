/*****************************************************************************
 *   cmp.h:  Header file for NXP LPC17xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.19  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __CMP_H__
#define __CMP_H__

#define CMP_VP	0
#define CMP_VM	1

#define T0CAP2  0
#define T0CAP3  1
#define T1CAP2  2
#define T1CAP3  3

#define DISABLE           0
#define DISABLE_IN_DS_PD  1
#define DISABLE_IN_PD     2
#define POWERUP           3
#define ENABLE            3

#define VDDA    0
#define VREF    1

#define ASYNC   0
#define SYNC    1

extern void CMP0_IRQHandler (void);
extern void CMP1_IRQHandler (void);
extern void CMP_IOConfig( void );
extern void CMP_Init( void );
extern void CMP_SelectInput( uint32_t num, uint32_t power, uint32_t channel, uint32_t input );
extern void CMP_SelectReference( uint32_t num, uint32_t power, uint32_t vref, uint32_t level );
extern void CMP_SetOutput( uint32_t num, uint32_t enable, uint32_t sync );
extern void CMP_SetInterrupt( uint32_t num, uint32_t inverted, uint32_t level, uint32_t edge );
extern void CMP_SetHysteresis( uint32_t num, uint32_t level );
extern uint32_t CMP_GetOutputStatus( uint32_t num );
extern void CMP_ConnectCapture( uint32_t capture, uint32_t select );
extern void CMP_ROSCControl( uint32_t input, uint32_t reset );
extern void CMP_TempSenControl( uint32_t enable, uint32_t power );
extern void CMP_BangapControl( uint32_t enable );
extern void CMP_CurrentSrcControl( uint32_t power );

#endif  /* __CMP_H__ */
/*****************************************************************************
**                            End Of File
******************************************************************************/

