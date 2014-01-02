/*
 *  SSL client demonstration program
 *
 *  Copyright (C) 2006-2011, Brainspark B.V.
 *
 *  This file is part of PolarSSL (http://www.polarssl.org)
 *  Lead Maintainer: Paul Bakker <polarssl_maintainer at polarssl.org>
 *
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include "ssl_client.h"
#include "polarssl/config.h"
#include "emac.h"
#include "uip.h"
#include "debug_frmwrk.h"

#define DEBUG_LEVEL 0
#include <stdio.h>

/*
 * htons() is not always available.
 * By default go for LITTLE_ENDIAN variant. Otherwise hope for _BYTE_ORDER and __BIG_ENDIAN
 * to help determine endianess.
 */
#if defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && __BYTE_ORDER == __BIG_ENDIAN
#define POLARSSL_HTONS(n) (n)
#else
#define POLARSSL_HTONS(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#endif

unsigned short net_htons(unsigned short n);
#define net_htons(n) POLARSSL_HTONS(n)
#if defined ( __CC_ARM )
typedef unsigned int size_t;
#endif

/*-----------------------------------------------------------------------------------*/
int net_recv( void *ctx, unsigned char *buf, size_t len )
{ 
    int recvbytes = len;
    if(recvbytes > uip_len)
        recvbytes = uip_len;
    if(recvbytes > 0)
    {
      memcpy(buf, uip_appdata, recvbytes);
      uip_len -= recvbytes;
      uip_appdata = ((char*)uip_appdata) + recvbytes;
    }
    return( recvbytes );
}

/*-----------------------------------------------------------------------------------*/
int net_send( void *ctx, const unsigned char *buf, size_t len )
{
    int sendbytes = len;
    int maxlen = (uip_mss() - uip_slen);
    if(sendbytes > maxlen)
        sendbytes = maxlen;
    memcpy(&((unsigned char*)uip_sappdata)[uip_slen], buf, sendbytes);
    uip_slen += sendbytes;
    return( sendbytes );
}

/*-----------------------------------------------------------------------------------*/
static int ctr_drbg;
void my_debug( void *ctx, int level, const char *str )
{
    if( level < DEBUG_LEVEL )
    {
        _DBG((char*)str);
    }
}

/*-----------------------------------------------------------------------------------*/
int sslclient_random( void *p_rng,
                     unsigned char *output, size_t output_len )
{
    size_t i = 0;
    for(i = 0; i < output_len; i++)
    {
      *output = i+1;
      output++;
    }
    return 0;
}
/*-----------------------------------------------------------------------------------*/
int sslclient_init(ssl_context *ssl)
{
    int ret;
 
    SDRAMInit();
   
    memset( ssl, 0, sizeof( ssl_context ) );

     /*
     * 2. Setup stuff
     */
    _DBG_("[DEBUG]Set up the SSL/TLS structure..." );

    if( ( ret = ssl_init( ssl) ) != 0 )
    {
        _DBG_(" Setup failed\n");
        return ret;
    }

    ssl_set_endpoint( ssl, SSL_IS_CLIENT );
    ssl_set_authmode( ssl, SSL_VERIFY_NONE );

    /* Set the random generation callback */
    ssl_set_rng( ssl, sslclient_random, &ctr_drbg );
    /* Set the debug callback */
    ssl_set_dbg( ssl, my_debug, 0 );
    /* Set read, write callback */
    ssl_set_bio( ssl, net_recv,0,
                       net_send, 0 );
    
    /* Set ciphers */
    //ssl_set_ciphersuites( ssl, ssl_default_ciphersuites );
 
    return 0;
}
/*-----------------------------------------------------------------------------------*/
int sslclient_close(ssl_context *ssl)
{
  ssl_close_notify( ssl );
  ssl_free( ssl );
  memset( ssl, 0, sizeof( ssl_context ) );
  return 0;
}



