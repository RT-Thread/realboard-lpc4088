# 简介 #

0_basic_kernel是一个RB4088的基本例程，仅包含一个基本的内核，串口输出等信息。

一个基本的RT-Thread程序由以下代码构成：

* RT-Thread的启动代码（application\\startup.c文件），包括其中的`main()`函数、`rtthread_startup()`函数。
* 用户自己的应用代码（application\\application.c文件），包括了用户自己的应用入口函数：`rt_application_init()`函数实现；
* 用户的配置文件（rtconfig.h文件）

在这个示例中，用户自己的应用代码仅仅是创建了一个线程，而后打印一句

    Hello RT-Thread!

具体的代码示例（application\\application.c文件）：

```
#include <rtthread.h>

void rt_init_thread_entry(void* parameter)
{
    rt_kprintf("Hello RT-Thread!\n");
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
```

函数`rt_application_init()`会在RT-Thread启动函数（`rtthread_startup`）中被调用。在这个应用初始化入口函数中，用户可以创建自己的任务（或第一个任务）。在这个示例中是创建一个名称是"init"的初始化线程，这个初始化线程的入口是`rt_init_thread_entry`函数。

rt_init_thread_entry函数调用了rt_kprintf函数输出一行`Hello RT-Thread!`消息。

## 运行结果 ##

使用GNU GCC或Keil MDK编译，下载到RB4088开发板后，我们可以把方口的USB线接到RB4088板子上接标记有UART0/ISP的端口上，电脑上会有一个串口设备出来，使用PuTTY可以看到运行的结果：

     \ | /
    - RT -     Thread Operating System
     / | \     1.2.0 build Dec 18 2013
     2006 - 2013 Copyright by rt-thread team
    Hello RT-Thread!

