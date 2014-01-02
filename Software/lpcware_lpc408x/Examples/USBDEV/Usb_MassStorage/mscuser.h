/*----------------------------------------------------------------------------
 *      U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    MSCUSER.H
 *      Purpose: Mass Storage Class Custom User Definitions
 *      Version: V1.10
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC family microcontroller devices only. Nothing 
 *      else gives you the right to use this software.
 *
 *      Copyright (c) 2005-2009 Keil Software.
 *---------------------------------------------------------------------------*/

#ifndef __MSCUSER_H__
#define __MSCUSER_H__


/* Mass Storage Memory Layout */
#if 0
#define MSC_MemorySize  16384
#endif
#if 1
#define MSC_MemorySize  8192
#endif
#define MSC_BlockSize   512
#define MSC_BlockCount  (MSC_MemorySize / MSC_BlockSize)


/* Max In/Out Packet Size */
#define MSC_MAX_PACKET  64

/* MSC In/Out Endpoint Address */
#define MSC_EP_IN       0x82
#define MSC_EP_OUT      0x02

/* MSC Requests Callback Functions */
extern uint32_t MSC_Reset     (void);
extern uint32_t MSC_GetMaxLUN (void);

/* MSC Bulk Callback Functions */
extern void MSC_GetCBW (void);
extern void MSC_SetCSW (void);
extern void MSC_BulkIn (void);
extern void MSC_BulkOut(void);


#endif  /* __MSCUSER_H__ */
