/***********************************************************************//**
 * @file        Emac_uIP.c
 * @purpose     This example used to test EMAC uIP operation on LPC1768
 * @version     1.0
 * @date        13. Dec. 2010
 * @author      NXP MCU SW Application Team
 * @note        This example reference from LPC1700CMSIS package source
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
#include "uipopt.h"


/************************** PRIVATE DEFINITIONS ***********************/
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#define LED_PIN         (1<<6)
#define LED2_MASK       ((1<<2) | (1<<3) | (1<<4) | (1<<5) | (1<<6))
#define LED1_MASK       ((1<<28) | (1<<29) | (1<<31))

const char EMAC_ADDR[]  = {0x00, 0x21, 0x70, 0xD7, 0x74, 0x93};

#if defined (DHCP_ENABLE) || defined(WEB_CLIENT) || defined(SMTP_APP)
uint16_t     my_ipaddr[2]               = {0,0};
uint8_t      my_ip_assigned             = 0;
#endif

#if UIP_CONF_UDP
const uint8_t DNS_SERVER_ADDR[]         = {8,8,8,8};
uint8_t       dns_configured            = 0;
int8_t        host_resolv               = 0;
uint16_t      host_ipaddr[2]            = {0,0};
#endif /*UIP_CONF_UDP*/

#if defined (WEB_CLIENT)
char server_addr[128]           =  "www.sics.se";
char file_addr[80]              =    "/~adam/uip";
int server_port                 = 80;
char server_get                 = 1;
#endif /*WEB_CLIENT*/

#if defined(SMTP_APP)
const char server_addr[]        = "yoursmtpserver.com";   
const char server_name[]        = "servername";
uint16_t   server_ipaddr[2]     = {0,0};
#if ESMTP_ENABLE
const char username[]           = "username";
const char password[]           = "password";
#endif /*ESMTP_ENABLE*/
const char from_addr[]          = "youremailaddress@example.com";
const char to_addr[]            = "toaddress@example.com";
const char cc_addr[]            = "ccaddress@example.com";
const char subject[]            = "Testing SMTP from uIP";
const char message[]            = "Test message sent by uIP\r\n";
#endif /*SMTP_APP*/

/* For debugging... */
#include <stdio.h>
#define DB    _DBG((uint8_t *)_db)
char _db[128];

/*********************************************************************//**
 * @brief       Events logging
 * @param[in]   None
 * @return      None
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
        host_ipaddr[0] = ipaddr[0];
        host_ipaddr[1] = ipaddr[1];
       
        _DBG("[DEBUG]Found  name \"");_DBG(name);_DBG("\" = ");
        _DBD(htons(ipaddr[0]) >> 8);_DBG(".");
        _DBD(htons(ipaddr[0]) & 0xff);_DBG(".");
        _DBD(htons(ipaddr[1]) >> 8);_DBG(".");
        _DBD(htons(ipaddr[1]) & 0xff);_DBG(".");
        _DBG_("");
        
#if defined(WEB_CLIENT)
        if((strcmp(server_addr,name) == 0) && server_get)
        {
            _DBG("[DEBUG]Webclient: go to \"");_DBG(server_addr);_DBG(file_addr);_DBG_("\"...");
            webclient_get(server_addr, server_port, (char*)file_addr);
            server_get = 0;
        }   
#endif /*WEB_CLIENT*/       
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
#if defined(SMTP_APP)
void smtp_done(unsigned char code)
{
    switch(code)
    {
        case 0:
            _DBG_("[DEBUG]SMTP: Sending succedded\n");
            break;
        default:
            _DBG_("[DEBUG]SMTP: Sending failed\n");
            break;
    }
}
#endif /*SMTP_APP*/
#if defined(WEB_BROWSER)
void htmlparser_link(char *text, unsigned char textlen, char *url)
{
    unsigned int i;
    
    sprintf(_db, "<link=%s>",url);
    DB;
    
    for(i = 0; i < textlen; i++)
        _DBC(text[i]);
}

void htmlparser_newline(void)
{
    _DBG_("");
}
void htmlparser_word(char *word, unsigned char wordlen)
{
    unsigned int i;
    _DBC(' ');
    for(i = 0; i < wordlen; i++)
        _DBC(word[i]);
}

#endif /*WEB_BROWSER*/
#if defined (WEB_CLIENT)
void webclient_connected (void)
{
     _DBG_("[DEBUG]Webclient: connected, waiting for data...");
}
void webclient_datahandler (char* data, u16_t len)
{
    if(len)
    {
#if defined(WEB_BROWSER)
        htmlparser_parse(data, len);
#else        
        uint32_t i;
        sprintf(_db, "[DEBUG]Webclient: got %d bytes of data.\n\r",len);
        DB;
        for(i = 0; i < len; i++)
            _DBC(data[i]);
        //_DBG_("");
#endif        
    } else {
        _DBG_("");
        return;
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
        
        if(resolv_lookup(host) == NULL) {
            dns_query(host);
            server_get = 1;
        } else {
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
#endif /*WEB_CLIENT*/

/*-------------------------MAIN FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       c_entry: Main program body
 * @param[in]   None
 * @return      None
 **********************************************************************/
void c_entry(void)
{
    UNS_32 i;
    volatile UNS_32 delay;
    struct timer periodic_timer, arp_timer;
#if UIP_CONF_UDP
    uip_ipaddr_t ipaddr;
#endif     
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
#if CORE_M4
    _DBG("uIP porting on LPC407x_8x");
#else
    _DBG("uIP porting on LPC177x_8x");
#endif    
#if defined (WEB_SERVER)
    _DBG_(" (WebServer application)");
#elif defined(WEB_CLIENT)
    _DBG_(" (WebClient application)");
#elif defined (SMTP_APP)
    _DBG_(" (SMTP application)");
#elif defined(TELNET_APP)
    _DBG_(" (TELNET application)");    
#else
    #error "Please select an application"
#endif
    _DBG_("*********************************************");

    _DBG_("[DEBUG]Init Clock");
    // Sys timer init 1/100 sec tick
    clock_init();

    timer_set(&periodic_timer, CLOCK_SECOND); /*0.5s */
    timer_set(&arp_timer, CLOCK_SECOND * 10);    /*10s */

    _DBG_("[DEBUG]Init EMAC");
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
#if defined(WEB_CLIENT) || defined(SMTP_APP)
    my_ip_assigned = 1;
    uip_ipaddr(my_ipaddr, UIP_IPADDR0,UIP_IPADDR1,UIP_IPADDR2,UIP_IPADDR3); 
#endif    
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
  while(1)
  {
#if defined (WEB_SERVER)
    if(app_init)
    {
        // Initialize the HTTP server ----------------------------
        _DBG_("[DEBUG]Web Server: Init HTTP");
        httpd_init();
        _DBG_("[DEBUG]Web Server: Init complete!");
        app_init = 0;
    }
#endif /*WEB_SERVER*/
#if defined (WEB_CLIENT)
    if(app_init)
    {
        _DBG_("[DEBUG]Init Web Client");
        webclient_init(); 
#if defined(WEB_BROWSER)
        htmlparser_init();
#endif        
        app_init = 0;
    }
#endif /*WEB_CLIENT*/    
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
#if defined (WEB_CLIENT) || defined (SMTP_APP)
        _DBG("[DEBUG]Resolve server name \"");_DBG(server_addr);_DBG_("\"");
        dns_query((char*)server_addr); 
#endif        
    }
#endif  /*UIP_CONF_UDP*/
    
    
#if defined(SMTP_APP)
    if((host_resolv > 0) && app_init)
    {
        server_ipaddr[0] = host_ipaddr[0];
        server_ipaddr[1] = host_ipaddr[1];
        sprintf(_db, "[DEBUG]SMTP: Configure SMTP Server at address: %d.%d.%d.%d \n\r", \
                    htons(server_ipaddr[0]) >> 8,  htons(server_ipaddr[0]) & 0xFF , \
                    htons(server_ipaddr[1]) >> 8, htons(server_ipaddr[1]) & 0xFF);
        DB;
#if ESMTP_ENABLE
        esmtp_configure((char*)server_name, server_ipaddr,(char*)username, (char*)password);
#else        
        smtp_configure((char*)server_name, server_ipaddr);
#endif        
        _DBG_("[DEBUG]SMTP: Send email...");
        _DBG("From: ");_DBG(from_addr);_DBG_("");
        _DBG("To: ");_DBG(to_addr);_DBG_("");
        _DBG("CC: ");_DBG(cc_addr);_DBG_("");
        _DBG("Subject: ");_DBG(subject);_DBG_("");
        _DBG("Message: ");_DBG(message);_DBG_("");
        SMTP_SEND((char*)to_addr, (char*)cc_addr, (char*)from_addr,(char*)subject,(char*)message);
        app_init = 0;
    }
#endif   
#if defined(TELNET_APP)
    if(app_init)
    {
        // Initialize the Telnet server ----------------------------
        _DBG_("[DEBUG]Telnet: Init telnet server");
        telnetd_init();
        _DBG_("[DEBUG]Telnet: Init complete!");
        app_init = 0;
    }
#endif /*TELNET_APP*/
    
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
  }

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

