#include <rtthread.h>
#include <rtdevice.h>

#ifdef RT_USING_RTGUI

#include <board.h>
#include <rtgui/rtgui.h>
#include <rtgui/event.h>
#include <rtgui/touch.h>
#include <rtgui/calibration.h>
#include "drv_touch.h"
#include "lpc_eeprom.h"

static touch_isr_t touch_isr = RT_NULL;
void touch_install_isr(touch_isr_t isr)
{
	touch_isr = isr;
}

void GPIO_IRQHandler(void)
{
    if ((LPC_GPIOINT->IO0IntStatF >> 13) & 0x01)
    {
		if (touch_isr != RT_NULL)
		{
			touch_isr();
		}

        /* disable interrupt */
        LPC_GPIOINT->IO0IntClr |= (1 << 13);
    }
}

#define CALIBRATION_DATA_MAGIC  0x55
#define CALIBRATION_DATA_PAGE   0x00
#define CALIBRATION_DATA_OFFSET 0x01

static rt_bool_t calibration_data_save(calculate_data_t *data)
{
    rt_uint8_t count;
    rt_uint8_t *buf;
    count = sizeof(calculate_data_t) + 1;
    buf = (rt_uint8_t *)rt_malloc(count);
    buf[0] = CALIBRATION_DATA_MAGIC;
    rt_memcpy(&buf[1], data, count - 1);
    /* power up the eeprom */
    EEPROM_PowerDown(DISABLE);
    /* erase the page for touch data */
    EEPROM_Erase(CALIBRATION_DATA_PAGE);
    EEPROM_Write(0, CALIBRATION_DATA_PAGE, buf, MODE_8_BIT, count);
    rt_free(buf);
    return RT_TRUE;
}

/* initialize for touch & calibration */
int touch_calibration_init(void)
{
    rt_uint8_t magic = 0;
    calculate_data_t data;
    struct rtgui_calibration_ops *ops;

    ops = calibration_get_ops();

    /* initialization the eeprom  on chip  */
    EEPROM_Init();
    /* initialization the touch driver */
    tsc2046_hw_init("spi10");
    /* set callback to save calibration data */
    calibration_set_after(calibration_data_save);
    /* initialization rtgui tonch server */
    rtgui_touch_init(ops);
    /* restore calibration data */
    EEPROM_Read(0, CALIBRATION_DATA_PAGE, &magic, MODE_8_BIT, 1);
    if (CALIBRATION_DATA_MAGIC != magic)
    {
        rt_kprintf("touch is not calibration,now calibration it please.\n");
        calibration_init(RT_NULL);
    }
    else
    {
        EEPROM_Read(CALIBRATION_DATA_OFFSET, CALIBRATION_DATA_PAGE, &data, MODE_8_BIT, sizeof(calculate_data_t));
        calibration_set_data(&data);
    }
    /* power down the EEPROM */
    EEPROM_PowerDown(ENABLE);
    return 0;
}

int touch_hw_init(void)
{
	if (ft6206_hw_init() != 0)
	{
		/* ft6206 failed, use tsc2046 touch driver */
		touch_calibration_init();
	}
    return 0;
}
INIT_APP_EXPORT(touch_hw_init);

#endif
