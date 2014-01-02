/**********************************************************************
* $Id$      Pwm_SingleEdge.c            2011-06-02
*//**
* @file     Pwm_SingleEdge.c
* @brief    This program illustrates the PWM signal on 6 Channels
*           in single edge mode
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
#include "lpc_pwm.h"
#include "lpc_pinsel.h"

/* Example group ----------------------------------------------------------- */
/** @defgroup PWM_Single_Edge   PWM Single Edge
 * @ingroup PWM_Examples
 * @{
 */

#define _USING_PWM_NO                   0

#define PWM_CHANNEL_DIFFERENT_VALUE     (30)

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main PWM program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    uint8_t pwmChannel, channelVal;
    PWM_TIMERCFG_Type PWMCfgDat;
    PWM_MATCHCFG_Type PWMMatchCfgDat;

    /* PWM block section -------------------------------------------- */
    /* Initialize PWM peripheral, timer mode
    * PWM prescale value = 1 (absolute value - tick value) */
    PWMCfgDat.PrescaleOption = PWM_TIMER_PRESCALE_TICKVAL;
    PWMCfgDat.PrescaleValue = 1;
    PWM_Init(_USING_PWM_NO, PWM_MODE_TIMER, (void *) &PWMCfgDat);

    // Initialize PWM pin connect
#if (_USING_PWM_NO == 1)
    for (pwmChannel = 0; pwmChannel <= 6; pwmChannel++)
    {
        PINSEL_ConfigPin (2, pwmChannel, 1);
    }
#elif (_USING_PWM_NO == 0)
    PINSEL_ConfigPin (1, 2, 3);//PWM0.1
    PINSEL_ConfigPin (1, 3, 3);//PWM0.2
    PINSEL_ConfigPin (1, 5, 3);//PWM0.3
    PINSEL_ConfigPin (1, 6, 3);//PWM0.4
    PINSEL_ConfigPin (1, 7, 3);//PWM0.5
    PINSEL_ConfigPin (1, 11, 3);//PWM0.6
#else
    return 0;
#endif

    /* Set match value for PWM match channel 0 = 256, update immediately */
    PWM_MatchUpdate(_USING_PWM_NO, 0, 256, PWM_MATCH_UPDATE_NOW);

    /* PWM Timer/Counter will be reset when channel 0 matching
    * no interrupt when match
    * no stop when match */
    PWMMatchCfgDat.IntOnMatch = DISABLE;
    PWMMatchCfgDat.MatchChannel = 0;
    PWMMatchCfgDat.ResetOnMatch = ENABLE;
    PWMMatchCfgDat.StopOnMatch = DISABLE;
    PWM_ConfigMatch(_USING_PWM_NO, &PWMMatchCfgDat);

    /* Configure each PWM channel: --------------------------------------------- */
    /* - Single edge
    * - PWM Duty on each PWM channel determined by
    * the match on channel 0 to the match of that match channel.
    * Example: PWM Duty on PWM channel 1 determined by
    * the match on channel 0 to the match of match channel 1.
    */

    /* Configure PWM channel edge option
    * Note: PWM Channel 1 is in single mode as default state and
    * can not be changed to double edge mode */
    for (pwmChannel = 2; pwmChannel < 7; pwmChannel++)
    {
        PWM_ChannelConfig(_USING_PWM_NO, pwmChannel, PWM_CHANNEL_SINGLE_EDGE);
    }


    /* Configure match value for each match channel */
    channelVal = 10;
    for (pwmChannel = 1; pwmChannel < 7; pwmChannel++)
    {
        /* Set up match value */
        PWM_MatchUpdate(_USING_PWM_NO, pwmChannel, channelVal, PWM_MATCH_UPDATE_NOW);

        /* Configure match option */
        PWMMatchCfgDat.IntOnMatch = DISABLE;
        PWMMatchCfgDat.MatchChannel = pwmChannel;
        PWMMatchCfgDat.ResetOnMatch = DISABLE;
        PWMMatchCfgDat.StopOnMatch = DISABLE;
        PWM_ConfigMatch(_USING_PWM_NO, &PWMMatchCfgDat);

        /* Enable PWM Channel Output */
        PWM_ChannelCmd(_USING_PWM_NO, pwmChannel, ENABLE);

        /* Increase match value by 30 */
        channelVal += PWM_CHANNEL_DIFFERENT_VALUE;
    }

    /* Reset and Start counter */
    PWM_ResetCounter(_USING_PWM_NO);

    PWM_CounterCmd(_USING_PWM_NO, ENABLE);

    /* Start PWM now */
    PWM_Cmd(_USING_PWM_NO, ENABLE);

    /* Loop forever */
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
