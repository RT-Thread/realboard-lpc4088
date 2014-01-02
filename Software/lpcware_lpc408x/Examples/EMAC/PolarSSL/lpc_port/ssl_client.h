/**
 * \addtogroup webclient
 * @{
 */

/**
 * \file
 * Header file for the HTTP client.
 * \author Adam Dunkels <adam@dunkels.com>
 */

/*
 * Copyright (c) 2002, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: webclient.h,v 1.2 2006/06/11 21:46:37 adam Exp $
 *
 */
#ifndef __SSLCLIENT_H__
#define __SSLCLIENT_H__
#include <string.h>
#include <stdio.h>
#include "bsp.h"
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
#include "sdram_k4s561632j.h"
#elif (_CURR_USING_BRD == _QVGA_BOARD)
#include "sdram_mt48lc8m32lfb5.h"
#elif (_CURR_USING_BRD == _EA_PA_BOARD)
#include "sdram_is42s32800d.h"
#endif
#include "polarssl/error.h"
#include "polarssl/config.h"
#include "polarssl/net.h"
#include "polarssl/ssl.h"

#define SSL_DECRYPT_BUF_START_ADDR  (SDRAM_BASE_ADDR)
#define SSL_DECRYPT_BUF_SIZE        (SSL_BUFFER_LEN)

/**
 * Initialize the sslclient module.
 */
int sslclient_init(ssl_context *ssl);
/**
 * Close the sslclient module.
 */
int sslclient_close(ssl_context *ssl);
#endif /* __SSLCLIENT_H__ */

/** @} */
