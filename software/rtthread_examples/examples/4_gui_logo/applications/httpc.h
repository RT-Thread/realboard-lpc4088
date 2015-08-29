/*
 * File      : httpc.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2015, RT-Thread Development Team
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
 *
 * Change Logs:
 * Date           Author       Notes
 * 2015-05-02     Bernard      port it from realboard-stm32f4
 */

#ifndef __HTTPC_H__
#define __HTTPC_H__

#include <rtthread.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

struct http_session
{
    char* user_agent;
	int   socket;

    /* size of http file */
    rt_size_t size;
    rt_off_t  position;
};

struct http_session* http_session_open(const char* url);
rt_size_t http_session_read(struct http_session* session, rt_uint8_t *buffer, rt_size_t length);
rt_off_t http_session_seek(struct http_session* session, rt_off_t offset, int mode);
int http_session_close(struct http_session* session);

int http_resolve_address(struct sockaddr_in *server, const char * url, char *host_addr, char** request);
int http_is_error_header(char *mime_buf);
int http_read_line( int socket, char * buffer, int size );

#endif
