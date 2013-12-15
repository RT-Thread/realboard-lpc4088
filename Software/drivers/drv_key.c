#include <rtthread.h>
#include <board.h>

#include <rtgui/event.h>
#include <rtgui/rtgui_server.h>

#define key_left_GETVALUE()     (LPC_GPIO4->PIN&(1<<19))  	  //定义KEY1连接的GPIO管脚
#define key_right_GETVALUE()    (LPC_GPIO4->PIN&(1<<13))	    //定义KEY2连接的GPIO管脚
#define key_enter_GETVALUE()    (LPC_GPIO2->PIN&(1<<10))  		//定义KEY3连接的GPIO管脚


/************************************************************************************************************
*函数名： Key_GPIO_Config()
*参数：void
*返回值：void
*功能：按键GPIO的初始化函数，使用按键前必须先调用此函数进行初始化
************************************************************************************************************/
static void key_gpio_config(void)
{
  LPC_IOCON->P4_19 &= ~0x07;				  //配置P4_19为GPIO模式
 // LPC_IOCON->P4_13 &= ~0x07;					//配置P4_13为GPIO模式
  LPC_IOCON->P2_10 &= ~0x07;					//配置P2_10为GPIO模式
  LPC_GPIO4->DIR&=~(1<<19);					  //配置P4_19的GPIO方向为输入
  //LPC_GPIO4->DIR&=~(1<<13);					  //配置P4_13的GPIO方向为输入
  LPC_GPIO2->DIR&=~(1<<10);					  //配置P2_10的GPIO方向为输入     
}

static void key_thread_entry(void *parameter)
{
    rt_time_t next_delay;
    struct rtgui_event_kbd kbd_event;

    key_gpio_config();
 
    /* init keyboard event */
    RTGUI_EVENT_KBD_INIT(&kbd_event);
	kbd_event.wid = RT_NULL;
    kbd_event.mod  = RTGUI_KMOD_NONE;
    kbd_event.unicode = 0;

    while (1)
    {
        next_delay = 10;
        kbd_event.key = RTGUIK_UNKNOWN;
        kbd_event.type = RTGUI_KEYDOWN;

        if ( key_enter_GETVALUE() == 0 )
        {
            rt_thread_delay( next_delay*4 );
            if (key_enter_GETVALUE() != 0)
            {
                /* HOME key */
                rt_kprintf("key_home\n");
                kbd_event.key  = RTGUIK_HOME;
            }
            else
            {
                rt_kprintf("key_enter\n");
                kbd_event.key  = RTGUIK_RETURN;
            }
        }

       if ( key_left_GETVALUE()    == 0 )
        {
            rt_kprintf("key_left\n");
            kbd_event.key  = RTGUIK_LEFT;
        }

//        if ( key_right_GETVALUE()  == 0 )
//        {
//            rt_kprintf("key_right\n");
//            kbd_event.key  = RTGUIK_RIGHT;
//        }


        if (kbd_event.key != RTGUIK_UNKNOWN)
				{
            /* post down event */
            rtgui_server_post_event(&(kbd_event.parent), sizeof(kbd_event));

            next_delay = 10;
            /* delay to post up event */
            rt_thread_delay(next_delay);

            /* post up event */
            kbd_event.type = RTGUI_KEYUP;
            rtgui_server_post_event(&(kbd_event.parent), sizeof(kbd_event));
        }

        /* wait next key press */
        rt_thread_delay(next_delay);
    }
}

void rt_hw_key_init(void)
{
    rt_thread_t key_tid;
    key_tid = rt_thread_create("key",
                               key_thread_entry, RT_NULL,
                               512, RTGUI_SVR_THREAD_PRIORITY-1, 5);
    if (key_tid != RT_NULL) rt_thread_startup(key_tid);
}

