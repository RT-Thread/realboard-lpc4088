/**********************************************************************
* $Id$      decoder.c     2012-11-01    
*//**
* @file     decoder.c
* @brief    This example implements an Interface Radio Device.
* @version  1.0
* @date     01. November. 2012
* @author   NXP MCU SW Application Team
*
* Copyright(C) 2011, NXP Semiconductor
* All rights reserved.
*
***********************************************************************
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
#include "bsp.h"
#include "lpc_i2s.h"
#include "lpc_pinsel.h"
#include "lpc_gpdma.h"
#include "lpc_emac.h"
#include "debug_frmwrk.h"
#include "clock.h"
#include "uda1380.h"
#include "mp3dec.h"
#include "coder.h"
#if (_CURR_USING_BRD == _IAR_OLIMEX_BOARD)
#include "sdram_k4s561632j.h"
#elif (_CURR_USING_BRD == _QVGA_BOARD)
#include "sdram_mt48lc8m32lfb5.h"
#elif (_CURR_USING_BRD == _EA_PA_BOARD)
#include "sdram_is42s32800d.h"
#elif (_CURR_USING_BRD == _RB4088_BOARD)
#include "sdram_h57v2562gtr.h"
#endif

/** @defgroup Mp3_decoder Mp3 Decoder
 * @ingroup Emac_WebRadio
 * @{
 */

/************************** PRIVATE DEFINITIONS *************************/
#define DMA_USED                1
#define TRACE_LOG               0
#define ERROR_LOG               1
#define DEBUG_DATA_READ         0
#define DEBUG_DECODE_TIME       0
#define DEBUG_DATA_WRITE        0

/** Max buffer length */
#define IN_FRAME_MIN_SIZE               (MAINBUF_SIZE)
#define OUT_FRAME_MAX_SIZE              (MAX_NCHAN * MAX_NGRAN * MAX_NSAMP*2)
#if DMA_USED
#define DMA_FRAME_SIZE                  (audio_frame_size)
#endif
#define DECODER_IN_BUFF_SIZE            (MAINBUF_SIZE*20)
#define DECODER_IN_BUFF_START_ADDR      (SDRAM_BASE_ADDR)
#define AUDIO_BUFF_START_ADDR           (DECODER_IN_BUFF_START_ADDR + DECODER_IN_BUFF_SIZE)
#define AUDIO_BUFF_FRAME_NUM            (500) 
#define AUDIO_BUFF_SIZE                 (OUT_FRAME_MAX_SIZE*AUDIO_BUFF_FRAME_NUM)
#define PLAY_BUFF_START_ADDR            (AUDIO_BUFF_START_ADDR + AUDIO_BUFF_SIZE)
#define PLAY_BUFF_MIN_SIZE              (AUDIO_BUFF_SIZE/4)


/* Check buff is full or not */
#define __BUF_IS_FULL(head,tail,size)       ((head%size)== ((tail+1)%size))
/* Check buff will be full in next receiving or not */
#define __BUF_WILL_FULL(head,tail,size)     ((head%size)== ((tail+2)%size))
/* Check buff is empty */
#define __BUF_IS_EMPTY(head,tail,size)      ((head%size)==(tail%size))
/* Reset buff */
#define __BUF_RESET(bufidx)                 (bufidx=0)
#define __BUF_INCR(bufidx,size)             (bufidx=(bufidx+1)%size)
#define __BUF_DATA_NUM(head,tail,size)      ((tail>=head) ? (tail-head):(size-head+tail))
#define __BUF_FREE_DATA_NUM(head,tail,size) (size-1-__BUF_DATA_NUM(head,tail,size))                                        
#define __BUF_INCR_NUM(bufidx,num,size)     (bufidx=(bufidx+num)%size)
#define __AUDIO_BUFF_NUM(buff)              (__BUF_DATA_NUM((buff)->head,(buff)->tail,(buff)->buffer_size))
#define __AUDIO_FREE_NUM(buff)              (__BUF_FREE_DATA_NUM((buff)->head,(buff)->tail,(buff)->buffer_size))
#define __PLAY_READ_SIZE()                  (audio_frame_num*audio_frame_size)

/************************** PRIVATE VARIABLES ***********************/
typedef struct
{
    uint32_t head;
    uint32_t tail;
    uint8_t  *buffer;
    uint32_t  buffer_size;
} RING_BUFFER;

typedef RING_BUFFER       AUDIO_BUFFER;
AUDIO_BUFFER              audio_buffer;

__IO uint8_t    play_setup_stat = 0;
#if DMA_USED
__IO uint32_t   play_dma_size = 0;
#else
__IO uint32_t   play_cur_ofs;
__IO uint32_t   play_end_ofs;
#endif
__IO FlagStatus play_int_stat = RESET;

HMP3Decoder     mp3_decoder = 0;
MP3FrameInfo    mp3_frame_info; 
uint32_t        read_ptr = DECODER_IN_BUFF_START_ADDR;
int32_t         input_frame_size = 0;
uint32_t        audio_frame_size = 0;
uint32_t        audio_frame_num = 0;

#if DEBUG_DATA_READ
static clock_time_t rticks_1s;
static int64_t rsamples_cnt = -1;
#endif /*_DEBUG_FREQ*/
#if DEBUG_DATA_WRITE
static clock_time_t wticks_1s;
static int64_t wsamples_cnt = -1;
#endif
#if DEBUG_DECODE_TIME
static clock_time_t decode_ticks;
static int64_t dsamples_cnt = -1;
#endif
/************************** PRIVATE FUNCTIONS *************************/
#if DMA_USED
GPDMA_Channel_CFG_Type GPDMACfg;
void DMA_Send(void);
#else
void I2S_IRQHandler(void);
#endif
void     AudioBuf_Init(AUDIO_BUFFER *buff);
uint32_t AudioBuf_GetWritePtr(AUDIO_BUFFER *buff, uint32_t size);
void     AudioBuf_IncWritePtr(AUDIO_BUFFER *buff, uint32_t size);
uint32_t AudioBuf_Read(AUDIO_BUFFER *buff, uint8_t* data, uint32_t size);
Status   Play_Setup(void);
void     Play_Control(uint8_t start);
int      Decoder_Decode(void);

/*********************************************************************//**
 * @brief       Initialize audio buffer
 * @param[in]   Audio Buffer
 * @return      None
 **********************************************************************/
void AudioBuf_Init(AUDIO_BUFFER *buff)
{
    __BUF_RESET(buff->head);
    __BUF_RESET(buff->tail);
    buff->buffer = (uint8_t*)AUDIO_BUFF_START_ADDR;
    buff->buffer_size = AUDIO_BUFF_SIZE;
}
/*********************************************************************//**
 * @brief       Get address for writting
 * @param[in]   buffer size.
 * @return      the number of written bytes.
 **********************************************************************/
uint32_t AudioBuf_GetWritePtr(AUDIO_BUFFER *buff, uint32_t size)
{
    if(__AUDIO_FREE_NUM(buff) >= size)
    {
        return (uint32_t)&buff->buffer[buff->tail];
    }
    return 0;
}
/*********************************************************************//**
 * @brief       Increase write pointer
 * @param[in]   buffer size.
 * @return      the number of written bytes.
 **********************************************************************/
void AudioBuf_IncWritePtr(AUDIO_BUFFER *buff, uint32_t size)
{
    __BUF_INCR_NUM(buff->tail,size,buff->buffer_size);
#if DEBUG_DATA_WRITE        
    {
        clock_time_t cur_ticks = clock_time();
        if(wsamples_cnt == -1)
        {
            wticks_1s = cur_ticks;
            wsamples_cnt = 0;
        }
        wsamples_cnt += mp3_frame_info.outputSamps/mp3_frame_info.nChans;
        
        if((cur_ticks - wticks_1s) >= 10*CLOCK_CONF_SECOND)
        {  
            _DBG("Receive: ");_DBD32(((uint64_t)wsamples_cnt*CLOCK_CONF_SECOND)/(cur_ticks - wticks_1s));_DBG_("samples/s.");
            wticks_1s = clock_time();
            wsamples_cnt = 0;
        }
    }
#endif /*DEBUG_DATA_WRITE*/      
}

/*********************************************************************//**
 * @brief       Read data from audio buffer
 * @param[in]   Audio Buffer, buffer pointer and buffer size.
 * @return      The number of read bytes.
 **********************************************************************/
uint32_t AudioBuf_Read(AUDIO_BUFFER *buff, uint8_t* data, uint32_t size)
{
    int n_rbytes = 0;
    
    if(__AUDIO_BUFF_NUM(buff) < size)
        size = __AUDIO_BUFF_NUM(buff);
    for(n_rbytes = 0; n_rbytes < size; n_rbytes++)
    {
        data[n_rbytes] = buff->buffer[buff->head];
        __BUF_INCR_NUM(buff->head, 1, buff->buffer_size);
    }
 
    return n_rbytes;
}
/*********************************************************************//**
 * @brief       Playback setup
 * @param[in]   none
 * @return      None
 **********************************************************************/
Status Play_Setup(void)
{
    I2S_CFG_Type I2S_SetupStruct;
    I2S_MODEConf_Type I2S_ClkConfig;
#if DMA_USED    
    I2S_DMAConf_Type I2S_DMAStruct;
#endif    
    uint32_t samplerate = 0;
    uint32_t compress = 0;
    if(play_setup_stat)
        return SUCCESS;  
    
    _DBG_("[DEBUG]Mp3 frame info:");
    _DBG("[DEBUG]Wordwidth = ");_DBD(mp3_frame_info.bitsPerSample);_DBG_("");
    _DBG("[DEBUG]Channels = ");_DBD(mp3_frame_info.nChans);_DBG_("");
    _DBG("[DEBUG]Sample Rate = ");_DBD32(mp3_frame_info.samprate);_DBG_("");
    switch(mp3_frame_info.version)
    {
        case MPEG1:
            _DBG("[DEBUG]Version = MPEG1");
            break;
        case MPEG2:
            _DBG("[DEBUG]Version = MPEG2");
            break;
        case MPEG25:
            _DBG("[DEBUG]Version = MPEG2.5");
            break;
        default:
            _DBG_("[DEBUG]Unsupported version.");
            return ERROR;
    }
    _DBG(" layer ");_DBD(mp3_frame_info.layer);_DBG_("");
    _DBG("[DEBUG]Bitrate = ");_DBD32(mp3_frame_info.bitrate);_DBG_("bit/s");
    
    /* Audio Config*/
    if(mp3_frame_info.bitsPerSample == 8)
        I2S_SetupStruct.wordwidth = I2S_WORDWIDTH_8;
    else if (mp3_frame_info.bitsPerSample == 16)
        I2S_SetupStruct.wordwidth = I2S_WORDWIDTH_16;
    else if (mp3_frame_info.bitsPerSample == 32)
        I2S_SetupStruct.wordwidth = I2S_WORDWIDTH_32;
    else
        goto setup_err;
    if(mp3_frame_info.nChans == 1)
        I2S_SetupStruct.mono = I2S_MONO;
    else if (mp3_frame_info.nChans == 2)
        I2S_SetupStruct.mono = I2S_STEREO;
    else
        goto setup_err;
    I2S_SetupStruct.stop = I2S_STOP_ENABLE;
    I2S_SetupStruct.reset = I2S_RESET_ENABLE;
    I2S_SetupStruct.ws_sel = I2S_MASTER_MODE;
    I2S_SetupStruct.mute = I2S_MUTE_DISABLE;
    I2S_Config(LPC_I2S,I2S_TX_MODE,&I2S_SetupStruct);

    /* Clock Mode Config*/
    I2S_ClkConfig.clksel = I2S_CLKSEL_FRDCLK;
    I2S_ClkConfig.fpin = I2S_4PIN_DISABLE;
    I2S_ClkConfig.mcena = I2S_MCLK_ENABLE;
    I2S_ModeConfig(LPC_I2S,&I2S_ClkConfig,I2S_TX_MODE);
             

    if(mp3_frame_info.nChans == 1)
        samplerate = mp3_frame_info.samprate*2;
    else
        samplerate = mp3_frame_info.samprate;
    if( samplerate > 96000)
        goto setup_err;
    if(I2S_FreqConfig(LPC_I2S, samplerate*17/16, I2S_TX_MODE) != SUCCESS)
        goto setup_err;
    
    compress = samplerate*mp3_frame_info.bitsPerSample*mp3_frame_info.nChans*100/mp3_frame_info.bitrate;
    audio_frame_num = IN_FRAME_MIN_SIZE*compress/(100*audio_frame_size);
    I2S_Stop(LPC_I2S, I2S_TX_MODE);

    /* TX FIFO depth is 4 */
    I2S_IRQConfig(LPC_I2S,I2S_TX_MODE,4);
    I2S_IRQCmd(LPC_I2S,I2S_TX_MODE,ENABLE);
    
    Uda1380_Init(100000, mp3_frame_info.samprate);
    _DBG_("[DEBUG]I2S configured.");
 
#if DMA_USED
      GPDMA_Init();
    /*
     * Configure GPDMA channel 0 -------------------------------------------------------------
     * Used for I2S Transmit
     */
    // Setup GPDMA channel --------------------------------
    // channel 0
    GPDMACfg.ChannelNum = 0;
    // Source memory
    GPDMACfg.SrcMemAddr = 0;
    // Destination memory
    GPDMACfg.DstMemAddr = 0;
    // Transfer size
    GPDMACfg.TransferSize = 0;
    // Transfer width - unused
    GPDMACfg.TransferWidth = 0;
    // Transfer type
    GPDMACfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
    // Source connection
    GPDMACfg.SrcConn = 0;
    // Destination connection - unused
    GPDMACfg.DstConn = GPDMA_CONN_I2S_Channel_0;
    // Linker List Item - unused
    GPDMACfg.DMALLI = 0;
    GPDMA_Setup(&GPDMACfg);
      
    I2S_DMAStruct.DMAIndex = I2S_DMA_1;
    I2S_DMAStruct.depth = 1;
    I2S_DMAConfig(LPC_I2S, &I2S_DMAStruct, I2S_TX_MODE);
    I2S_DMACmd(LPC_I2S, I2S_DMA_1, I2S_TX_MODE, ENABLE);
      
    /* Setting GPDMA interrupt */
    // Disable interrupt for DMA
    NVIC_DisableIRQ (DMA_IRQn);
    /* preemption = 1, sub-priority = 1 */
    NVIC_SetPriority(DMA_IRQn, 0x01);
    /* Enable interrupt for DMA */
    NVIC_EnableIRQ (DMA_IRQn);
#endif    
    I2S_Start(LPC_I2S);
    play_setup_stat = 1;
    return SUCCESS;
    
setup_err:
#if ERROR_LOG
    _DBG_("[DEBUG]I2S Setup error!!!");
#endif    
    return ERROR;
}
/*********************************************************************//**
 * @brief       Start/Stop playback
 * @param[in]   0: stop, 1: start
 * @return      None
 **********************************************************************/
void Play_Control(uint8_t start)
{
    static volatile uint8_t inFunc = 0;
    if(inFunc)
        return;
    inFunc = 1;
    do
    {
        if(start)
        {
            if((play_int_stat == SET) ||
                (PLAY_BUFF_MIN_SIZE > __AUDIO_BUFF_NUM(&audio_buffer)))
                break;        
            play_int_stat = SET;
        
            if(Play_Setup() == SUCCESS)
            {
#if DMA_USED            
                DMA_Send();
#else
                I2S_Start(LPC_I2S);
                NVIC_EnableIRQ(I2S_IRQn);
#endif            
            }
        }
        else
        {
            if(play_int_stat == RESET)
                break;
            
            play_int_stat = RESET;
        
#if !DMA_USED    
            NVIC_DisableIRQ(I2S_IRQn);
            I2S_Stop(LPC_I2S, I2S_TX_MODE);
#endif    
#if ERROR_LOG 
        {        
            clock_time_t cur_ticks = clock_time();
            _DBD32(cur_ticks);_DBG_(": Bufferring...");
        }
#endif    
        }
    }while(0);
    inFunc = 0;
}
/*----------------- INTERRUPT SERVICE ROUTINES --------------------------*/
#if DMA_USED
/*********************************************************************//**
 * @brief        GPDMA interrupt handler sub-routine
 * @param[in]    None
 * @return       None
 **********************************************************************/
void DMA_IRQHandler (void)
{
    // check GPDMA interrupt on channel 0
    if (GPDMA_IntGetStatus(GPDMA_STAT_INT, 0)){ //check interrupt status on channel 0
        // Check counter terminal status
        if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0)){
            // Clear terminate counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTTC, 0); 
#if DEBUG_DATA_READ   
            {
                clock_time_t cur_ticks = clock_time();
                
                rsamples_cnt += mp3_frame_info.outputSamps*((play_dma_size*4)/audio_frame_size)/mp3_frame_info.nChans;
                if (cur_ticks >= (rticks_1s + 10*CLOCK_CONF_SECOND))
                {
                    _DBG("Play: ");_DBD32(((uint64_t)rsamples_cnt*CLOCK_CONF_SECOND)/(cur_ticks - rticks_1s ));_DBG_("samples/s");
                    rticks_1s = clock_time();
                    rsamples_cnt = 0;
                }
            }
#endif           
            play_dma_size = 0;  
            DMA_Send();
        }
        // Check error terminal status
        if (GPDMA_IntGetStatus(GPDMA_STAT_INTERR, 0)){
            // Clear error counter Interrupt pending
            GPDMA_ClearIntPending (GPDMA_STATCLR_INTERR, 0);
            __BUF_INCR_NUM(audio_buffer.head,play_dma_size*4,audio_buffer.buffer_size) ;
            play_dma_size = 0;
#if ERROR_LOG
            _DBG_("DMA Error!!!");
#endif            
            DMA_Send();
        }
    }
    
}
/*********************************************************************//**
 * @brief        Send data to I2S through DMA
 * @param[in]    None
 * @return       None
 **********************************************************************/
void DMA_Send(void)
{
    static volatile char in_func = 0;
    uint32_t data_num;
    
    if(in_func)
        return;
    
    in_func = 1;
    do
    {
        if(play_dma_size || (audio_frame_size == 0) || (audio_frame_num == 0))
            break;   
        
        GPDMA_ChannelCmd(0, DISABLE);
        
        data_num =  __AUDIO_BUFF_NUM(&audio_buffer);
        if(data_num < DMA_FRAME_SIZE)
        {
#if TRACE_LOG  
           _DBG_("Audio buffer underflow!");
#endif          
            Play_Control(0);
            break;
        }
       
        if(data_num >= __PLAY_READ_SIZE())
            data_num = __PLAY_READ_SIZE();
        play_dma_size = (data_num/4);
        if(play_dma_size > 0xFFF)
            play_dma_size = 0xFFF;
        AudioBuf_Read(&audio_buffer, (uint8_t*)PLAY_BUFF_START_ADDR, play_dma_size*4);

#if DEBUG_DATA_READ   
    {
        clock_time_t cur_ticks = clock_time();
        if(rsamples_cnt == -1)
        {
            rticks_1s = cur_ticks;
            rsamples_cnt = 0;
        }
    }
#endif                
        
#if TRACE_LOG     
        _DBG("Send ");_DBD32(play_dma_size*4/audio_frame_size);_DBG_(" frames to I2S");
#endif
        
        // Source memory
        GPDMACfg.SrcMemAddr = PLAY_BUFF_START_ADDR;
#if ERROR_LOG    
        if(GPDMACfg.SrcMemAddr%4)
            _DBG_("Invalid DMA Source Address");
#endif    
        // Transfer size
        GPDMACfg.TransferSize = play_dma_size;
        
        GPDMA_SetupTransfer(&GPDMACfg);
           
        GPDMA_ChannelCmd(0, ENABLE);
    }while(0);
    in_func = 0;
}
#else

/*********************************************************************//**
 * @brief       I2S IRQ Handler, call to send data to transmit buffer
 * @param[in]   None
 * @return      None
 **********************************************************************/
void I2S_IRQHandler()
{
    uint32_t txlevel,i;
    //uint32_t n_rbytes = 0;
    txlevel = I2S_GetLevel(LPC_I2S,I2S_TX_MODE);
    
    if(play_cur_ofs >= play_end_ofs)
    {
        uint32_t data_num = __AUDIO_BUFF_NUM(&audio_buffer);
        if(data_num > __PLAY_READ_SIZE())
            data_num = __PLAY_READ_SIZE();
        data_num = (data_num/4)*4;
        if(data_num == 0)
        {
            Play_Control(0);
            return;
        }
        AudioBuf_Read(&audio_buffer, (uint8_t*)PLAY_BUFF_START_ADDR, data_num);
        play_cur_ofs = 0;
        play_end_ofs = data_num;
    }
    for(i=0;i<8-txlevel;i++)
    {
        LPC_I2S->TXFIFO = *((uint32_t*)(PLAY_BUFF_START_ADDR + play_cur_ofs));
        play_cur_ofs+= 4;
        //n_rbytes += 4;
    }
    //_DBG("Play ");_DBD16(n_rbytes);_DBG_(" bytes.");
}
#endif

/*-------------------------PUBLIC FUNCTION------------------------------*/
/*********************************************************************//**
 * @brief       Init decoder
 * @param[in]   None
 * @return      None
 **********************************************************************/
Status Decoder_Init (void) {                      

    AudioBuf_Init(&audio_buffer);

    /* Initialize I2S peripheral ------------------------------------*/
    /* Pin configuration:
     * Assign:  - P0.7 as I2STX_CLK
     *          - P0.8 as I2STX_WS
     *          - P0.9 as I2STX_SDA
     *          - P1.16 as I2SMCLK
     */
    PINSEL_ConfigPin(0,7,1);
    PINSEL_ConfigPin(0,8,1);
    PINSEL_ConfigPin(0,9,1);
    PINSEL_ConfigPin(1,16,2);
#if (_CURR_USING_BRD == _RB4088_BOARD)
    /* Pin configuration:
     * Assign:  - P0.6 as I2SRX_SDA
     *          - P0.23 as I2SRX_SCK
     *          - P0.24 as I2SRX_WS
     */
    PINSEL_ConfigPin(0,6,1);
    PINSEL_ConfigPin(0,23,2);
    PINSEL_ConfigPin(0,24,2);
#endif
    _DBG_("[DEBUG]Init I2S interface");
    I2S_Init(LPC_I2S);
    
    _DBG_("[DEBUG]Init MP3 decoder");
    mp3_decoder = MP3InitDecoder();
    if(mp3_decoder == 0)
    {
#if ERROR_LOG        
      _DBG_("[DEBUG]Cannot init the MP3 Decoder");
#endif        
        return ERROR;
    }
    _DBG_("[DEBUG]Init SDRAM");
    SDRAMInit();
    
    return SUCCESS;
}
/*********************************************************************//**
 * @brief       Close decoder
 * @param[in]   None
 * @return      None
 **********************************************************************/
void Decoder_Close(void)
{
    MP3FreeDecoder(mp3_decoder);
    mp3_decoder = 0;
    I2S_Stop(LPC_I2S, I2S_TX_MODE);
    I2S_DeInit(LPC_I2S);
}
/*********************************************************************//**
 * @brief       Add MP3 data to input buffer
 * @param[in]   buffer address and buffer size
 * @return      None
 **********************************************************************/
void Decoder_FillInputBuffer(uint8_t* data, uint32_t size)
{   
    if(read_ptr != DECODER_IN_BUFF_START_ADDR)
    {
        if(input_frame_size) 
        {
            memmove((char*)(DECODER_IN_BUFF_START_ADDR),
                     (char*)read_ptr,
                     input_frame_size);
        }
        read_ptr = DECODER_IN_BUFF_START_ADDR; 
    }
    
    if(size == 0)
        return;      
    
    // Input buffer underflow?
    if((input_frame_size + size) >= DECODER_IN_BUFF_SIZE)
    {
#if ERROR_LOG        
        _DBG_("Input Buffer Overflow!");
#endif       
        Decoder_Decode();
        return;
    }
  
    if(size)
    {
        memcpy((char*)(DECODER_IN_BUFF_START_ADDR + input_frame_size),
                data,
                size);
        input_frame_size += size;
    } 
#if TRACE_LOG    
    if(size) {
    _DBG("Add ");_DBD32(size);
    _DBG(" bytes to buffer, current buffer size: ");_DBD32(input_frame_size);_DBG_("");
    }
#endif    
    Decoder_Decode();
}
/*********************************************************************//**
 * @brief       Decoder data
 * @param[in]   None
 * @return      None
 **********************************************************************/
int Decoder_Decode(void)
{
    int err;
    int samples = 0;
    int sync_word = 0;
    unsigned char* decode_out_buf = 0;   
    unsigned char exit_flg = 0;
#if TRACE_LOG   
    int32_t in_frame_size_bef = 0;
#endif    
#if DEBUG_DECODE_TIME
    clock_time_t dstart_ticks = clock_time();
    clock_time_t dend_ticks;
     
    if(dsamples_cnt == -1)
    {
        decode_ticks = 0;
        dsamples_cnt = 0;
    }

#endif        
#if TRACE_LOG        
    in_frame_size_bef = input_frame_size;
#endif    
    while((input_frame_size >= IN_FRAME_MIN_SIZE) &&
          (exit_flg == 0)){ 
        // Find the start of actual MP3 data
        sync_word = MP3FindSyncWord((unsigned char*)read_ptr, input_frame_size);
        
        if(sync_word < 0)
        {
#if ERROR_LOG            
            _DBG_("Invalid MP3 data (No Sync Word found).");
#endif            
            input_frame_size = 0;
            read_ptr = DECODER_IN_BUFF_START_ADDR;
            return 0;
        }
        
        input_frame_size -= sync_word;
        read_ptr += sync_word;
#if TRACE_LOG        
        _DBG("Found sync word at ");_DBD32(sync_word);_DBG_("");
#endif    
        decode_out_buf = (unsigned char*)AudioBuf_GetWritePtr(&audio_buffer, audio_frame_size); 
        if(decode_out_buf == 0)
        {
#if TRACE_LOG  
          _DBG_("Audio Buffer overflow!");
#endif           
          return 0;
        }
       
        err = MP3Decode(mp3_decoder, (unsigned char**)&read_ptr, 
                        (int*)&input_frame_size, (short *)decode_out_buf, 0);      
       
        if(err)
        {
            switch(err)
            {
                case ERR_MP3_INDATA_UNDERFLOW:  
#if TRACE_LOG  
                    _DBG_("Input data underflow.");
#endif                
                    input_frame_size = 0;
                    read_ptr = DECODER_IN_BUFF_START_ADDR;
                    exit_flg = 1;
                    break;
                case ERR_MP3_MAINDATA_UNDERFLOW:
#if TRACE_LOG                    
                    _DBG_("-->Need more MP3 data");
#endif                
                    exit_flg = 1;
                    break;
                case ERR_MP3_INVALID_FRAMEHEADER:
#if TRACE_LOG                    
                    _DBG_("Invalid MP3 data (Invalid Frame header)!");
#endif                
                    input_frame_size--;
                    read_ptr++;
                    break;
                default:   
#if TRACE_LOG                    
                    _DBG("Decode failed! (Error code: ");_DBH(err);_DBG_(").");
#endif                
                    break;
            }
        }
        else
        {  
            // Get frame info
            MP3GetLastFrameInfo(mp3_decoder, &mp3_frame_info);    
            audio_frame_size = (mp3_frame_info.bitsPerSample / 8) * mp3_frame_info.outputSamps;                       
            AudioBuf_IncWritePtr(&audio_buffer, audio_frame_size); 
            samples += mp3_frame_info.outputSamps/mp3_frame_info.nChans;
            Play_Control(1);
            exit_flg = 1;
        }
   }  
#if TRACE_LOG    
    if(in_frame_size_bef > input_frame_size) {
    _DBG("Remove ");_DBD32(in_frame_size_bef - input_frame_size);
    _DBG(" bytes from buffer, remain buffer size = ");_DBD32(input_frame_size);_DBG_(" bytes");
    }
#endif
#if DEBUG_DECODE_TIME 
   {
       dend_ticks = clock_time();
       dsamples_cnt += samples;
       decode_ticks += dend_ticks - dstart_ticks; 
       if(decode_ticks >= 10*CLOCK_CONF_SECOND)
        {  
            _DBG("Decode ");_DBD32(decode_ticks*1000/dsamples_cnt);_DBG_(" microseconds/sample.");
            decode_ticks = 0;
            dsamples_cnt = 0;
        }   
    }
#endif  /*DEBUG_DECODE_TIME*/

   return samples;
}

/**
 * @}
 */
