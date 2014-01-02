/**********************************************************************
* $Id$      Pwm_DualEdge.c          2011-06-02
*//**
* @file     Pwm_DualEdge.c
* @brief    This program illustrates the PWM signal on 3 Channels in
*           both edge mode and single mode.
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
/** @defgroup PWM_Dual_Edge PWM Dual Edge
 * @ingroup PWM_Examples
 * @{
 */

#define _USING_PWM_NO                   0


/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main PWM program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    uint8_t pwmChannel;
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


    /* Set match value for PWM match channel 0 = 100, update immediately */
    PWM_MatchUpdate(_USING_PWM_NO, 0, 100, PWM_MATCH_UPDATE_NOW);

    /* PWM Timer/Counter will be reset when channel 0 matching
     * no interrupt when match
     * no stop when match */
    PWMMatchCfgDat.IntOnMatch = DISABLE;
    PWMMatchCfgDat.MatchChannel = 0;
    PWMMatchCfgDat.ResetOnMatch = ENABLE;
    PWMMatchCfgDat.StopOnMatch = DISABLE;
    PWM_ConfigMatch(_USING_PWM_NO, &PWMMatchCfgDat);

    /* Configure each PWM channel: --------------------------------------------- */
    /* - Channel 2: Double Edge
     * - Channel 4: Double Edge
     * - Channel 5: Single Edge
     * The Match register values are as follows:
     * - MR0 = 100 (PWM rate)
     * - MR1 = 41, MR2 = 78 (PWM2 output)
     * - MR3 = 53, MR4 = 27 (PWM4 output)
     * - MR5 = 65 (PWM5 output)
     * PWM Duty on each PWM channel:
     * - Channel 2: Set by match 1, Reset by match 2.
     * - Channel 4: Set by match 3, Reset by match 4.
     * - Channel 5: Set by match 0, Reset by match 5.
     */

    /* Edge setting ------------------------------------ */
    PWM_ChannelConfig(_USING_PWM_NO, 2, PWM_CHANNEL_DUAL_EDGE);
    PWM_ChannelConfig(_USING_PWM_NO, 4, PWM_CHANNEL_DUAL_EDGE);
    PWM_ChannelConfig(_USING_PWM_NO, 5, PWM_CHANNEL_SINGLE_EDGE);

    /* Match value setting ------------------------------------ */
    PWM_MatchUpdate(_USING_PWM_NO, 1, 41, PWM_MATCH_UPDATE_NOW);
    PWM_MatchUpdate(_USING_PWM_NO, 2, 78, PWM_MATCH_UPDATE_NOW);
    PWM_MatchUpdate(_USING_PWM_NO, 3, 53, PWM_MATCH_UPDATE_NOW);
    PWM_MatchUpdate(_USING_PWM_NO, 4, 27, PWM_MATCH_UPDATE_NOW);
    PWM_MatchUpdate(_USING_PWM_NO, 5, 65, PWM_MATCH_UPDATE_NOW);


    /* Match option setting ------------------------------------ */
    for (pwmChannel = 1; pwmChannel < 6; pwmChannel++)
    {
        /* Configure match option */
        PWMMatchCfgDat.IntOnMatch = DISABLE;
        PWMMatchCfgDat.MatchChannel = pwmChannel;
        PWMMatchCfgDat.ResetOnMatch = DISABLE;
        PWMMatchCfgDat.StopOnMatch = DISABLE;

        PWM_ConfigMatch(PWM_1, &PWMMatchCfgDat);
    }

    /* Enable PWM Channel Output ------------------------------------ */
    /* Channel 2 */
    PWM_ChannelCmd(_USING_PWM_NO, 2, ENABLE);

    /* Channel 4 */
    PWM_ChannelCmd(_USING_PWM_NO, 4, ENABLE);

    /* Channel 5 */
    PWM_ChannelCmd(_USING_PWM_NO, 5, ENABLE);

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
