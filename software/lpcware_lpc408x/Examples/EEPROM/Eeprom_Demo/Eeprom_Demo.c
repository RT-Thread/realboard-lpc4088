/**********************************************************************
* $Id$      Eeprom_Demo.c   2011-06-02
*//**
* @file     Eeprom_Demo.c
* @brief    This example describes how to use I2S transfer in interrupt
*           mode
* @version  1.0
* @date     02. June. 2011
* @author   NXP MCU SW Application Team
*
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/
#include "LPC407x_8x_177x_8x.h"
#include "lpc_eeprom.h"
#include "debug_frmwrk.h"
#include "lpc_clkpwr.h"

#define PAGE_OFFSET         0x10
#define PAGE_ADDR           0x01


/* Example group ----------------------------------------------------------- */
/** @defgroup EEPROM_Demo   EEPROM Demo
 * @ingroup EEPROM_Examples
 * @{
 */

/************************** PRIVATE VARIABLES ***********************/
uint8_t menu[]=
"********************************************************************************\n\r"
" Hello NXP Semiconductors \n\r"
" EEPROM demo example \n\r"
#ifdef CORE_M4
"\t - MCU: LPC407x_8x \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
#else
"\t - MCU: LPC177x_8x \n\r"
"\t - Core: ARM CORTEX-M3 \n\r"
#endif
"\t - Communicate via: UART0\1\2 - 115200 bps \n\r"
" This example used to demo EEPROM operation on LPC177x_8x.\n\r"
" A 'Hello' sentence will be written into EEPROM memory, then read back and check.\n\r"
"********************************************************************************\n\r";

#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=4
uint8_t read_buffer[EEPROM_PAGE_SIZE*2];
#pragma data_alignment=4
uint8_t write_buffer[]="NXP Semiconductor LPC177x_8x-CortexM3 \n\r\t--- HELLO WORLD!!! ---";
#else
uint8_t __attribute__ ((aligned (4))) read_buffer[EEPROM_PAGE_SIZE*2];
#ifdef CORE_M4
uint8_t __attribute__ ((aligned (4))) write_buffer[EEPROM_PAGE_SIZE]="NXP Semiconductor LPC407x_8x-CortexM4 \n\r\t--- HELLO WORLD!!! ---";
#else
uint8_t __attribute__ ((aligned (4))) write_buffer[EEPROM_PAGE_SIZE]="NXP Semiconductor LPC177x_8x-CortexM3 \n\r\t--- HELLO WORLD!!! ---";
#endif
#endif

/************************** PRIVATE FUNCTIONS *************************/
/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry (void) {                       /* Main Program */
    uint32_t i, j;
    uint8_t count;
    uint8_t error = 0;

    debug_frmwrk_init();
    _DBG(menu);
    EEPROM_Init();

    count = sizeof(write_buffer);
    count &= 0xFC;

    _DBG_("Erase EEPROM");
    for(i = 0; i < EEPROM_PAGE_NUM; i++)
    {
        EEPROM_Erase(i);
    }
    for(i=0;i<EEPROM_PAGE_NUM;i++)
    {
        uint32_t *ptr = (uint32_t*)read_buffer;
        EEPROM_Read(0,i,(void*)read_buffer,MODE_32_BIT,EEPROM_PAGE_SIZE/4);
        for(j = 0; j < EEPROM_PAGE_SIZE/4; j++)
        {
            if(*ptr++ != 0)
            {
                _DBG("Erase ERROR at page ");_DBD(i);_DBG_("");
                error = 1;
                break;
            }
        }
    }
    if(error)
        while(1);
    _DBG_("Write data to EEPROM");
    EEPROM_Write(PAGE_OFFSET,PAGE_ADDR,(void*)write_buffer,MODE_8_BIT,count/1);
    _DBG_("Read data from EEPROM");
    EEPROM_Read(PAGE_OFFSET,PAGE_ADDR,(void*)read_buffer,MODE_16_BIT,count/2);

    //display eeprom data
    for(i=0;i<count;i++)
    {
        if(read_buffer[i] != write_buffer[i])
        {
             _DBG("Difference at position ");_DBD(i);_DBG_("");
             error = 1;
        }
    }
        
    if(error)
          _DBG_("ERROR!!!!");
        else
        {
            for(i=0;i<count;i++)
            {
              _DBC(read_buffer[i]);
            }
            _DBG_("");
            _DBG_("Demo is terminated");
        }
     
    while(1);

}

/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
int main(void)
{
    c_entry();
    return 0;
}


/**
 * @}
*/
