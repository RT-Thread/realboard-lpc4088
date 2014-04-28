#include "board.h"
#include "drv_uda1380.h"
#include "lpc_i2s.h"

//#define USE_DMA
#define DATA_NODE_MAX 5
/* data node for Tx Mode */
struct codec_data_node
{
    rt_uint16_t *data_ptr;
    rt_size_t  data_size;
	rt_size_t  data_offset;
};
struct codec_device
{
    /* inherit from rt_device */
    struct rt_device parent;
    /* pcm data list */
    struct codec_data_node data_list[DATA_NODE_MAX];
    rt_uint16_t read_index, put_index;
    /* i2c mode */
    struct rt_i2c_bus_device * i2c_device;
};
struct codec_device codec;
static rt_uint16_t volume=0x8080;


static rt_err_t uda1380_read_reg(rt_uint8_t reg, rt_uint16_t *data)
{
    struct rt_i2c_msg msg[2];
	rt_uint8_t send_buf[1];
    rt_uint8_t recv_buf[2];
    send_buf[0] = reg;

    msg[0].addr  = UDA1380_SLAVE_ADDR;
    msg[0].flags = RT_I2C_WR;
    msg[0].len   = 1;
    msg[0].buf   = send_buf;
	
	  msg[1].addr  = UDA1380_SLAVE_ADDR;
    msg[1].flags = RT_I2C_RD;
    msg[1].len   = 2;
    msg[1].buf   = recv_buf;
	
    rt_i2c_transfer(codec.i2c_device, msg, 2);
	  *data=recv_buf[0];
	  *data<<=8;
		*data|= recv_buf[1];
		return RT_EOK;
}
static rt_err_t uda1380_write_reg(rt_uint8_t reg, rt_uint16_t data)
{
    struct rt_i2c_msg msg;
	rt_uint8_t send_buf[UDA1380_CMD_BUFF_SIZE];
	
    send_buf[0] = reg;
	  send_buf[1] =((data >> 8)&0xff);
    send_buf[2] =(data & 0xFF);
	
    msg.addr  = UDA1380_SLAVE_ADDR;
    msg.flags = RT_I2C_WR;
    msg.len   = UDA1380_CMD_BUFF_SIZE;
    msg.buf   = send_buf;
	
	  rt_i2c_transfer(codec.i2c_device, &msg, 1);

		return RT_EOK;
}



static rt_err_t uda1380_i2s_config(void)
{ 
  /* enable I2S power and clock */  
	LPC_SC->PCONP |= (1 << 27);

	LPC_IOCON->P0_7 &= ~0x07;	// TX_SCK
	LPC_IOCON->P0_7 |= 0x01;	
	LPC_IOCON->P0_8 &= ~0x07;	// TX_WS
	LPC_IOCON->P0_8 |= 0x01;
	LPC_IOCON->P0_9 &= ~0x07;	// TX_DATA
	LPC_IOCON->P0_9 |= 0x01;

	LPC_IOCON->P1_16 &= ~0x1F;	// TX_MCLK
	LPC_IOCON->P1_16 |= (0x01 << 1);

	// MCLK for TX
	LPC_I2S->TXMODE = (1<<3);	// generated

	// MCLK: 15MHz
	LPC_I2S->TXRATE = (1<<8) | (4);


	// Bitclock: 22321(22.05k)
	LPC_I2S->TXBITRATE = 20;

	/* stop and reset I2S FIFO */
	LPC_I2S->DAO = (1<<4) | (1<<3);
	LPC_I2S->DAI = (1<<4) | (1<<3);

	/* config the I2S Data Input reg */
	LPC_I2S->DAI = 0x01              //config wordwidth to 16bit
	              | (15<<6);         //set up the clock for 16bit
	/* config the I2S Data Output reg */
	LPC_I2S->DAO = 0x1 | (15<<6);

 rt_thread_delay(5);
 
 #ifdef USE_DMA
 
 #else
   /* config I2S FIFO interrupt to 4x32bit */
	 LPC_I2S->IRQ |= (4 << 16);
	 /* enable the I2S TX Mode interrupt */
   LPC_I2S->IRQ |= 0x02;
#endif
	
	return RT_EOK;
}


static rt_err_t codec_init(rt_device_t dev)
{   
    /* software Reset */
  uda1380_write_reg(UDA1380_SOFT_RESET,0x00);
  	  /* power up the uda1380 */
	uda1380_write_reg(UDA1380_PW_CONTROL, 0xA5DF);
	uda1380_write_reg(UDA1380_EVALMODE_CLK, 0x0F39);
	uda1380_write_reg(UDA1380_I2S_BUS_IO, 0x00 );
	uda1380_write_reg(UDA1380_ANALOG_MIX, 0x0000);
  uda1380_write_reg(UDA1380_HEADHONE_AMP, 0x0202 );
	uda1380_write_reg(UDA1380_MIXER_VOLUME, 0x0000 );
	uda1380_write_reg(UDA1380_MODE_SELECT, 0x5515 );
  uda1380_write_reg(UDA1380_MASTER_CHAN_MUTE, 0x00 );
	uda1380_write_reg(UDA1380_MIXER_DETECT_OVER, 0x00 );
  uda1380_write_reg(UDA1380_DECIM_VOLUM, 0x0000 );
	uda1380_write_reg(UDA1380_PGA, 0x0000 );
	uda1380_write_reg(UDA1380_ADC, 0x0f02 );
  uda1380_write_reg(UDA1380_AGC, 0x00 );
	
    return RT_EOK;
}

void uda1380_volume(rt_uint16_t value) // 0~100
{
	 volume=value;
    /* set the volume*/
    uda1380_write_reg(UDA1380_MASTER_VOLUME, volume);
}
/* Exported functions */
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(uda1380_volume, Set uda1038 volume);
#endif

rt_err_t uda1380_mute(rt_bool_t enable)
{
    rt_uint16_t tmp;
    rt_err_t ret;
    /* backup old value setting of clock */
    ret = uda1380_read_reg(UDA1380_EVALMODE_CLK,&tmp);
    if(ret != RT_EOK)
		return ret;
    /* Use sysclk */
    ret = uda1380_write_reg(UDA1380_EVALMODE_CLK, tmp & (~EVALCLK_DAC_SEL_WSPLL));
    if(ret != RT_EOK)
		return ret;

    if(enable==RT_TRUE)
    {
       ret = uda1380_write_reg(UDA1380_MASTER_CHAN_MUTE,0x0202);
    }
    else
    {
       ret = uda1380_write_reg(UDA1380_MASTER_CHAN_MUTE,0x4808);
    }
	if(ret != RT_EOK)
		return ret;
	
     /* Use sysclk */
    ret = uda1380_write_reg(UDA1380_EVALMODE_CLK, tmp);
    if(ret != RT_EOK)
		return ret;

    return RT_EOK;
}
/* Exported functions */
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(uda1380_mute, turn on/off mute);
#endif
rt_err_t sample_rate(int sr)
{
   rt_uint8_t clk;
	 rt_err_t ret = RT_ERROR;

   if(sr >= 6250 && sr < 12500)
		clk = EVALCLK_WSPLL_SEL6_12K;
	else if(sr >= 12501 && sr < 25000)  
		clk = EVALCLK_WSPLL_SEL12_25K;
	else if(sr >= 25001 && sr < 50000)
		clk = EVALCLK_WSPLL_SEL25_50K;
	else if(sr >= 50001 && sr < 100000)
		clk = EVALCLK_WSPLL_SEL50_100K;
	else
		clk= 0;
		
	ret = uda1380_write_reg(UDA1380_EVALMODE_CLK, 
 	                 EVALCLK_DEC_EN | EVALCLK_DAC_EN | EVALCLK_INT_EN | EVALCLK_DAC_SEL_WSPLL | clk);
	if(ret != RT_EOK)
		return ret;
     I2S_FreqConfig(LPC_I2S, sr, I2S_TX_MODE);

    return RT_EOK;
}

/* Exported functions */
#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(sample_rate, Set sample rate);
#endif

static rt_err_t codec_open(rt_device_t dev, rt_uint16_t oflag)
{   
	  /* restore the volume */
    uda1380_write_reg(UDA1380_MASTER_VOLUME, volume);
    return RT_EOK;
}

static rt_err_t codec_close(rt_device_t dev)
{

      I2S_Stop(LPC_I2S,I2S_TX_MODE);
	    NVIC_DisableIRQ(I2S_IRQn);
	    /* turn off the volume*/
    uda1380_write_reg(UDA1380_MASTER_VOLUME, 0xffff);
        /* remove all data node */
        if (codec.parent.tx_complete != RT_NULL)
        {
            rt_base_t level = rt_hw_interrupt_disable();

            while (codec.read_index != codec.put_index)
            {
                codec.parent.tx_complete(&codec.parent, codec.data_list[codec.read_index].data_ptr);
                codec.read_index++;
                if (codec.read_index >= DATA_NODE_MAX)
                {
                    codec.read_index = 0;
                }
            }

            rt_hw_interrupt_enable(level);
        }


    return RT_EOK;
}

static rt_err_t codec_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    rt_err_t result = RT_EOK;

    switch (cmd)
    {
    case CODEC_CMD_RESET:
        result = codec_init(dev);
        break;

    case CODEC_CMD_VOLUME:
        uda1380_volume(*((uint16_t*) args));
        break;

    case CODEC_CMD_SAMPLERATE:
        result = sample_rate(*((int*) args));
        break;

    default:
        result = RT_ERROR;
    }
    return result;
}

static rt_size_t codec_write(rt_device_t dev, rt_off_t pos,
                             const void* buffer, rt_size_t size)
{
    struct codec_device* device;
    struct codec_data_node* node;
    rt_uint32_t level;
    rt_uint16_t next_index;
    //rt_kprintf("write to uda1380\n");
    device = (struct codec_device*) dev;
    RT_ASSERT(device != RT_NULL);

    next_index = device->put_index + 1;
    if (next_index >= DATA_NODE_MAX)
        next_index = 0;

    /* check data_list full */
    if (next_index == device->read_index)
    {
			 rt_kprintf("data_list full\n");
        rt_set_errno(-RT_EFULL);
        return 0;
    }

    level = rt_hw_interrupt_disable();
    node = &device->data_list[device->put_index];
    device->put_index = next_index;

    /* set node attribute */
    node->data_ptr = (rt_uint16_t*) buffer;
    node->data_size = size >> 1; /* size is byte unit, convert to half word unit */
    node->data_offset=0;
    next_index = device->read_index + 1;
    if (next_index >= DATA_NODE_MAX)
        next_index = 0;
     // rt_kprintf("next:%d,put:%d\n",next_index,device->put_index);
    /* check data list whether is empty */
    if (next_index == device->put_index)
    {  
        I2S_Start(LPC_I2S);
        NVIC_EnableIRQ(I2S_IRQn);   
    }
    rt_hw_interrupt_enable(level);

    return size;
}

rt_err_t codec_hw_init(const char * i2c_bus_device_name)
{
    struct rt_i2c_bus_device * i2c_device;

    i2c_device = rt_i2c_bus_device_find(i2c_bus_device_name);
    if(i2c_device == RT_NULL)
    {
        rt_kprintf("i2c bus device %s not found!\r\n", i2c_bus_device_name);
        return -RT_ENOSYS;
    }
    codec.i2c_device = i2c_device;
    uda1380_i2s_config();
    codec.parent.type = RT_Device_Class_Sound;
    codec.parent.rx_indicate = RT_NULL;
    codec.parent.tx_complete = RT_NULL;
    codec.parent.user_data   = RT_NULL;

    codec.parent.control = codec_control;
    codec.parent.init    = codec_init;
    codec.parent.open    = codec_open;
    codec.parent.close   = codec_close;
    codec.parent.read    = RT_NULL;
    codec.parent.write   = codec_write;

    /* set read_index and put index to 0 */
    codec.read_index = 0;
    codec.put_index = 0;
    //codec_init(&codec.parent);
	  sample_rate(44100);
    /* register the device */
    rt_device_register(&codec.parent, "snd", RT_DEVICE_FLAG_WRONLY | RT_DEVICE_FLAG_DMA_TX);
		return RT_EOK;//codec_init(&codec.parent);
}

static void codec_i2s_isr(void)
{
    void* data_ptr;
      uint32_t txlevel,i;
    /* enter interrupt */
    rt_interrupt_enter();
 if(codec.data_list[codec.read_index].data_size<=codec.data_list[codec.read_index].data_offset)
	 {
		 data_ptr=codec.data_list[codec.read_index].data_ptr;
	  codec.read_index ++;
    if (codec.read_index >= DATA_NODE_MAX)
        codec.read_index = 0;	
     /* notify transmitted complete. */
    if (codec.parent.tx_complete != RT_NULL)
    {
        codec.parent.tx_complete(&codec.parent, data_ptr);
    }		
	}
	txlevel = I2S_GetLevel(LPC_I2S,I2S_TX_MODE);
	if(txlevel <= 4)
	{ 
		if(codec.read_index != codec.put_index)
		{
		for(i=0;i<8-txlevel;i++)
		{
		//	rt_kprintf("send data to i2s\n");
			LPC_I2S->TXFIFO = *(uint32_t *)(codec.data_list[codec.read_index].data_ptr + codec.data_list[codec.read_index].data_offset);
			codec.data_list[codec.read_index].data_offset +=2;
		
		}
	}
			  else//reach the end of buffer
			{
			//	rt_kprintf("stop i2s\n");
				NVIC_DisableIRQ(I2S_IRQn);
				I2S_Stop(LPC_I2S, I2S_TX_MODE);
			}
    }
    /* leave interrupt */
    rt_interrupt_leave();
}
void I2S_IRQHandler()
{
codec_i2s_isr();
}

