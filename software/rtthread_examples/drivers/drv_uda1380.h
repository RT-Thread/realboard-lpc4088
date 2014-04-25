#ifndef __UDA1380_H
#define __UDA1380_H
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>

#define UDA1380_SYSCLK_USED 0
#define UDA1380_SLAVE_ADDR  0x1A
#define UDA1380_CMD_BUFF_SIZE 3

/** UDA1380 Registers */
/* Register definition for UDA1380 */
typedef enum
{
	UDA1380_EVALMODE_CLK =		0x00,
	UDA1380_I2S_BUS_IO =		0x01,
	UDA1380_PW_CONTROL =		0x02,
	UDA1380_ANALOG_MIX =		0x03,
	UDA1380_HEADHONE_AMP =		0x04,
	UDA1380_MASTER_VOLUME =		0x10,
	UDA1380_MIXER_VOLUME =		0x11,
	UDA1380_MODE_SELECT =		0x12,
	UDA1380_MASTER_CHAN_MUTE =	0x13,
	UDA1380_MIXER_DETECT_OVER =	0x14,
	UDA1380_DECIM_VOLUM =		0x20,
	UDA1380_PGA =				0x21,
	UDA1380_ADC =				0x22,
	UDA1380_AGC =				0x23,
	UDA1380_SOFT_RESET =		0x7F,
	UDA1380_FILTER_STATUS =		0x18,
	UDA1380_DECIM_STATUS =		0x28,
	UDA1380_END =				0xFF
} UDA1380_Registers;

// UDA1380_REG_EVALCLK bit defines
#define EVALCLK_ADC_EN           0x0800  // Enable ADC clock
#define EVALCLK_DEC_EN           0x0400  // Enable decimator clock
#define EVALCLK_DAC_EN           0x0200  // Enable DAC clock
#define EVALCLK_INT_EN           0x0100  // Enable interpolator clock
#define EVALCLK_ADC_SEL_WSPLL   0x0020  // Select SYSCLK input for ADC clock
#define EVALCLK_ADC_SEL_SYSCLK    0x0000  // Select WSPLL clock for ADC clock
#define EVALCLK_DAC_SEL_WSPLL   0x0010  // Select SYSCLK input for DAC clock
#define EVALCLK_DAC_SEL_SYSCLK    0x0000  // Select WSPLL clock for DAC clock
#define EVALCLK_SYSDIV_SEL(n)    ((n) << 2) // System clock input divider select
#define EVALCLK_WSPLL_SEL6_12K   0x0000  // WSPLL input freq selection = 6.25 to 12.5K
#define EVALCLK_WSPLL_SEL12_25K  0x0001  // WSPLL input freq selection = 12.5K to 25K
#define EVALCLK_WSPLL_SEL25_50K  0x0002  // WSPLL input freq selection = 25K to 50K
#define EVALCLK_WSPLL_SEL50_100K 0x0003  // WSPLL input freq selection = 50K to 100K

// UDA1380_REG_I2S
#define I2S_SFORI_I2S 0x0000
#define I2S_SFORI_LSB16 0x0100
#define I2S_SFORI_LSB18 0x0200
#define I2S_SFORI_LSB20 0x0300
#define I2S_SFORI_MSB 0x0500
#define I2S_SFORI_MASK 0x0700
#define I2S_SFORO_I2S 0x0000
#define I2S_SFORO_LSB16 0x0001
#define I2S_SFORO_LSB18 0x0002
#define I2S_SFORO_LSB20 0x0003
#define I2S_SFORO_LSB24 0x0004
#define I2S_SFORO_MSB 0x0005
#define I2S_SFORO_MASK 0x0007
#define I2S_SEL_SOURCE 0x0040
#define I2S_SIM 0x0010

// UDA1380_REG_PWRCTRL bit defines 
#define PWR_PON_PLL_EN           0x8000  // WSPLL enable
#define PWR_PON_HP_EN            0x2000  // Headphone driver enable
#define PWR_PON_DAC_EN           0x0400  // DAC power enable
#define PWR_PON_BIAS_EN          0x0100  // Power on bias enable (for ADC, AVC, and FSDAC)
#define PWR_EN_AVC_EN            0x0080  // Analog mixer enable
#define PWR_PON_AVC_EN           0x0040  // Analog mixer power enable
#define PWR_EN_LNA_EN            0x0010  // LNA and SDC power enable
#define PWR_EN_PGAL_EN           0x0008  // PGA left power enable
#define PWR_EN_ADCL_EN           0x0004  // ADC left power enable
#define PWR_EN_PGAR_EN           0x0002  // PGA right power enable
#define PWR_EN_ADCR_EN           0x0001  // ADC right power enable

// UDA1380_REG_MSTRMUTE bit defines
#define MSTRMUTE_MTM_MUTE_EN     0x4000  // Master mute enable
#define MSRTMUTE_CHANNEL2_MUTE_EN 0x0800 
#define MSRTMUTE_CHANNEL1_MUTE_EN 0x0008


// UDA1380_REG_MODEBBT bit defines
#define MODEBBT_BOOST_FLAT       0x0000  // Bits for selecting flat boost
#define MODEBBT_BOOST_FULL       0xC000  // Bits for selecting maximum boost
#define MODEBBT_BOOST_MASK       0xC000  // Bits for selecting boost mask



/* Device Control Commands */
#define CODEC_CMD_RESET			0
#define CODEC_CMD_VOLUME		1
#define CODEC_CMD_SAMPLERATE	2
#define CODEC_CMD_EQ			3
#define CODEC_CMD_3D			4


#endif

