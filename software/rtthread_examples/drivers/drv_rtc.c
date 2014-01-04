#include <string.h>
#include <time.h>
#include <rtthread.h>
#include <board.h>
#include "drv_rtc.h"

#ifdef RT_USING_RTC
#define FIRST_DATA          0x32F2
static struct rt_device rtc;
static rt_err_t rt_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* Open Interrupt */
    }

    return RT_EOK;
}

static rt_size_t rt_rtc_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    return 0;
}

static rt_err_t rt_rtc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    time_t *time;
    RTC_TIME_Type   RTC_TimeStructure;
	  
    struct tm time_temp;

    RT_ASSERT(dev != RT_NULL);
    memset(&time_temp, 0, sizeof(struct tm));

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        time = (time_t *)args;

        /* Get the current Time */
		    RTC_GetFullTime(LPC_RTC,&RTC_TimeStructure);
        /* Years since 1900 : 0-99 range */
        time_temp.tm_year = RTC_TimeStructure.YEAR + 2000 - 1900;
        /* Months *since* january 0-11 : RTC_Month_Date_Definitions 1 - 12 */
        time_temp.tm_mon = RTC_TimeStructure.MONTH - 1;
        /* Day of the month 1-31 : 1-31 range */
        time_temp.tm_mday = RTC_TimeStructure.DOM;
        /* Hours since midnight 0-23 : 0-23 range */
        time_temp.tm_hour = RTC_TimeStructure.HOUR;
        /* Minutes 0-59 : the 0-59 range */
        time_temp.tm_min = RTC_TimeStructure.MIN;
        /* Seconds 0-59 : the 0-59 range */
        time_temp.tm_sec = RTC_TimeStructure.SEC;

        *time = mktime(&time_temp);
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
    {
        const struct tm* time_new;
        time = (time_t *)args;
        time_new = localtime(time);

        /* 0-99 range              : Years since 1900 */
        RTC_TimeStructure.YEAR = time_new->tm_year + 1900 - 2000;
        /* RTC_Month_Date_Definitions 1 - 12 : Months *since* january 0-11 */
        RTC_TimeStructure.MONTH = time_new->tm_mon + 1;
        /* 1-31 range : Day of the month 1-31 */
        RTC_TimeStructure.DOM = time_new->tm_mday;
        /* 1 - 7 : Days since Sunday (0-6) */
        RTC_TimeStructure.DOW = time_new->tm_wday + 1;
        /* 0-23 range : Hours since midnight 0-23 */
        RTC_TimeStructure.HOUR = time_new->tm_hour;
        /* the 0-59 range : Minutes 0-59 */
        RTC_TimeStructure.MIN = time_new->tm_min;
        /* the 0-59 range : Seconds 0-59 */
        RTC_TimeStructure.SEC = time_new->tm_sec;

        /* Set Current Time and Date */
				RTC_SetFullTime(LPC_RTC,&RTC_TimeStructure);
				RTC_WriteGPREG(LPC_RTC,0,FIRST_DATA);
    }
    break;
    }

    return RT_EOK;
}

void rt_hw_rtc_init(void)
{
	rtc.type	= RT_Device_Class_RTC;
	RTC_Init(LPC_RTC);
    if (RTC_ReadGPREG(LPC_RTC,0) != FIRST_DATA)
    {
        rt_kprintf("rtc is not configured\n");
        rt_kprintf("please configure with set_date and set_time\n");
    }
    else
    {
    }
    RTC_Cmd(LPC_RTC,ENABLE);
    /* register rtc device */
    rtc.init 	= RT_NULL;
    rtc.open 	= rt_rtc_open;
    rtc.close	= RT_NULL;
    rtc.read 	= rt_rtc_read;
    rtc.write	= RT_NULL;
    rtc.control = rt_rtc_control;

    /* no private */
    rtc.user_data = RT_NULL;

    rt_device_register(&rtc, "rtc", RT_DEVICE_FLAG_RDWR);

#ifdef RT_USING_FINSH
    {
        extern void list_date(void);
        list_date();
    }
#endif

    return;
}

#endif
