/*

demac - A Monkey's Audio decoder

$Id: demac.c 25850 2010-05-06 21:04:40Z kugel $

Copyright (C) Dave Chapman 2007

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA

*/

/* 

This example is intended to demonstrate how the decoder can be used in
embedded devices - there is no usage of dynamic memory (i.e. no
malloc/free) and small buffer sizes are chosen to minimise both the
memory usage and decoding latency.

This implementation requires the following memory and supports decoding of all APE files up to 24-bit Stereo.

32768 - data from the input stream to be presented to the decoder in one contiguous chunk.
18432 - decoding buffer (left channel)
18432 - decoding buffer (right channel)

17408+5120+2240 - buffers used for filter histories (compression levels 2000-5000)

In addition, this example uses a static 27648 byte buffer as temporary
storage for outputting the data to a WAV file but that could be
avoided by writing the decoded data one sample at a time.

*/

#include <inttypes.h>
#include <rtthread.h>
#include <dfs_posix.h>
#include "demac.h"
#include "board.h"
#include "parser.h"


#define BLOCKS_PER_LOOP     1152//4608
#define MAX_CHANNELS        2

#define INPUT_CHUNKSIZE     (5*1024)

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

//ape结构的全局变量
struct ape_ctx_t ape_ctx;


// 两个PCM Buffer
static unsigned char *PCM_buffer0;//[BLOCKS_PER_LOOP*4];
static unsigned char *PCM_buffer1;//[BLOCKS_PER_LOOP*4];
static int32_t *decoded0;//[BLOCKS_PER_LOOP];
static int32_t *decoded1;//[BLOCKS_PER_LOOP];

/* We assume that 32KB of compressed data is enough to extract up to
   27648 bytes of decompressed data. */
//file buffer
static unsigned char *inbuffer;


//本次用到的信号量
static struct rt_semaphore ape_sem;

/* Statically allocate the filter buffers */

#ifdef FILTER256_IRAM
extern filter_int *filterbuf32;
                  /* 2432 or 4864 bytes */
extern filter_int *filterbuf256;
                  /* 5120 or 10240 bytes */
#else
extern filter_int *filterbuf64;
                  /* 2432 or 4864 bytes */
extern filter_int *filterbuf256; /* 5120 or 10240 bytes */
#endif
extern filter_int *filterbuf1280;

//将临时的decoded0与decoded1合成送CODE的PCM
void __inline decoded_to_PCM(int32_t* decoded0, int32_t* decoded1,unsigned char *PCM_buffer ,int blockstodecode)
{
		    unsigned char* p;
		    int16_t  sample16;
		    int32_t  sample32;
			int i;


            /* Convert the output samples to WAV format and write to output file */
            p = PCM_buffer;
            if (ape_ctx.bps == 8) {
                for (i = 0 ; i < blockstodecode ; i++)
                {
                    /* 8 bit WAV uses unsigned samples */
                    *(p++) = (decoded0[i] + 0x80) & 0xff;

                    if (ape_ctx.channels == 2) {
                        *(p++) = (decoded1[i] + 0x80) & 0xff;
                    }
                }
            } else if (ape_ctx.bps == 16) {
                for (i = 0 ; i < blockstodecode ; i++)
                {
                    sample16 = decoded0[i];
                    *(p++) = sample16 & 0xff;
                    *(p++) = (sample16 >> 8) & 0xff;

                    if (ape_ctx.channels == 2) {
                        sample16 = decoded1[i];
                        *(p++) = sample16 & 0xff;
                        *(p++) = (sample16 >> 8) & 0xff;
                    }
                }
            } else if (ape_ctx.bps == 24) {
                for (i = 0 ; i < blockstodecode ; i++)
                {
                    sample32 = decoded0[i];
                    *(p++) = sample32 & 0xff;
                    *(p++) = (sample32 >> 8) & 0xff;
                    *(p++) = (sample32 >> 16) & 0xff;

                    if (ape_ctx.channels == 2) {
                        sample32 = decoded1[i];
                        *(p++) = sample32 & 0xff;
                        *(p++) = (sample32 >> 8) & 0xff;
                        *(p++) = (sample32 >> 16) & 0xff;
                    }
                }
            }


}
static rt_err_t ape_decoder_init(void)
{
  PCM_buffer0 = (unsigned char *)rt_malloc(BLOCKS_PER_LOOP*4);
	rt_memset(PCM_buffer0,0,BLOCKS_PER_LOOP*4);
	PCM_buffer1 = (unsigned char *)rt_malloc(BLOCKS_PER_LOOP*4);
	rt_memset(PCM_buffer1,0,BLOCKS_PER_LOOP*4);
  decoded0 = (int32_t *)rt_malloc(BLOCKS_PER_LOOP*4);
	rt_memset(decoded0,0,BLOCKS_PER_LOOP*4);
	decoded1 = (int32_t *)rt_malloc(BLOCKS_PER_LOOP*4);
	rt_memset(decoded1,0,BLOCKS_PER_LOOP*4);
  inbuffer = (unsigned char *)rt_malloc(INPUT_CHUNKSIZE);
	rt_memset(inbuffer,0,INPUT_CHUNKSIZE);
	
	filterbuf64   = (filter_int *)rt_malloc_align((64*3 + FILTER_HISTORY_SIZE) * 2*sizeof(filter_int),16);
	rt_memset(filterbuf64,0,(64*3 + FILTER_HISTORY_SIZE) * 2*sizeof(filter_int));
	filterbuf256  = (filter_int *)rt_malloc_align((256*3 + FILTER_HISTORY_SIZE) * 2*sizeof(filter_int),16);
	rt_memset(filterbuf256,0,(256*3 + FILTER_HISTORY_SIZE) * 2*sizeof(filter_int));
	filterbuf1280 = (filter_int *)rt_malloc_align((1280*3 + FILTER_HISTORY_SIZE) * 2*sizeof(filter_int),16);
	rt_memset(filterbuf1280,0,(1280*3 + FILTER_HISTORY_SIZE) * 2*sizeof(filter_int));
	
	return RT_EOK;
}
static rt_err_t ape_decoder_deinit(void)
{
  rt_free(PCM_buffer0);
	rt_free(PCM_buffer1);
	rt_free(decoded0);
	rt_free(decoded1);
	rt_free(inbuffer);
	rt_free_align(filterbuf64);
  rt_free_align(filterbuf256);
	rt_free_align(filterbuf1280);
	return RT_EOK;
}

//DMA回调函数,当DMA将PCM buffer的内容发完后,会执行此函数
static rt_err_t ape_decoder_tx_done(rt_device_t dev, void *buffer)
{
    /* release ape_sem */
	rt_sem_release(&ape_sem);

	return RT_EOK;
}


int ape(char* path)
{
    int fd;
    int currentframe;
    int nblocks;
    int bytesconsumed;
    int bytesinbuffer;
    int blockstodecode;
    int res;
    int firstbyte;
	int n;
    int crc_errors = 0;
	int cnt = 0;
	rt_device_t snd_device;


	//本次没用内存池..主要因为这种情况下不知道如何使用
	if (rt_sem_init(&ape_sem, "ape_sem", 2, RT_IPC_FLAG_FIFO) != RT_EOK)
		rt_kprintf("init ape_sem semaphore failed\n");


	fd=open(path,O_RDONLY,0);
    if (fd < 0) return -1;

	/* open audio device */
	snd_device = rt_device_find("snd");
	if (snd_device == RT_NULL)
	{
   rt_kprintf("init ape_sem semaphore failed\n");
	}
 		/*  set tx complete call back function 
		 *  设置回调函数,当DMA传输完毕时,会执行ape_decoder_tx_done
		 */
		rt_device_set_tx_complete(snd_device, ape_decoder_tx_done);
		rt_device_open(snd_device, RT_DEVICE_OFLAG_WRONLY);
	
   ape_decoder_init();
    /* Read the file headers to populate the ape_ctx struct */
    if (ape_parseheader(fd,&ape_ctx) < 0) {
        rt_kprintf("Cannot read header\n");
        close(fd);
			  ape_decoder_deinit();
        return -1;
    }

    if ((ape_ctx.fileversion < APE_MIN_VERSION) || (ape_ctx.fileversion > APE_MAX_VERSION)) {
        rt_kprintf("Unsupported file version - %.2f\n", ape_ctx.fileversion/1000.0);
        close(fd);
			ape_decoder_deinit();
        return -2;
    }

    ape_dumpinfo(&ape_ctx);

    currentframe = 0;

    /* Initialise the buffer */
    lseek(fd, ape_ctx.firstframe, SEEK_SET);
    bytesinbuffer = read(fd, inbuffer, INPUT_CHUNKSIZE);
    firstbyte = 3;  /* Take account of the little-endian 32-bit byte ordering */

	//set CODEC's samplerate
	rt_device_control(snd_device, 2, &(ape_ctx.samplerate));


    /* The main decoding loop - we decode the frames a small chunk at a time */
    while (currentframe < ape_ctx.totalframes)
    {

        /* Calculate how many blocks there are in this frame */
        if (currentframe == (ape_ctx.totalframes - 1))
            nblocks = ape_ctx.finalframeblocks;
        else
            nblocks = ape_ctx.blocksperframe;

        ape_ctx.currentframeblocks = nblocks;

        /* Initialise the frame decoder */
        init_frame_decoder(&ape_ctx, inbuffer, &firstbyte, &bytesconsumed);

        /* Update buffer */
        rt_memmove(inbuffer,inbuffer + bytesconsumed, bytesinbuffer - bytesconsumed);
        bytesinbuffer -= bytesconsumed;

        n = read(fd, inbuffer + bytesinbuffer, INPUT_CHUNKSIZE - bytesinbuffer);
        bytesinbuffer += n;

        /* Decode the frame a chunk at a time */
        while (nblocks > 0)
        {
            blockstodecode = MIN(BLOCKS_PER_LOOP, nblocks);

			//尝试获得信号量,如获取不到将挂起.
		 	rt_sem_take(&ape_sem, RT_WAITING_FOREVER);

			 //轮流使用两PCM_buffer
	   		 if(cnt++ & 1)
	   		 {	   
			       if ((res = decode_chunk(&ape_ctx, inbuffer, &firstbyte,
                                   		   &bytesconsumed,
                                   		   decoded0, decoded1,
                                           blockstodecode)) < 0)
            		{
                		/* Frame decoding error, abort */
                		close(fd);
									ape_decoder_deinit();
                		return res;
            		}
			  		decoded_to_PCM(decoded0,decoded1,PCM_buffer0 ,blockstodecode);
		   			rt_device_write(snd_device, 0, (int16_t *)PCM_buffer0, ((BLOCKS_PER_LOOP * 4) ));
		 	 }
			 else
		 	 {
			        if ((res = decode_chunk(&ape_ctx, inbuffer, &firstbyte,
                                      		&bytesconsumed,
                                    		decoded0, decoded1,
                                    		blockstodecode)) < 0)
            		{
                		/* Frame decoding error, abort */
                		close(fd);
									ape_decoder_deinit();
                		return res;
            		}

					decoded_to_PCM(decoded0,decoded1,PCM_buffer1 ,blockstodecode);
		   			rt_device_write(snd_device, 0, (int16_t *)PCM_buffer1, ((BLOCKS_PER_LOOP * 4) ));
		 	 }

            /* Update the buffer */
            memmove(inbuffer,inbuffer + bytesconsumed, bytesinbuffer - bytesconsumed);
            bytesinbuffer -= bytesconsumed;

            n = read(fd, inbuffer + bytesinbuffer, INPUT_CHUNKSIZE - bytesinbuffer);
            bytesinbuffer += n;

            /* Decrement the block count */
            nblocks -= blockstodecode;
        }

        currentframe++;
    }
    
    close(fd);
   ape_decoder_deinit();
    if (crc_errors > 0)
        return -1;
    else
        return 0;
}
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(ape, ape(char* path) );
#endif
