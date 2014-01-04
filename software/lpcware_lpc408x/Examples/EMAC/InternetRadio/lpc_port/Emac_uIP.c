/***********************************************************************//**
 * @file       Emac_uIP.c
 * @purpose    This example implements an Interface Radio Device.
 * @version    1.0
 * @date       01. November. 2012
 * @author     NXP MCU SW Application Team
 * @note       This example reference from LPC1700CMSIS package source
 *---------------------------------------------------------------------
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
#include <stdio.h>
#include <string.h>
#include "debug_frmwrk.h"
#include "clock-arch.h"
#include "timer.h"
#include "uip-conf.h"
#include "uipopt.h"
#include "uip_arp.h"
#include "uip.h"
#include "emac.h"
#include "lpc_types.h"
#include "lpc_pinsel.h"
#include "lpc_gpio.h"
#include "uda1380.h"
#include "uipopt.h"

/** @defgroup Emac_WebRadio Web Radio Devide
 * @ingroup Emac_Examples
 * @{
 */

/************************** PRIVATE DEFINITIONS ***********************/
static  struct uip_eth_hdr *BUF = ((struct uip_eth_hdr *)&uip_buf[0]);

const char EMAC_ADDR[]  = {0x0C, 0x1D, 0x12, 0xE0, 0x1F, 0x10};
uint16_t     my_ipaddr[2]               = {0,0};
uint8_t      my_ip_assigned             = 0;

#if UIP_CONF_UDP
const uint8_t DNS_SERVER_ADDR[]         = {8,8,8,8};
uint8_t       dns_configured            = 0;
int8_t        host_resolv               = 0;
uint16_t      host_ipaddr[2]            = {0,0};
#endif /*UIP_CONF_UDP*/

/* Go to http://www.shoutcast.com/, select a Radio Station which has stream 
type is MP3, right click on Play button and select Save Link As into a .pls file. 
Open the saved file to get the server address, server port and file path. Fill this 
information to the below variables. */ 
// Station 1
//char server_addr[128]           =  "stream-200.shoutcast.com";
//char file_addr[80]              =    "/smoothjazz_skyfm_mp3_96kbps";
//int server_port                 = 80;

// Station 2
char server_addr[128]           =  "173.192.225.172";
char file_addr[80]              =  "/";
int server_port                 = 9500;

// Station 3
//char server_addr[128]           =  "76.73.4.30";
//char file_addr[80]              =  "/";
//int server_port                 = 80;

// Station 4
//char server_addr[128]           =  "209.73.138.42";
//char file_addr[80]              =  "/Live";
//int server_port                 = 7115;


char server_get                 = 1;
char server_resolv              = 0;

/* For debugging... */
#include <stdio.h>
#define DB    _DBG((uint8_t *)_db)
char _db[128];

#define DEBUG_DATA_RECV 0
#if DEBUG_DATA_RECV
static int64_t ticks_1s;
static int64_t bytes_cnt = -1;
#endif /*DEBUG_DATA_RECV*/


extern unsigned char uiplib_ipaddrconv(char *addrstr, unsigned char *ipaddr);
extern Status Decoder_Init (void);
extern void Decoder_Close(void);
extern void Decoder_FillInputBuffer(uint8_t* data, uint32_t size);
extern int  Decoder_Decode(void);

/*********************************************************************//**
 * @brief        Events logging
 * @param[in]    None
 * @return         None
 **********************************************************************/
void uip_log (char *m)
{
#if DEBUG    
    _DBG("[DEBUG] ");
    _DBG(m);
    _DBG_("");
#endif    
}
#if UIP_CONF_UDP
void dns_query (char *name)
{
    host_resolv = 0;
    host_ipaddr[0] = 0;
    host_ipaddr[1] = 0;
    resolv_query(name); 
}
void resolv_found (char *name, u16_t *ipaddr)
{
    if(ipaddr == NULL)
    {
        host_resolv = -1;
        _DBG("[DEBUG]Host ");_DBG(name);_DBG_(" not found.");
    }
    else
    {
        host_resolv = 1;
        server_resolv = 1;
        host_ipaddr[0] = ipaddr[0];
        host_ipaddr[1] = ipaddr[1];
       
        _DBG("[DEBUG]Found  name \"");_DBG(name);_DBG("\" = ");
        _DBD(htons(ipaddr[0]) >> 8);_DBG(".");
        _DBD(htons(ipaddr[0]) & 0xff);_DBG(".");
        _DBD(htons(ipaddr[1]) >> 8);_DBG(".");
        _DBD(htons(ipaddr[1]) & 0xff);_DBG(".");
        _DBG_("");
        
        if((strcmp(server_addr,name) == 0) && server_get)
        {
            _DBG("[DEBUG]Go to \"");_DBG(server_addr);_DBG(file_addr);_DBG_("\"...");
            webclient_get(server_addr, server_port, (char*)file_addr);
            server_get = 0;
        }   
   
    }         
}
#endif /*UIP_CONF_UDP*/

#if defined (DHCP_ENABLE)
void
dhcpc_configured(const struct dhcpc_state *s)
{
  my_ipaddr[0] = s->ipaddr[0];
  my_ipaddr[1] = s->ipaddr[1];  
  uip_sethostaddr(s->ipaddr);
  uip_setnetmask(s->netmask);
  uip_setdraddr(s->default_router);
  
  sprintf(_db, "[DEBUG]Set own IP address: %d.%d.%d.%d \n\r", \
            htons(s->ipaddr[0]) >> 8,  htons(s->ipaddr[0]) & 0xFF , \
            htons(s->ipaddr[1]) >> 8, htons(s->ipaddr[1]) & 0xFF);
  DB;
    
  sprintf(_db, "[DEBUG]Set Router IP address: %d.%d.%d.%d \n\r", \
            htons(s->default_router[0]) >> 8,  htons(s->default_router[0]) & 0xFF , \
            htons(s->default_router[1]) >> 8, htons(s->default_router[1]) & 0xFF);
  DB;
    
  sprintf(_db, "[DEBUG]Set Subnet mask: %d.%d.%d.%d \n\r", \
            htons(s->netmask[0]) >> 8,  htons(s->netmask[0]) & 0xFF , \
            htons(s->netmask[1]) >> 8, htons(s->netmask[1]) & 0xFF);
  DB;
  my_ip_assigned = 1;
}
#endif /* DHCP_ENABLE */

void webclient_connected (void)
{
     _DBG_("[DEBUG]Connected, waiting for data...");
}
void webclient_datahandler (char* data, u16_t len)
{   
    if(len)
    {
        Decoder_FillInputBuffer((uint8_t*)data,len);      
    }
}
void webclient_timedout (void)
{
     _DBG_("[DEBUG]Webclient: connection timed out");
     _DBG("[DEBUG]Webclient: re-connect to \"");_DBG(server_addr);_DBG(file_addr);_DBG_("\"...");
     webclient_get((char*)server_addr, server_port, (char*)file_addr);
}
void webclient_aborted (void)
{
     _DBG_("[DEBUG]Webclient: connection aborted");
     _DBG("[DEBUG]Webclient: re-connect to \"");_DBG(server_addr);_DBG(file_addr);_DBG_("\"...");
     webclient_get((char*)server_addr, server_port, (char*)file_addr);
}
void webclient_closed (void)
{
     _DBG_("[DEBUG]Webclient: connection closed");
}
void webclient_move(char *host, u16_t port, char *file)
{
    if(strcmp(server_addr,host))
    {
        sprintf(_db, "[DEBUG]Webclient: move to \"%s:%d%s\".\n\r",host,port,file);
        DB;
        strcpy(server_addr, host);
        server_port = port;
        strcpy(file_addr, file);
#if UIP_CONF_UDP        
        if(resolv_lookup(host) == NULL) {
            dns_query(host);
            server_get = 1;
        } 
        else 
#endif            
        {
            webclient_get(host, port, file);
            server_get = 0;
        }
    }
    else if((strcmp(file_addr,file)) || (server_port != port))
    {
        webclient_get(host, port, file);
        server_get = 0;
    }
}

void Error_Loop(void)
{
    while(1);
}
/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief        c_entry: Main program body
 * @param[in]    None
 * @return         None
 **********************************************************************/
void c_entry(void)
{
    UNS_32 i;
    volatile UNS_32 delay;
    struct timer periodic_timer, arp_timer;
    uip_ipaddr_t ipaddr;
    uint8_t app_init = 1;   
    /* Initialize debug via UART0
     * – 115200bps
     * – 8 data bit
     * – No parity
     * – 1 stop bit
     * – No flow control
     */
    debug_frmwrk_init();

    _DBG_("\r\n*********************************************");
    _DBG_("Hello NXP Semiconductors");
    _DBG_("Web Radio application");
    _DBG_("*********************************************");

    _DBG_("[DEBUG]Init Clock");
    // Sys timer init 1/100 sec tick
    clock_init();
    
    if(Decoder_Init() == ERROR)
    {
        Error_Loop();
    }

    timer_set(&periodic_timer, CLOCK_SECOND); /*0.5s */
    timer_set(&arp_timer, CLOCK_SECOND * 10);    /*10s */

    // Initialize the ethernet device driver
    while(!tapdev_init((uint8_t*)EMAC_ADDR)){
        // Delay for a while then continue initializing EMAC module
        _DBG_("[DEBUG]Error during initializing EMAC, restart after a while");
        for (delay = 0x100000; delay; delay--);
    }

    _DBG_("[DEBUG]Init uIP");
    // Initialize the uIP TCP/IP stack.
    uip_init();

    // init MAC address
    uip_ethaddr.addr[0] = EMAC_ADDR[0];
    uip_ethaddr.addr[1] = EMAC_ADDR[1];
    uip_ethaddr.addr[2] = EMAC_ADDR[2];
    uip_ethaddr.addr[3] = EMAC_ADDR[3];
    uip_ethaddr.addr[4] = EMAC_ADDR[4];
    uip_ethaddr.addr[5] = EMAC_ADDR[5];
    uip_setethaddr(uip_ethaddr);

#if defined (DHCP_ENABLE)
    my_ip_assigned = 0;  
    dhcpc_init(&uip_ethaddr, 6); 
#else
    my_ip_assigned = 1;
    uip_ipaddr(my_ipaddr, UIP_IPADDR0,UIP_IPADDR1,UIP_IPADDR2,UIP_IPADDR3); 
    sprintf(_db, "[DEBUG]Set own IP address: %d.%d.%d.%d \n\r", \
            UIP_IPADDR0, UIP_IPADDR1, \
            UIP_IPADDR2, UIP_IPADDR3);
    DB;
    
    sprintf(_db, "[DEBUG]Set Router IP address: %d.%d.%d.%d \n\r", \
            UIP_DRIPADDR0, UIP_DRIPADDR1, \
            UIP_DRIPADDR2, UIP_DRIPADDR3);
    DB;
    
    sprintf(_db, "[DEBUG]Set Subnet mask: %d.%d.%d.%d \n\r", \
            UIP_NETMASK0, UIP_NETMASK1, \
            UIP_NETMASK2, UIP_NETMASK3);
    DB;
    
#endif
  server_resolv = 0;
  if(uiplib_ipaddrconv(server_addr, (unsigned char *)ipaddr))
  {
      server_resolv = 1;
  }
  while(1)
  {

    if(app_init)
    {
        _DBG_("[DEBUG]Init Web Client");
        webclient_init();   
        app_init = 0;
    }
#if UIP_CONF_UDP
    if(my_ip_assigned && !dns_configured)
    {
        resolv_init();
        uip_ipaddr(ipaddr, DNS_SERVER_ADDR[0],DNS_SERVER_ADDR[1],DNS_SERVER_ADDR[2],DNS_SERVER_ADDR[3]); 
        sprintf(_db, "[DEBUG]DNS Server: %d.%d.%d.%d \n\r", \
                DNS_SERVER_ADDR[0], DNS_SERVER_ADDR[1], \
                DNS_SERVER_ADDR[2], DNS_SERVER_ADDR[3]);
        DB;
        resolv_conf(ipaddr);  
        dns_configured = 1;
        
        if(!server_resolv)
        {
            _DBG("[DEBUG]Resolve server name \"");_DBG(server_addr);_DBG_("\"");
            dns_query((char*)server_addr); 
        }
    }
#endif
     if(my_ip_assigned && server_get && server_resolv)
     {
        _DBG("[DEBUG]Go to \"");_DBG(server_addr);_DBG(file_addr);_DBG_("\"...");
        webclient_get(server_addr, server_port, file_addr);
        server_get = 0;
     }

       
    uip_len = tapdev_read(uip_buf);
    if(uip_len > 0)
    {
      if(BUF->type == htons(UIP_ETHTYPE_IP))
      {
          uip_arp_ipin();
          uip_input();
          /* If the above function invocation resulted in data that
             should be sent out on the network, the global variable
             uip_len is set to a value > 0. */

          if(uip_len > 0)
          {
            uip_arp_out();
            tapdev_send(uip_buf,uip_len);
          }
      }
      else if(BUF->type == htons(UIP_ETHTYPE_ARP)) 
      {
        uip_arp_arpin();
          /* If the above function invocation resulted in data that
             should be sent out on the network, the global variable
             uip_len is set to a value > 0. */
          if(uip_len > 0)
        {
            tapdev_send(uip_buf,uip_len);
          }
      }
    }
    else if(timer_expired(&periodic_timer))
    {
      timer_reset(&periodic_timer);
      for(i = 0; i < UIP_CONNS; i++)
      {
          uip_periodic(i);
        /* If the above function invocation resulted in data that
           should be sent out on the network, the global variable
           uip_len is set to a value > 0. */
        if(uip_len > 0)
        {
          uip_arp_out();
          tapdev_send(uip_buf,uip_len);
        }
      }
#if UIP_UDP
      for(i = 0; i < UIP_UDP_CONNS; i++) {
        uip_udp_periodic(i);
        /* If the above function invocation resulted in data that
           should be sent out on the network, the global variable
           uip_len is set to a value > 0. */
        if(uip_len > 0) {
          uip_arp_out();
          tapdev_send(uip_buf,uip_len);
        }
      }
#endif /* UIP_UDP */
      /* Call the ARP timer function every 10 seconds. */
      if(timer_expired(&arp_timer))
      {
        timer_reset(&arp_timer);
        uip_arp_timer();
      }
    }
    Decoder_Decode();
  }
  //Decoder_Close();
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
