/*
 * File      : wget.h
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

#include <stdint.h>
#include <rtthread.h>

#include <dfs_posix.h>
#include <lwip/sockets.h>
#include <finsh.h>

#include "httpc.h"

#define BUFFER_SIZE                 4096

const char _get[] = "GET %s HTTP/1.0\r\nHost: %s:%d\r\nUser-Agent: RT-Thread HTTP Agent\r\nConnection: close\r\n\r\n";

static int servicer_connect(struct sockaddr_in* server, char* host_addr, const char* url)
{
    int socket_handle;
    int peer_handle;
    int rc;
    char mimeBuffer[256];

    if ((socket_handle = lwip_socket( PF_INET, SOCK_STREAM, IPPROTO_TCP )) < 0)
    {
        rt_kprintf( "[httpc]: SOCKET FAILED\n" );
        return -1;
    }

    peer_handle = lwip_connect( socket_handle, (struct sockaddr *) server, sizeof(*server));
    if ( peer_handle < 0 )
    {
        lwip_close(socket_handle);
        rt_kprintf( "[httpc]: CONNECT FAILED %i\n", peer_handle );
        return -1;
    }

    {
        char *buf;
        rt_uint32_t length;

        buf = rt_malloc (512);
        if (*url)
            length = rt_snprintf(buf, 512, _get, url, host_addr, ntohs(server->sin_port));
        else
            length = rt_snprintf(buf, 512, _get, "/", host_addr, ntohs(server->sin_port));

        rc = lwip_send(peer_handle, buf, length, 0);

        /* release buffer */
        rt_free(buf);
    }
    /* read the header information */
    while ( 1 )
    {
        // read a line from the header information.
        rc = http_read_line(peer_handle, mimeBuffer, sizeof(mimeBuffer));
        rt_kprintf(">>%s", mimeBuffer);

        if ( rc < 0 )
        {
            lwip_close(peer_handle);
            return rc;
        }

        // End of headers is a blank line.  exit.
        if (rc == 0) break;
        if ((rc == 2) && (mimeBuffer[0] == '\r')) break;

        if (strstr(mimeBuffer, "HTTP/1."))
        {
            rc = http_is_error_header(mimeBuffer);
            if(rc)
            {
                rt_kprintf("[httpc]: status code = %d!\n", rc);
                lwip_close(peer_handle);
                return -rc;
            }
        }
    }

    return peer_handle;
}

static int servicer_session_open(const char* url)
{
    int peer_handle = -1;
    struct sockaddr_in server;
    char *request, host_addr[32];

    {
        uint32_t dns_try = 5;
        while(dns_try--)
        {
            if(http_resolve_address(&server, url, &host_addr[0], &request) == 0)
            {
                break;
            }
            rt_kprintf("[INFO] dns try...!\r\n");
        }
        if(dns_try == 0)
        {
            rt_kprintf("[ERR] dns retry timeout!\r\n");
            return -1;
        }
    }

    rt_kprintf("connect to: %s...\n", host_addr);

    if((peer_handle = servicer_connect(&server, host_addr, request)) < 0)
    {
        rt_kprintf("[httpc]: failed to connect to '%s'!\n", host_addr);
    }
    return peer_handle;

}

int http_down(const char * file_name, const char * url)
{
    int peer_handle = 0;
    int fd;
    int rc;

    peer_handle = servicer_session_open(url);

    if(peer_handle < 0)
    {
        return -1;
    }

    fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0);
    if(fd < 0)
    {
        return -1;
    }

    /* get and write */
    {
        uint32_t get_size = 0;
        uint32_t get_count = 0;
        uint8_t * buf = NULL;
        uint8_t * get_buffer;

        buf = rt_malloc (BUFFER_SIZE * 2);
        if(buf == RT_NULL)
        {
            return -1;
        }
        else
        {
            get_buffer = buf;
        }

        while ( 1 )
        {
            // read a line from the header information.
            rc = lwip_recv(peer_handle, get_buffer, BUFFER_SIZE, 0);

            if ( rc < 0 ) break;

            // End of headers is a blank line.  exit.
            if (rc == 0) break;

            get_buffer += rc;
            get_count += rc;

            get_size += rc;
            if(get_size >= BUFFER_SIZE)
            {
                write(fd, buf, BUFFER_SIZE);
                rt_kprintf("#");
                get_size -= BUFFER_SIZE;

                if(get_size > 0)
                {
                    memcpy(buf, buf + BUFFER_SIZE, get_size);
                    get_buffer = buf + get_size;
                }
                else
                {
                    get_buffer = buf;
                }
            }
        }

        if(get_size > 0)
        {
            write(fd, buf, get_size);
            rt_kprintf("[httpc] write %u\r\n", get_size);
        }
        rt_free(buf);
    } /* get and write */

    lwip_close(peer_handle);
	close(fd);

    return 0;
}

static void usage(void)
{
	rt_kprintf("wget - download file by http client\n");
	rt_kprintf("Usage:\n");
	rt_kprintf("       wget URL\n");
	rt_kprintf("       wget -o output URL\n");

	return;
}

int wget(int argc, char** argv)
{
	char* filename;
	char* URL;

	/* get options */
	if (argc == 2)
	{
		URL = argv[1];
		filename = strrchr(URL, '/');
		if (filename)
		{
			filename += 1;
		}

		if (filename == RT_NULL || *filename == '\0')
		{
			filename = "/file.txt";
		}
	}
	else if (argc == 4)
	{
		if (strcmp(argv[1], "-o") != 0)
		{
			usage();
			return -1;
		}

		filename = argv[2];
		URL = argv[3];
	}
	else
	{
		usage();
		return -1;
	}

    http_down(filename, URL);

    return 0;
}
MSH_CMD_EXPORT(wget, http get file);
