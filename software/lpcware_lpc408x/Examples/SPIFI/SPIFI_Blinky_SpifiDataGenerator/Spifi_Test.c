/**********************************************************************
* $Id$      Spifi_ReadWrite.c   2012-01-16
*//**
* @file     Spifi_ReadWrite.c
* @brief    This example describes how to use GPIO to drive LEDs
* @version  1.0
* @date     16. January. 2012
* @author   NXP MCU SW Application Team
*
* Copyright(C) 2012, NXP Semiconductor
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
**********************************************************************/
#include "lpc_gpio.h"
#include "lpc_clkpwr.h"
#include "lpc_spifi_rom_api.h"
#include "debug_frmwrk.h"
#include "bsp.h"
#include "spifi_blinky.h"
#include "spifi_delay.h"
#include <string.h>

/* Example group ----------------------------------------------------------- */
/** @defgroup Spifi_ReadWrite   Spifi_ReadWrite
 * @ingroup SPIFI_Examples
 * @{
 */


/************************** PRIVATE DEFINITIONS *************************/
#define LED1_BIT            1 //LEDUSB
#define LED1_PORT           4
//#define SPIFI_BASE                  0x28000000

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] =
"********************************************************************************\n\r"
"Hello NXP Semiconductors \n\r"
" SPIFI demo \n\r"
"\t - MCU: LPC4000 \n\r"
"\t - Core: ARM CORTEX-M4 \n\r"
"\t - Communicate via: UART1 - 115200 bps \n\r"
"********************************************************************************\n\r";
SPIFIobj obj;
SPIFI_RTNS * pSpifi;
SPIFIopers opers;
unsigned char ProgramData[PROG_SIZE];
typedef void (*ENTRY_FUNC)(void);
static ENTRY_FUNC entry_func;
/*
const uint8_t spifi_blinky[] = {
0x10, 0xB5, 0x0F, 0xE0, 0x00, 0x24, 0x06, 0xE0, 0x01, 0x20, 0xA0, 0x40, 0x06, 0x49, 0x88, 0x61,
0x07, 0xF0, 0xF6, 0xFF, 0x64, 0x1C, 0x04, 0x2C, 0xF6, 0xD3, 0x0F, 0x20, 0x02, 0x49, 0xC8, 0x61,
0x07, 0xF0, 0xEE, 0xFF, 0xEE, 0xE7, 0x00, 0x00, 0x00, 0x80, 0x09, 0x20
};
*/
/*
const uint8_t spifi_blinky[] = {
0xF7, 0xFF, 0xFE, 0xF0, 0xF4, 0x4F, 0x21, 0x80, 0x4A, 0x89, 0x63, 0x91, 0x48, 0x89, 0xE0, 0x00, 
0x1E, 0x40, 0x28, 0x00, 0xD1, 0xFC, 0xF4, 0x4F, 0x21, 0x80, 0x4A, 0x85, 0x63, 0xD1, 0x48, 0x85, 
0xE0, 0x00, 0x1E, 0x40, 0x28, 0x00, 0xD1, 0xFC
};*/
/*
const uint8_t spifi_blinky[] = {
0x80, 0x21, 0x4F, 0xF4, 0x91, 0x63, 0x8D, 0x4A, 0x00, 0xE0, 0x8D, 0x48, 
0x00, 0x28, 0x40, 0x1E, 0x4F, 0xF4, 0xFC, 0xD1, 0x89, 0x4A, 0x80, 0x21, 0x89, 0x48, 0xD1, 0x63, 
0x40, 0x1E, 0x00, 0xE0, 0xFC, 0xD1, 0x00, 0x28
};*/
/*
const uint8_t spifi_blinky[] = {
0x4F, 0xF4, 0x80, 0x21, 0x8D, 0x4A, 0x91, 0x63, 0x8D, 0x48, 0x00, 0xE0,  
0x40, 0x1E, 0x00, 0x28, 0xFC, 0xD1, 0x4F, 0xF4, 0x80, 0x21, 0x89, 0x4A,  0xD1, 0x63, 0x89, 0x48, 
0x00, 0xE0, 0x40, 0x1E, 0x00, 0x28, 0xFC, 0xD1, 
};*/
/*
const uint8_t spifi_blinky[] = {
0x80, 0x20, 0x4F, 0xF4,  0x88, 0x63,  0x8C, 0x49, 0xFF, 0xFF, 0xC8, 0x63};
*/

const uint8_t spifi_blinky_test[] = {
0x4F, 0xF4, 0x80, 0x20,  0x8C, 0x49,  0x88, 0x63, 0xC8, 0x63};

/******************************************************************************
** Function name:       led_blinky
**
** Descriptions:        led blinky
**
** parameters:          None
** Returned value:      None
** 
******************************************************************************/
void led_blinky(void)
{
  uint32_t i, j;
 
  LPC_GPIO1->SET = 1 << 18;
//    for(j = 1000000; j > 0; j--);
  
  LPC_GPIO1->CLR = 1 << 18;
//  for(j = 1000000; j > 0; j--);
}
void led_blinky2(void)
{
  uint32_t i, j;
  for(i = 0; i < 4; i++)
  {
    LPC_GPIO0->SET = 1 << i;
    for(j = 1000000; j > 0; j--);
  }
  LPC_GPIO0->CLR = 0x0000000F;
  for(j = 1000000; j > 0; j--);
}
/*-------------------------PRIVATE FUNCTIONS------------------------------*/
/*********************************************************************//**
 * @brief       Print menu
 * @param[in]   None
 * @return      None
 **********************************************************************/
void print_menu(void)
{
    _DBG(menu1);
}

/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/

void spifi_io_clk_init (void) { 
    LPC_SC->PCONP |= 0x00010000;

    LPC_IOCON->P2_7 &= ~0x07;
    LPC_IOCON->P2_7 |= 0x05;    /* SPIFI_CSN @ P2.7 */
    LPC_IOCON->P0_22 &= ~0x07;
    LPC_IOCON->P0_22 |= 0x05;   /* SPIFI_CLK @ P0.22 */
    LPC_IOCON->P0_15 &= ~0x07;
    LPC_IOCON->P0_15 |= 0x5;    /* SPIFI_IO2 @ P0.15 */
    LPC_IOCON->P0_16 &= ~0x07;
    LPC_IOCON->P0_16 |= 0x5;    /* SPIFI_IO3 @ P0.16 */
    LPC_IOCON->P0_17 &= ~0x07;
    LPC_IOCON->P0_17 |= 0x5;    /* SPIFI_IO1 @ P0.17 */
    LPC_IOCON->P0_18 &= ~0x07;
    LPC_IOCON->P0_18 |= 0x5;    /* SPIFI_IO0 @ P0.18 */

    return;
}

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       Main program body
 * @param[in]   None
 * @return      int
 **********************************************************************/

int c_entry (void) {                       /* Main Program                       */

    uint32_t i;
    volatile uint32_t error = 0;

    spifi_io_clk_init();

    /* Initialize debug via UART1
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    GPIO_Init();

    GPIO_SetDir(BRD_LED_1_CONNECTED_PORT, BRD_LED_1_CONNECTED_MASK, GPIO_DIRECTION_OUTPUT);

    // print welcome screen
    print_menu();

#ifdef USE_SPIFI_LIB
    pSpifi = &spifi_table;
#else
    pSpifi = (SPIFI_RTNS *)(SPIFI_ROM_TABLE);
#endif

    _DBG("Initializing SPIFI driver...");
    /* Initialize SPIFI driver */
    error = pSpifi->spifi_init(&obj, 4, S_RCVCLK | S_FULLCLK, 60);

    if (error) while (1);

    _DBG("OK\r\nErasing QSPI device...");
        /* Erase Entire SPIFI Device if required */
    for ( i = 0 ; i < obj.memSize / 4; i+=4 )
    {
        if ( *((uint32_t *)(obj.base+i)) != 0xFFFFFFFF )
        {
            opers.dest = (char *)(obj.base);
            opers.length = obj.memSize;
            opers.scratch = NULL;
            opers.options = S_VERIFY_ERASE;
            /* Erase Device */
            if (pSpifi->spifi_erase(&obj, &opers)) while (1);
            break;
        }
    }

    _DBG("OK\r\nProgramming + verifying QSPI device...");
    for(i=0;i<PROG_SIZE;i++)
        ProgramData[i] = 0xFF;

    for (i = 0; i < 10; i++)
  {
    ProgramData[i] = *(uint8_t *)&spifi_blinky_test[i];
  }

    opers.length = PROG_SIZE;
    opers.scratch = NULL;
    opers.protect = 0;
    opers.options = S_CALLER_ERASE;
    for ( i = 0 ; i < obj.memSize / PROG_SIZE; i++ )
    {
        /* Write */
        opers.dest = (char *)( obj.base + (i*PROG_SIZE) );
        if (pSpifi->spifi_program (&obj, (char *)ProgramData, &opers)) while (1);
        /* Verify */
        if (memcmp((void *)opers.dest,(void *)ProgramData,PROG_SIZE)) while (1);
    }
    _DBG("OK!\r\n");

    // run blinky
    GPIO_SetValue(LED1_PORT,(1<<LED1_BIT)); // Light LED
    if(1)
    {
    led_blinky();
    led_blinky();
    spifi_blinky();
    spifi_delay();
    //led_blinky2();
    }
    entry_func = (ENTRY_FUNC)(obj.base+1);
  entry_func();
    
    while (1)
    {                                               // Loop forever
    }
}

extern void fpu_init(void);
/* With ARM and GHS toolsets, the entry point is main() - this will
   allow the linker to generate wrapper code to setup stacks, allocate
   heap area, and initialize and copy code and data segments. For GNU
   toolsets, the entry point is through __start() in the crt0_gnu.asm
   file, and that startup code will setup stacks and data */
int main(void)
{
    
    return c_entry();
}

#ifdef  DEBUG
/*******************************************************************************
* @brief        Reports the name of the source file and the source line number
*               where the CHECK_PARAM error has occurred.
* @param[in]    file Pointer to the source file name
* @param[in]    line assert_param error line source number
* @return       None
*******************************************************************************/
void check_failed(uint8_t *file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while(1);
}
#endif

/**
 * @}
 */
