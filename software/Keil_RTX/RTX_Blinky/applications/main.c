/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    BLINKY.C
 *      Purpose: RTX example program
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2013 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "lpc407x_8x_177x_8x.h"                    // Device header
#include "drv_led.h"


osThreadId t_ledOn;                     /* assigned task id of task: ledOn   */
osThreadId t_ledOff;                    /* assigned task id of task: ledOff  */


/*----------------------------------------------------------------------------
  Task 1 'ledOn': switches the LED on
 *---------------------------------------------------------------------------*/
 void ledOn (void const *argument) {
  for (;;) {
    led_ctrl_on(LED0|LED1);             /* Turn LED On                       */
    osSignalSet(t_ledOff, 0x0001);      /* send event to task 'ledoff'       */
    osDelay(500);                       /* delay 500ms                       */
  }
}

/*----------------------------------------------------------------------------
  Task 2 'ledOff': switches the LED off
 *---------------------------------------------------------------------------*/
 void ledOff (void const *argument) {
  for (;;) {
    osSignalWait (0x0001, osWaitForever); /* wait for an event flag 0x0001   */
    osDelay(800);                         /* delay 800ms                     */
    led_ctrl_off(LED0|LED1);              /* Turn LED Off                    */
  }
}

osThreadDef(ledOn,  osPriorityNormal, 1, 0);
osThreadDef(ledOff, osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
  Main: Initialize
 *---------------------------------------------------------------------------*/
int main (void) {

 led_gpio_init();                                     /* Initialize LEDs                  */

  t_ledOn  = osThreadCreate(osThread(ledOn),  NULL);  /* start task 'ledOn'  */
  t_ledOff = osThreadCreate(osThread(ledOff), NULL);  /* start task 'ledOff' */
  osDelay(osWaitForever);
}

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
