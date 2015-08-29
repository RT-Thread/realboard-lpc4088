/*
 * FT6206 touch driver
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>

#include <rtgui/event.h>
#include <rtgui/touch.h>
#include <rtgui/rtgui_server.h>

#include "board.h"
#include "drv_touch.h"

#define LCD_WIDTH     480
#define LCD_HEIGHT    272
#define BSP_TOUCH_SAMPLE_HZ  30

#define I2CBUS_NAME  "i2c1"

#if 0
#define FTDEBUG      rt_kprintf
#else
#define FTDEBUG(...)
#endif

#define TOUCH_SLP_TIME          (RT_TICK_PER_SECOND * 5)

#define FT5206_TS_ADDR          0x38
#define TP_MAX_TOUCH_POINT      2

enum ft5x0x_ts_regs {
    FT5X0X_REG_THGROUP                  = 0x80,
    FT5X0X_REG_THPEAK                   = 0x81,
    FT5X0X_REG_THCAL                    = 0x82,
    FT5X0X_REG_THWATER                  = 0x83,
    FT5X0X_REG_THTEMP                   = 0x84,
    FT5X0X_REG_THDIFF                   = 0x85,
    FT5X0X_REG_CTRL                     = 0x86,
    FT5X0X_REG_TIMEENTERMONITOR         = 0x87,
    FT5X0X_REG_PERIODACTIVE             = 0x88,
    FT5X0X_REG_PERIODMONITOR            = 0x89,
    FT5X0X_REG_HEIGHT_B                 = 0x8a,
    FT5X0X_REG_MAX_FRAME                = 0x8b,
    FT5X0X_REG_DIST_MOVE                = 0x8c,
    FT5X0X_REG_DIST_POINT               = 0x8d,
    FT5X0X_REG_FEG_FRAME                = 0x8e,
    FT5X0X_REG_SINGLE_CLICK_OFFSET      = 0x8f,
    FT5X0X_REG_DOUBLE_CLICK_TIME_MIN    = 0x90,
    FT5X0X_REG_SINGLE_CLICK_TIME        = 0x91,
    FT5X0X_REG_LEFT_RIGHT_OFFSET        = 0x92,
    FT5X0X_REG_UP_DOWN_OFFSET           = 0x93,
    FT5X0X_REG_DISTANCE_LEFT_RIGHT      = 0x94,
    FT5X0X_REG_DISTANCE_UP_DOWN         = 0x95,
    FT5X0X_REG_ZOOM_DIS_SQR             = 0x96,
    FT5X0X_REG_RADIAN_VALUE             = 0x97,
    FT5X0X_REG_MAX_X_HIGH               = 0x98,
    FT5X0X_REG_MAX_X_LOW                = 0x99,
    FT5X0X_REG_MAX_Y_HIGH               = 0x9a,
    FT5X0X_REG_MAX_Y_LOW                = 0x9b,
    FT5X0X_REG_K_X_HIGH                 = 0x9c,
    FT5X0X_REG_K_X_LOW                  = 0x9d,
    FT5X0X_REG_K_Y_HIGH                 = 0x9e,
    FT5X0X_REG_K_Y_LOW                  = 0x9f,
    FT5X0X_REG_AUTO_CLB_MODE            = 0xa0,
    FT5X0X_REG_LIB_VERSION_H            = 0xa1,
    FT5X0X_REG_LIB_VERSION_L            = 0xa2,
    FT5X0X_REG_CIPHER                   = 0xa3,
    FT5X0X_REG_G_MODE                   = 0xa4,
    FT5X0X_REG_PMODE                    = 0xa5, /* Power Consume Mode */
    FT5X0X_REG_FIRMID                   = 0xa6,
    FT5X0X_REG_STATE                    = 0xa7,
    FT5X0X_REG_VENDID                   = 0xa8,
    FT5X0X_REG_ERR                      = 0xa9,
    FT5X0X_REG_CLB                      = 0xaa,
};

#define CTRL_NOAUTO_MONITOR    0x00
#define CTRL_AUTO_MONITOR      0x01

#define PMODE_ACTIVE           0x00
#define PMODE_MONITOR          0x01
#define PMODE_STANDBY          0x02
#define PMODE_HIBERNATE        0x03

#define G_MODE_POLLING         0x00
#define G_MODE_TRIGGER         0x01

/*
 * This struct is a touchpoint as stored in hardware.  Note that the id,
 * as well as the event, are stored in the upper nybble of the hi byte.
 */
struct ft6x06_touchpoint {
    /* high 2bits: event, low 4bits: xhi */
    rt_uint8_t event_xhi;
    rt_uint8_t xlo;

    /* high 4bits: id, low 4bites: yhi */
    rt_uint8_t id_yhi;
    rt_uint8_t ylo;

    /* Touch weight. */
    rt_uint8_t weight;
    /* Touch area. */
    rt_uint8_t misc;
} __attribute__((__packed__));

/* This packet represents the register map as read from offset 0 */
struct ft6x06_packet {
    rt_uint8_t dev_mode;
    rt_uint8_t gest_id;
    /* Low 4 bits. */
    rt_uint8_t td_status;
    struct ft6x06_touchpoint points[TP_MAX_TOUCH_POINT];
} __attribute__((__packed__));

struct touch_data
{
    uint8_t tpnr;

    struct {
        uint16_t x;
        uint16_t y;
    } pts[TP_MAX_TOUCH_POINT];
};

static struct rt_i2c_bus_device *_i2c_bus;
static struct rt_semaphore _tp_sem;

static void _enable_exti(void);

static int _ft6206_read(unsigned char cmd,
                        void *buf,
                        size_t len)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr  = FT5206_TS_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = &cmd;
    msgs[0].len   = sizeof(cmd);

    msgs[1].addr  = FT5206_TS_ADDR;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf   = buf;
    msgs[1].len   = len;

    if (rt_i2c_transfer(_i2c_bus, msgs, 2) == 2)
        return len;
    else
        return -1;
}

static int _ft6206_write(void *buf,
                         size_t len)
{
    struct rt_i2c_msg msgs[1];

    msgs[0].addr  = FT5206_TS_ADDR;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf   = buf;
    msgs[0].len   = len;

    if (rt_i2c_transfer(_i2c_bus, msgs, 1) == 2)
        return len;
    else
        return -1;
}

int ft6206_read_id(void)
{
	int id;
    unsigned char value;

    if (_ft6206_read(FT5X0X_REG_FIRMID, &value, 1) > 0)
    {
        rt_kprintf("get fw ver: %02x\n", value);
    }

	value = 0;
    if (_ft6206_read(FT5X0X_REG_VENDID, &value, 1) > 0)
    {
        rt_kprintf("vender id: %02x\n", value);
    }
    id = value << 8;

	value = 0;
	if (_ft6206_read(FT5X0X_REG_CIPHER, &value, 1) > 0)
    {
        rt_kprintf("chip id: %02x\n", value);
    }
	id = id | value;

	return id;
}
FINSH_FUNCTION_EXPORT_ALIAS(ft6206_read_id, ft6_dump, dump ft6206 fw version);

rt_inline unsigned short _ft6206_get_pointx(struct ft6x06_packet *pkt,
                                  unsigned char idx)
{
    RT_ASSERT(idx < TP_MAX_TOUCH_POINT);

    return ((unsigned short)pkt->points[idx].event_xhi & 0x0F) << 8 |
           (pkt->points[idx].xlo);
}

rt_inline unsigned short _ft6206_get_pointy(struct ft6x06_packet *pkt,
                                  unsigned char idx)
{
    RT_ASSERT(idx < TP_MAX_TOUCH_POINT);

    return ((unsigned short)pkt->points[idx].id_yhi & 0x0F) << 8 |
           (pkt->points[idx].ylo);
}

static void ft6206_read_touch(struct touch_data *dp)
{
    unsigned char tpnr, i;
    /* We have to read all the data. Otherwise the second coordinate will
     * wrong. */
    struct ft6x06_packet pkt;

    dp->tpnr = 0;

    if (_ft6206_read(0, &pkt, sizeof(pkt)) < 0)
        return;

    tpnr = pkt.td_status & 0x0F;
    if (tpnr > TP_MAX_TOUCH_POINT || tpnr == 0)
        return;

    RT_ASSERT(dp);

    FTDEBUG("%d: ", pkt.gest_id);
    for (i = 0; i < tpnr; i++)
    {
        FTDEBUG("index[%d] (%d, %d, %s, %01x)",
				i, 
                _ft6206_get_pointx(&pkt, i),
                _ft6206_get_pointy(&pkt, i),
                pkt.points[i].event_xhi & 0xC0 ? "up" : "dn",
                pkt.points[i].id_yhi);
        if (_ft6206_get_pointx(&pkt, i) > LCD_WIDTH ||
            _ft6206_get_pointy(&pkt, i) > LCD_HEIGHT)
            continue;
        dp->tpnr++;
        dp->pts[i].x = _ft6206_get_pointx(&pkt, i);
        dp->pts[i].y = _ft6206_get_pointy(&pkt, i);
    }
    FTDEBUG("\n");
}

void ft6206_isr(void)
{
    if ((LPC_GPIOINT->IO0IntStatF >> 13) & 0x01)
    {
        rt_sem_release(&_tp_sem);

        LPC_GPIOINT->IO0IntClr |= (1 << 13);
    }
}

static void _ft6206_cfg(unsigned char cmd,
                        unsigned char data)
{
    unsigned char _cmd[2];

    _cmd[0] = cmd;
    _cmd[1] = data;
    _ft6206_write(_cmd, sizeof(_cmd));
}

static int _ft6206_touched(void)
{
    return !(LPC_GPIO0->PIN & (0x01<<13));
}

static void _touch_session()
{
    struct touch_data tpd;
    struct rtgui_event_mouse emouse;

    do {
        if (!_ft6206_touched())
            continue;

        ft6206_read_touch(&tpd);
        if (tpd.tpnr == 0)
            continue;

        emouse.parent.sender = RT_NULL;
        emouse.wid = RT_NULL;

        emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
        emouse.button = RTGUI_MOUSE_BUTTON_LEFT | RTGUI_MOUSE_BUTTON_DOWN;
        emouse.x = tpd.pts[0].x;
        emouse.y = tpd.pts[0].y;
        emouse.ts = rt_tick_get();
        emouse.id = emouse.ts;
        if (emouse.id == 0)
            emouse.id = 1;
        rtgui_server_post_event(&emouse.parent, sizeof(emouse));

        do {
            rt_thread_delay(RT_TICK_PER_SECOND / BSP_TOUCH_SAMPLE_HZ);
            ft6206_read_touch(&tpd);
            if (tpd.tpnr == 0)
                break;

            emouse.parent.type = RTGUI_EVENT_MOUSE_MOTION;
            emouse.x = tpd.pts[0].x;
            emouse.y = tpd.pts[0].y;
            emouse.ts = rt_tick_get();
            rtgui_server_post_event(&emouse.parent, sizeof(emouse));

        } while (_ft6206_touched());

        ft6206_read_touch(&tpd);
        /* Always send touch up event. */
        emouse.parent.type = RTGUI_EVENT_MOUSE_BUTTON;
        emouse.button = RTGUI_MOUSE_BUTTON_LEFT | RTGUI_MOUSE_BUTTON_UP;
        emouse.x = tpd.pts[0].x;
        emouse.y = tpd.pts[0].y;
        emouse.ts = rt_tick_get();
        rtgui_server_post_event(&emouse.parent, sizeof(emouse));
    } while (rt_sem_take(&_tp_sem, TOUCH_SLP_TIME) == RT_EOK);
}

static void _touch(void *p)
{
    _ft6206_cfg(FT5X0X_REG_G_MODE, G_MODE_POLLING);
    /* 40Hz in active mode. */
    _ft6206_cfg(FT5X0X_REG_PERIODACTIVE, 2);
    _ft6206_cfg(FT5X0X_REG_PERIODMONITOR, 1);
    _ft6206_cfg(FT5X0X_REG_CTRL, CTRL_AUTO_MONITOR);

	/* set touch isr */
	touch_install_isr(ft6206_isr);

    do {
        _touch_session();
    } while (rt_sem_take(&_tp_sem, RT_WAITING_FOREVER)
             == RT_EOK);
}

static void _enable_exti(void)
{
    /* enable P0.13 failling edge interrupt */
    LPC_GPIOINT->IO0IntEnF |= (0x01 << 13);
}

static void _setup_int(void)
{
    /* P0.13 touch INT */
    {
        LPC_IOCON->P0_13 &= ~0x07;
        LPC_GPIO0->DIR &= ~(0x01 << 13);
    }
    /* Configure  EXTI  */
    LPC_GPIOINT->IO0IntEnF |= (0x01 << 13);
    //touch_int_enable(RT_TRUE);
    NVIC_SetPriority(GPIO_IRQn, ((0x01 << 3) | 0x01));
    NVIC_EnableIRQ(GPIO_IRQn);

    _enable_exti();
}

int ft6206_hw_init(void)
{
	int id;
    rt_thread_t tid;
    rt_device_t dev;

    dev = rt_device_find(I2CBUS_NAME);
    if (!dev) return -1;

    if (rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) != RT_EOK)
        return -1;

    FTDEBUG("ft6202 set i2c bus to %s\n", I2CBUS_NAME);
    _i2c_bus = (struct rt_i2c_bus_device *)dev;

    /* read ft6206 id */
	id = ft6206_read_id();
	if (id != 0x7955)
	{
		rt_device_close(dev);
		return -1;
	}

    rt_sem_init(&_tp_sem, "touch", 0, RT_IPC_FLAG_FIFO);
    tid = rt_thread_create("touch", _touch, RT_NULL,
                           2048, 10, 20);
    if (!tid)
    {
        rt_device_close(dev);
        return -1;
    }

    _setup_int();
    rt_thread_startup(tid);

    return 0;
}
