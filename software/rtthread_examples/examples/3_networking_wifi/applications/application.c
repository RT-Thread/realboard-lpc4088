/*
   此demo用于演示网络部分
 */
#include <board.h>
#include <rtthread.h>
#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
#endif


#include "components.h"

#include "spi_wifi_rw009.h"

#define WIFI_AP_SSID			"RW009 AP"
#define WIFI_AP_CHANNEL			6
#define WIFI_AP_SEC 			SECURITY_WPA2_MIXED_PSK // SECURITY_WEP_PSK SECURITY_WPA2_MIXED_PSK
#define WIFI_AP_PASS			"12345678" // "12345678"

extern void lpc_emac_hw_init(void);

void rt_init_thread_entry(void *parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif

#ifdef RT_USING_LWIP
    /* initialize eth interface */
    rt_hw_wifi_init("spi01", MODE_STATION); // MODE_STATION MODE_SOFTAP
    rw009_join("you_AP", "you_passwd");
	set_if("w0", "192.168.1.30", "192.168.1.1", "255.255.255.0");
	
    //rt_hw_wifi_init("spi01", MODE_SOFTAP); // MODE_STATION MODE_SOFTAP
	//rw009_softap(WIFI_AP_SSID, WIFI_AP_PASS, WIFI_AP_SEC, WIFI_AP_CHANNEL);
	//dhcpd_start();
#endif /* RT_USING_LWIP */
}

int rt_application_init()
{
    rt_thread_t tid;

    tid = rt_thread_create("init",
                           rt_init_thread_entry, RT_NULL,
                           2048, RT_THREAD_PRIORITY_MAX / 3, 20); //

    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}
