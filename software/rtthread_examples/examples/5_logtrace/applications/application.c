/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

#include <rtthread.h>
#include <board.h>

#ifndef RT_USING_LOGTRACE
#error turn on RT_USING_LOGTRACE
#endif

#include <log_trace.h>

void rt_other_thread(void* parameter);

static struct log_trace_session _lg1 = {
    /* A name at most 4 char long. */
    {"lg1"},
    /* Default log level. */
    LOG_TRACE_LEVEL_VERBOSE,
};

void rt_init_thread_entry(void* parameter)
{
    rt_err_t err;

    log_trace_init();
    err = log_trace_set_device(RT_CONSOLE_DEVICE_NAME);
    if (err != RT_EOK)
    {
        rt_kprintf("set logtrace device to %s err: %d\r\n",
                   RT_CONSOLE_DEVICE_NAME, err);
        return;
    }
    /* Use log_trace_get_device if you want to get the device back. */

    err = log_trace_register_session(&_lg1);
    /* It is an error if there are too many sessions or you are going to
     * registering a session twice. */
    if (err != RT_EOK)
    {
        rt_kprintf("register session %4.*s err: %d\r\n",
                   sizeof(_lg1.id), _lg1.id.name, err);
        return;
    }
    /* Put the session name in [] and logtrace will output according to the
     * session level. */
    log_trace(LOG_TRACE_INFO "[lg1]lg1 registered to logtrace on %d\r\n",
              rt_tick_get());
    log_trace(LOG_TRACE_ERROR "[lg1]show error\r\n");
    log_trace(LOG_TRACE_WARNING "[lg1]show warning\r\n");
    log_trace(LOG_TRACE_VERBOSE "[lg1]verbose info\r\n");
    log_trace(LOG_TRACE_VERBOSE "[lg1]if the output level is "
              "less or equal to current level, it will be shown\r\n");
    log_trace(LOG_TRACE_DEBUG "[lg1]this will not be shown\r\n");

    {
        rt_thread_t tid;

        tid = rt_thread_create("test",
                               rt_other_thread, RT_NULL,
                               2048, 0, 20);
        if (tid != RT_NULL)
            rt_thread_startup(tid);
    }

    log_trace_session_set_level(&_lg1, LOG_TRACE_LEVEL_ERROR);
    log_trace(LOG_TRACE_ERROR "[lg1] level adjusted\r\n");
    log_trace(LOG_TRACE_WARNING "[lg1] only error can be shown\r\n");

    log_trace_session_set_level(&_lg1, LOG_TRACE_LEVEL_VERBOSE);
    log_trace(LOG_TRACE_VERBOSE "[lg1] level switch back\r\n");

    log_session(&_lg1, LOG_TRACE_INFO "you can also use the session struct "
                "and omit [name]\r\n");
    log_session(&_lg1, LOG_TRACE_DEBUG "level applies.\r\n");

    log_session_lvl(&_lg1, LOG_TRACE_LEVEL_INFO, "you can use log_session_lvl "
                    "to get better performance"
                    "(when session struct is decleared as const).\r\n");
}

const static struct log_trace_session _lg2 = {
    {"test"},
    LOG_TRACE_LEVEL_VERBOSE,
};

void rt_other_thread(void* parameter)
{
    /* I omitted the error check. It won't error in this simple condition. */
    log_trace_register_session(&_lg2);

    log_trace(LOG_TRACE_VERBOSE "[test]an other session\r\n");

    log_session_lvl(&_lg2, LOG_TRACE_LEVEL_VERBOSE, "on tick %d\r\n",
                    rt_tick_get());

    log_session_lvl(&_lg2, LOG_TRACE_LEVEL_DEBUG,
                    "this will be removed by compiler when optimize is on\r\n");
}

int rt_application_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("init",
        rt_init_thread_entry, RT_NULL,
        2048, RT_THREAD_PRIORITY_MAX/3, 20);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
