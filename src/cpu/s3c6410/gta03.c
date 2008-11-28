#include <qi.h>
#include <neo_gta03.h>
#include <s3c6410.h>
#include <serial-s3c64xx.h>
#include <i2c-bitbang-s3c6410.h>
#include <pcf50633.h>

#define PCF50633_I2C_ADS 0x73

const struct pcf50633_init gta03_pcf50633_init[] = {

	{ PCF50633_REG_OOCWAKE,		0xd3 }, /* wake from ONKEY,EXTON!,RTC,USB,ADP */
	{ PCF50633_REG_OOCTIM1,		0xaa },	/* debounce 14ms everything */
	{ PCF50633_REG_OOCTIM2,		0x4a },
	{ PCF50633_REG_OOCMODE,		0x55 },
	{ PCF50633_REG_OOCCTL,		0x47 },

	{ PCF50633_REG_SVMCTL,		0x08 },	/* 3.10V SYS voltage thresh. */
	{ PCF50633_REG_BVMCTL,		0x02 },	/* 2.80V BAT voltage thresh. */

	{ PCF50633_REG_AUTOENA,		0x01 },	/* always on */

	{ PCF50633_REG_DOWN1OUT,	0x17 }, /* 1.2V (0x17 * .025V + 0.625V) */
	{ PCF50633_REG_DOWN1ENA,	0x02 }, /* enabled if GPIO1 = HIGH */
	{ PCF50633_REG_LDO6ENA,		0x01 }, /* LCM power off */

	{ PCF50633_REG_INT1M,		0x00 },
	{ PCF50633_REG_INT2M,		0x00 },
	{ PCF50633_REG_INT3M,		0x00 },
	{ PCF50633_REG_INT4M,		0x00 },
	{ PCF50633_REG_INT5M,		0x00 },

	{ PCF50633_REG_MBCC2,		0x28 },	/* Vbatconid=2.7V, Vmax=4.20V */
	{ PCF50633_REG_MBCC3,		0x19 },	/* 25/255 == 98mA pre-charge */
	{ PCF50633_REG_MBCC4,		0xff }, /* 255/255 == 1A adapter fast */
	{ PCF50633_REG_MBCC5,		0x19 },	/* 25/255 == 98mA soft-start usb fast */
	{ PCF50633_REG_MBCC6,		0x00 }, /* cutoff current 1/32 * Ichg */
	{ PCF50633_REG_MBCC7,		0x00 },	/* 1.6A max bat curr, USB 100mA */
	{ PCF50633_REG_MBCC8,		0x00 },
	{ PCF50633_REG_MBCC1,		0xff }, /* chgena */

	{ PCF50633_REG_BBCCTL,		0x19 },	/* 3V, 200uA, on */
	{ PCF50633_REG_OOCSHDWN,	0x04 },  /* defeat 8s death from lowsys on A5 */

};

static const struct board_variant board_variants[] = {
	[0] = {
		.name = "GTA03 unknown",
		.machine_revision = 0
	},
	[1] = {
		.name = "GTA03 A1",
		.machine_revision = 1
	},
	[2] = {
		.name = "GTA03 A2",
		.machine_revision = 2
	},
	[3] = {
		.name = "GTA03 A3",
		.machine_revision = 3
	},
	[4] = {
		.name = "GTA03 A4",
		.machine_revision = 4
	},
	[5] = {
		.name = "GTA03 A5",
		.machine_revision = 5
	},
	[6] = {
		.name = "GTA03 A6",
		.machine_revision = 6
	},
	[7] = {
		.name = "GTA03 A7",
		.machine_revision = 7
	}
};

void port_init_gta03(void)
{
	int n;

	/* ---------------------------- Port A ---------------------------- */

	__REG(GPACON) =
		(2 << 0)  | /* GPA0  - UART_RXD0 */
		(2 << 4)  | /* GPA1  - UART_TXD0 */
		(2 << 8)  | /* GPA2  - UART_CTS0 */
		(2 << 12) | /* GPA3  - UART_RTS0 */
		(2 << 16) | /* GPA4  - UART_RXD1 */
		(2 << 20) | /* GPA5  - UART_TXD1 */
		(2 << 24) | /* GPA6  - UART_CTS1 */
		(2 << 28)   /* GPA7  - UART_RTS1 */
	;

	__REG(GPAPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPADAT) = 0; /* just for determinism */

	__REG(GPACONSLP) =
		(3 << 0)  | /* GPA0  - keep */
		(3 << 2)  | /* GPA1  - keep */
		(3 << 4)  | /* GPA2  - keep */
		(3 << 6)  | /* GPA3  - keep */
		(3 << 8)  | /* GPA4  - keep */
		(3 << 10) | /* GPA5  - keep */
		(3 << 12) | /* GPA6  - keep */
		(3 << 14)   /* GPA7  - keep */
	;

	/* ---------------------------- Port B ---------------------------- */

	__REG(GPBCON) =
		(2 << 0)  | /* GPB0  - UART_RXD2 */
		(2 << 4)  | /* GPB1  - UART_TXD2 */
		(2 << 8)  | /* GPB2  - UART_RXD3 */
		(2 << 12) | /* GPB3  - UART_TXD3 */
		(1 << 16) | /* GPB4  - (NC) output low */
		(1 << 20) | /* GPB5  - (I2C BB SCL) OUTPUT */
		(1 << 24)   /* GPB6  - (I2C BB SDA) OUTPUT */
	;

	__REG(GPBPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPBDAT) = 0; /* just for determinism */

	__REG(GPBCONSLP) =
		(3 << 0)  | /* GPB0  - keep */
		(3 << 2)  | /* GPB1  - keep */
		(3 << 4)  | /* GPB2  - keep */
		(3 << 6)  | /* GPB3  - keep */
		(3 << 8)  | /* GPB4  - keep */
		(3 << 10) | /* GPB5  - keep */
		(3 << 12) | /* GPB6  - keep */
		(3 << 14)   /* GPB7  - keep */
	;

	__REG(GPBPUDSLP) =
		(0 << 0)  | /* GPB0  - no pull up or down */
		(0 << 2)  | /* GPB1  - no pull up or down */
		(0 << 4)  | /* GPB2  - no pull up or down */
		(0 << 6)  | /* GPB3  - no pull up or down */
		(0 << 8)  | /* GPB4  - no pull up or down */
		(0 << 10) | /* GPB5  - no pull up or down */
		(0 << 12) | /* GPB6  - no pull up or down */
		(0 << 14)   /* GPB7  - no pull up or down */
	;

	/* ---------------------------- Port C ---------------------------- */

	__REG(GPCCON) =
		(0 << 0)  | /* GPC0  - SPI_MISO0 INPUT  */
		(1 << 4)  | /* GPC1  - SPI_CLK0  OUTPUT */
		(1 << 8)  | /* GPC2  - SPI_MOSI0 OUTPUT */
		(1 << 12) | /* GPC3  - SPI_CS0   OUTPUT */
		(1 << 16) | /* GPC4  - (NC)      OUTPUT */
		(1 << 20) | /* GPC5  - SPI_CLK1  OUTPUT */
		(1 << 24) | /* GPC6  - SPI_MOSI1 OUTPUT */
		(1 << 28)   /* GPC7  - SPI_CS1   OUTPUT */
	;

	__REG(GPCPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPCDAT) = 0; /* just for determinism */

	__REG(GPCCONSLP) =
		(3 << 0)  | /* GPC0  - keep */
		(3 << 2)  | /* GPC1  - keep */
		(3 << 4)  | /* GPC2  - keep */
		(3 << 6)  | /* GPC3  - keep */
		(3 << 8)  | /* GPC4  - keep */
		(3 << 10) | /* GPC5  - keep */
		(3 << 12) | /* GPC6  - keep */
		(3 << 14)   /* GPC7  - keep */
	;

	__REG(GPCPUDSLP) =
		(0 << 0)  | /* GPC0  - no pull up or down */
		(0 << 2)  | /* GPC1  - no pull up or down */
		(0 << 4)  | /* GPC2  - no pull up or down */
		(0 << 6)  | /* GPC3  - no pull up or down */
		(0 << 8)  | /* GPC4  - no pull up or down */
		(0 << 10) | /* GPC5  - no pull up or down */
		(0 << 12) | /* GPC6  - no pull up or down */
		(0 << 14)   /* GPC7  - no pull up or down */
	;

	/* ---------------------------- Port D ---------------------------- */

	__REG(GPDCON) =
		(3 << 0)  | /* GPD0  - I2S_CLK0  */
		(3 << 4)  | /* GPD1  - I2S_CDCLK0 */
		(3 << 8)  | /* GPD2  - I2S_LRCLK0 */
		(3 << 12) | /* GPD3  - I2S_DI */
		(3 << 16)   /* GPD4  - I2S_DO */
	;

	__REG(GPDPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPDDAT) = 0; /* just for determinism */

	__REG(GPDCONSLP) =
		(3 << 0)  | /* GPD0  - keep */
		(3 << 2)  | /* GPD1  - keep */
		(3 << 4)  | /* GPD2  - keep */
		(3 << 6)  | /* GPD3  - keep */
		(3 << 8)    /* GPD4  - keep */
	;

	__REG(GPDPUDSLP) =
		(0 << 0)  | /* GPD0  - no pull up or down */
		(0 << 2)  | /* GPD1  - no pull up or down */
		(0 << 4)  | /* GPD2  - no pull up or down */
		(0 << 6)  | /* GPD3  - no pull up or down */
		(0 << 8)    /* GPD4  - no pull up or down */
	;

	/* ---------------------------- Port E ---------------------------- */

	__REG(GPECON) =
		(3 << 0)  | /* GPE0  - PCM_SCLK1  */
		(3 << 4)  | /* GPE1  - PCM_EXTCLK1 */
		(3 << 8)  | /* GPE2  - PCM_FSYNC1 */
		(3 << 12) | /* GPE3  - PCM_SIN */
		(3 << 16)   /* GPE4  - PCM_SOUT */
	;

	__REG(GPEPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPEDAT) = 0; /* just for determinism */

	__REG(GPECONSLP) =
		(3 << 0)  | /* GPE0  - keep */
		(3 << 2)  | /* GPE1  - keep */
		(3 << 4)  | /* GPE2  - keep */
		(3 << 6)  | /* GPE3  - keep */
		(3 << 8)    /* GPE4  - keep */
	;

	__REG(GPEPUDSLP) =
		(1 << 0)  | /* GPE0  - pull down */
		(1 << 2)  | /* GPE1  - pull down */
		(1 << 4)  | /* GPE2  - pull down */
		(1 << 6)  | /* GPE3  - pull down */
		(1 << 8)    /* GPE4  - pull down */
	;

	/* ---------------------------- Port F ---------------------------- */

	__REG(GPFCON) =
		(2 << 0)  | /* GPF0  - CAMIF_CLK */
		(2 << 2)  | /* GPF1  - CAMIF_HREF */
		(2 << 4)  | /* GPF2  - CAMIF_PCLK */
		(2 << 6)  | /* GPF3  - CAMIF_RSTn */
		(2 << 8)  | /* GPF4  - CAMIF_VSYNC */
		(2 << 10) | /* GPF5  - CAMIF_YDATA0 */
		(2 << 12) | /* GPF6  - CAMIF_YDATA1 */
		(2 << 14) | /* GPF7  - CAMIF_YDATA2 */
		(2 << 16) | /* GPF8  - CAMIF_YDATA3 */
		(2 << 18) | /* GPF9  - CAMIF_YDATA4 */
		(2 << 20) | /* GPF10 - CAMIF_YDATA5 */
		(2 << 22) | /* GPF11 - CAMIF_YDATA6 */
		(2 << 24) | /* GPF12 - CAMIF_YDATA7 */
		(1 << 26) | /* GPF13 - OUTPUT Vibrator */
		(3 << 28) | /* GPF14 - CLKOUT0 */
		(1 << 30)   /* GPF15 - OUTPUT CAM_PWRDN */
	;

	__REG(GPFPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPFDAT) = (1 << 15); /* assert CAM_PWRDN */

	__REG(GPFCONSLP) =
		(0 << 0)  | /* GPF0  - OUTPUT 0 */
		(0 << 2)  | /* GPF1  - OUTPUT 0 */
		(0 << 4)  | /* GPF2  - OUTPUT 0 */
		(0 << 6)  | /* GPF3  - OUTPUT 0 */
		(0 << 8)  | /* GPF4  - OUTPUT 0 */
		(0 << 10) | /* GPF5  - OUTPUT 0 */
		(0 << 12) | /* GPF6  - OUTPUT 0 */
		(0 << 14) | /* GPF7  - OUTPUT 0 */
		(0 << 16) | /* GPF8  - OUTPUT 0 */
		(0 << 18) | /* GPF9  - OUTPUT 0 */
		(0 << 20) | /* GPF10 - OUTPUT 0 */
		(0 << 22) | /* GPF11 - OUTPUT 0 */
		(0 << 24) | /* GPF12 - OUTPUT 0 */
		(0 << 26) | /* GPF13 - OUTPUT 0 */
		(0 << 28) | /* GPF14 - OUTPUT 0 */
		(0 << 30)   /* GPF15 - OUTPUT 0 */
	;

	__REG(GPFPUDSLP) =
		(0 << 0)  | /* GPF0  - no pull up or down */
		(0 << 2)  | /* GPF1  - no pull up or down */
		(0 << 4)  | /* GPF2  - no pull up or down */
		(0 << 6)  | /* GPF3  - no pull up or down */
		(0 << 8)  | /* GPF4  - no pull up or down */
		(0 << 10) | /* GPF5  - no pull up or down */
		(0 << 12) | /* GPF6  - no pull up or down */
		(0 << 14) | /* GPF7  - no pull up or down */
		(0 << 16) | /* GPF8  - no pull up or down */
		(0 << 18) | /* GPF9  - no pull up or down */
		(0 << 20) | /* GPF10 - no pull up or down */
		(0 << 22) | /* GPF11 - no pull up or down */
		(0 << 24) | /* GPF12 - no pull up or down */
		(0 << 26) | /* GPF13 - no pull up or down */
		(0 << 28) | /* GPF14 - no pull up or down */
		(0 << 30)   /* GPF15 - no pull up or down */
	;

	/* ---------------------------- Port G ---------------------------- */

	__REG(GPGCON) =
		(2 << 0)  | /* GPG0  - MMC_CLK0 */
		(2 << 4)  | /* GPG1  - MMC_CMD0 */
		(2 << 8)  | /* GPG2  - MMC_DATA00 */
		(2 << 12) | /* GPG3  - MMC_DATA10 */
		(2 << 16) | /* GPG4  - MMC_DATA20 */
		(2 << 20) | /* GPG5  - MMC_DATA30 */
		(2 << 24)   /* GPG6  - (NC) MMC CARD DETECT */
	;

	__REG(GPGPUD) = (1 << (6 * 2)); /* pull down card detect */

	__REG(GPGDAT) = 0; /* just for determinism */

	__REG(GPGCONSLP) =
		(0 << 0)  | /* GPG0  - OUTPUT 0 */
		(0 << 2)  | /* GPG1  - OUTPUT 0 */
		(0 << 4)  | /* GPG2  - OUTPUT 0 */
		(0 << 6)  | /* GPG3  - OUTPUT 0 */
		(0 << 8)  | /* GPG4  - OUTPUT 0 */
		(0 << 10) | /* GPG5  - OUTPUT 0 */
		(0 << 12)   /* GPG6  - OUTPUT 0 */
	;

	__REG(GPGPUDSLP) =
		(0 << 0)  | /* GPG0  - no pull up or down */
		(0 << 2)  | /* GPG1  - no pull up or down */
		(0 << 4)  | /* GPG2  - no pull up or down */
		(0 << 6)  | /* GPG3  - no pull up or down */
		(0 << 8)  | /* GPG4  - no pull up or down */
		(0 << 10) | /* GPG5  - no pull up or down */
		(0 << 12)   /* GPG6  - no pull up or down */
	;

	/* ---------------------------- Port H ---------------------------- */

	__REG(GPHCON0) =
		(2 << 0)  | /* GPH0  - MMC_CLK1 */
		(2 << 4)  | /* GPH1  - MMC_CMD1 */
		(2 << 8)  | /* GPH2  - MMC_DATA01 */
		(2 << 12) | /* GPH3  - MMC_DATA11 */
		(2 << 16) | /* GPH4  - MMC_DATA21 */
		(2 << 20) | /* GPH5  - MMC_DATA31 */
		(1 << 24) | /* GPH6  - OUTPUT nWLAN_RESET */
		(1 << 28)   /* GPH7  - OUTPUT HDQ */
	;
	__REG(GPHCON1) =
		(1 << 0) | /* GPH8  - OUTPUT nWLAN_PD */
		(1 << 4)   /* GPH9  - OUTPUT (NC) */
	;

	__REG(GPHPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPHDAT) = 0;

	__REG(GPHCONSLP) =
		(0 << 0)  | /* GPH0  - OUTPUT 0 */
		(0 << 2)  | /* GPH1  - OUTPUT 0 */
		(0 << 4)  | /* GPH2  - OUTPUT 0 */
		(0 << 6)  | /* GPH3  - OUTPUT 0 */
		(0 << 8)  | /* GPH4  - OUTPUT 0 */
		(0 << 10) | /* GPH5  - OUTPUT 0 */
		(0 << 12) | /* GPH6  - OUTPUT 0 */
		(2 << 14) | /* GPH7  - INPUT (HDQ) */
		(0 << 16) | /* GPH8  - OUTPUT 0 */
		(0 << 18)   /* GPH9  - OUTPUT 0 */
	;

	__REG(GPHPUDSLP) =
		(0 << 0)  | /* GPH0  - no pull up or down */
		(0 << 2)  | /* GPH1  - no pull up or down */
		(0 << 4)  | /* GPH2  - no pull up or down */
		(0 << 6)  | /* GPH3  - no pull up or down */
		(0 << 8)  | /* GPH4  - no pull up or down */
		(0 << 10) | /* GPH5  - no pull up or down */
		(2 << 12) | /* GPH6  - PULLUP (HDQ) */
		(0 << 14) | /* GPH7  - no pull up or down */
		(0 << 16) | /* GPH8  - no pull up or down */
		(0 << 18)   /* GPH9  - no pull up or down */
	;

	/* ---------------------------- Port I ---------------------------- */

	__REG(GPICON) =
		(0 << 0)  | /* GPI0  - INPUT   version b0 */
		(0 << 2)  | /* GPI1  - INPUT   version b1 */
		(2 << 4)  | /* GPI2  - LCD_VD2 */
		(2 << 6)  | /* GPI3  - LCD_VD3 */
		(2 << 8)  | /* GPI4  - LCD_VD4 */
		(2 << 10) | /* GPI5  - LCD_VD5 */
		(2 << 12) | /* GPI6  - LCD_VD6 */
		(2 << 14) | /* GPI7  - LCD_VD7 */
		(0 << 16) | /* GPI8  - INPUT   version b2 */
		(2 << 18) | /* GPI9  - LCD_VD9 */
		(2 << 20) | /* GPI10 - LCD_VD10 */
		(2 << 22) | /* GPI11 - LCD_VD11 */
		(2 << 24) | /* GPI12 - LCD_VD12 */
		(2 << 26) | /* GPI13 - LCD_VD13 */
		(2 << 28) | /* GPI14 - LCD_VD14 */
		(2 << 30)   /* GPI15 - LCD_VD15 */
	;

	__REG(GPIPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPIDAT) = 0; /* just for determinism */

	__REG(GPICONSLP) =
		(0 << 0)  | /* GPI0  - OUTPUT 0 */
		(0 << 2)  | /* GPI1  - OUTPUT 0 */
		(0 << 4)  | /* GPI2  - OUTPUT 0 */
		(0 << 6)  | /* GPI3  - OUTPUT 0 */
		(0 << 8)  | /* GPI4  - OUTPUT 0 */
		(0 << 10) | /* GPI5  - OUTPUT 0 */
		(0 << 12) | /* GPI6  - OUTPUT 0 */
		(0 << 14) | /* GPI7  - OUTPUT 0 */
		(0 << 16) | /* GPI8  - OUTPUT 0 */
		(0 << 18) | /* GPI9  - OUTPUT 0 */
		(0 << 20) | /* GPI10 - OUTPUT 0 */
		(0 << 22) | /* GPI11 - OUTPUT 0 */
		(0 << 24) | /* GPI12 - OUTPUT 0 */
		(0 << 26) | /* GPI13 - OUTPUT 0 */
		(0 << 28) | /* GPI14 - OUTPUT 0 */
		(0 << 30)   /* GPI15 - OUTPUT 0 */
	;

	__REG(GPIPUDSLP) =
		(0 << 0)  | /* GPI0  - no pull up or down */
		(0 << 2)  | /* GPI1  - no pull up or down */
		(0 << 4)  | /* GPI2  - no pull up or down */
		(0 << 6)  | /* GPI3  - no pull up or down */
		(0 << 8)  | /* GPI4  - no pull up or down */
		(0 << 10) | /* GPI5  - no pull up or down */
		(0 << 12) | /* GPI6  - no pull up or down */
		(0 << 14) | /* GPI7  - no pull up or down */
		(0 << 16) | /* GPI8  - no pull up or down */
		(0 << 18) | /* GPI9  - no pull up or down */
		(0 << 20) | /* GPI10 - no pull up or down */
		(0 << 22) | /* GPI11 - no pull up or down */
		(0 << 24) | /* GPI12 - no pull up or down */
		(0 << 26) | /* GPI13 - no pull up or down */
		(0 << 28) | /* GPI14 - no pull up or down */
		(0 << 30)   /* GPI15 - no pull up or down */
	;

	/* ---------------------------- Port J ---------------------------- */

	__REG(GPJCON) =
		(2 << 0)  | /* GPJ0  - LCD_VD16 */
		(2 << 2)  | /* GPJ1  - LCD_VD17 */
		(2 << 4)  | /* GPJ2  - LCD_VD18 */
		(2 << 6)  | /* GPJ3  - LCD_VD19 */
		(2 << 8)  | /* GPJ4  - LCD_VD20 */
		(2 << 10) | /* GPJ5  - LCD_VD21 */
		(2 << 12) | /* GPJ6  - LCD_VD22 */
		(2 << 14) | /* GPJ7  - LCD_VD23 */
		(2 << 16) | /* GPJ8  - LCD_HSYNC */
		(2 << 18) | /* GPJ9  - LCD_VSYNC */
		(2 << 20) | /* GPJ10 - LCD_VDEN */
		(2 << 22)   /* GPJ11 - LCD_VCLK */
	;

	__REG(GPJPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPJDAT) = 0; /* just for determinism */

	__REG(GPJCONSLP) =
		(0 << 0)  | /* GPJ0  - OUTPUT 0 */
		(0 << 2)  | /* GPJ1  - OUTPUT 0 */
		(0 << 4)  | /* GPJ2  - OUTPUT 0 */
		(0 << 6)  | /* GPJ3  - OUTPUT 0 */
		(0 << 8)  | /* GPJ4  - OUTPUT 0 */
		(0 << 10) | /* GPJ5  - OUTPUT 0 */
		(0 << 12) | /* GPJ6  - OUTPUT 0 */
		(0 << 14) | /* GPJ7  - OUTPUT 0 */
		(0 << 16) | /* GPJ8  - OUTPUT 0 */
		(0 << 18) | /* GPJ9  - OUTPUT 0 */
		(0 << 20) | /* GPJ10 - OUTPUT 0 */
		(0 << 22)   /* GPJ11 - OUTPUT 0 */
	;

	__REG(GPJPUDSLP) =
		(0 << 0)  | /* GPJ0  - no pull up or down */
		(0 << 2)  | /* GPJ1  - no pull up or down */
		(0 << 4)  | /* GPJ2  - no pull up or down */
		(0 << 6)  | /* GPJ3  - no pull up or down */
		(0 << 8)  | /* GPJ4  - no pull up or down */
		(0 << 10) | /* GPJ5  - no pull up or down */
		(0 << 12) | /* GPJ6  - no pull up or down */
		(0 << 14) | /* GPJ7  - no pull up or down */
		(0 << 16) | /* GPJ8  - no pull up or down */
		(0 << 18) | /* GPJ9  - no pull up or down */
		(0 << 20) | /* GPJ10 - no pull up or down */
		(0 << 22)   /* GPJ11 - no pull up or down */
	;

	/* ---------------------------- Port K ---------------------------- */

	__REG(GPKCON0) =
		(1 << 0)  | /* GPK0  - OUTPUT  nWLAN_POWERON */
		(1 << 4)  | /* GPK1  - OUTPUT  (NC) */
		(1 << 8)  | /* GPK2  - OUTPUT  (nMODEM_ON) */
		(1 << 12) | /* GPK3  - OUTPUT  (NC) */
		(1 << 16) | /* GPK4  - OUTPUT  (NC) */
		(1 << 20) | /* GPK5  - OUTPUT  (NC) */
		(1 << 24) | /* GPK6  - OUTPUT  (NC) */
		(1 << 28)   /* GPK7  - OUTPUT  (NC) */
	;
	__REG(GPKCON1) =
		(1 << 0)  | /* GPK8  - OUTPUT  (NC) */
		(1 << 4)  | /* GPK9  - OUTPUT  (NC) */
		(1 << 8)  | /* GPK10 - OUTPUT  (NC) */
		(1 << 12) | /* GPK11 - OUTPUT  (NC) */
		(1 << 16) | /* GPK12 - OUTPUT  (NC) */
		(1 << 20) | /* GPK13 - OUTPUT  (NC) */
		(1 << 24) | /* GPK14 - OUTPUT  (NC) */
		(1 << 28)   /* GPK15 - OUTPUT  (NC) */
	;

	__REG(GPKPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPKDAT) =
		(1 << 2)  /* deassert nMODEM_ON */
	;

	/* ---------------------------- Port L ---------------------------- */

	__REG(GPLCON0) =
		(1 << 0)  | /* GPL0  - OUTPUT  (NC) */
		(1 << 4)  | /* GPL1  - OUTPUT  (NC) */
		(1 << 8)  | /* GPL2  - OUTPUT  (NC) */
		(1 << 12) | /* GPL3  - OUTPUT  (NC) */
		(1 << 16) | /* GPL4  - OUTPUT  (NC) */
		(1 << 20) | /* GPL5  - OUTPUT  (NC) */
		(1 << 24) | /* GPL6  - OUTPUT  (NC) */
		(1 << 28)   /* GPL7  - OUTPUT  (NC) */
	;
	__REG(GPLCON1) =
		(1 << 0)  | /* GPL8  - OUTPUT  (NC) */
		(1 << 4)  | /* GPL9  - OUTPUT  (NC) */
		(1 << 8)  | /* GPL10 - OUTPUT  (NC) */
		(1 << 12) | /* GPL11 - OUTPUT  (NC) */
		(1 << 16) | /* GPL12 - OUTPUT  (NC) */
		(1 << 20) | /* GPL13 - OUTPUT  (NC) */
		(1 << 24)   /* GPL14 - OUTPUT  (NC) */
	;

	__REG(GPLPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPLDAT) = 0;


	/* ---------------------------- Port M ---------------------------- */

	__REG(GPMCON) =
		(1 << 0)  | /* GPM0  - OUTPUT  (TP_RESET) */
		(1 << 4)  | /* GPM1  - OUTPUT  (NC) */
		(1 << 8)  | /* GPM2  - OUTPUT  (GPS_LNA_EN) */
		(1 << 12) | /* GPM3  - OUTPUT  (NC) */
		(0 << 16) | /* GPM4  - INPUT   (nUSB_FLT) */
		(0 << 20)   /* GPM5  - INPUT   (nUSB_OC) */
	;

	__REG(GPMPUD) = (2 << (4 * 2)) | (2 << (5 * 2)); /* Pup on inputs */

	__REG(GPMDAT) = 0;

	/* ---------------------------- Port N ---------------------------- */

	__REG(GPNCON) =
		(2 << 0)  | /* GPN0  - EXINT0 nG1INT1 */
		(2 << 2)  | /* GPN1  - EXINT1 KEY_MINUS */
		(2 << 4)  | /* GPN2  - EXINT2 KEY_PLUS */
		(2 << 6)  | /* GPN3  - EXINT3 PWR_IND */
		(2 << 8)  | /* GPN4  - EXINT4 PWR_IRQ */
		(2 << 10) | /* GPN5  - EXINT5 nTOUCH */
		(2 << 12) | /* GPN6  - EXINT6 nJACK_INSERT */
		(2 << 14) | /* GPN7  - EXINT7 GPS_INT */
		(2 << 16) | /* GPN8  - EXINT8 nHOLD */
		(2 << 18) | /* GPN9  - EXINT9 WLAN_WAKEUP */
		(2 << 20) | /* GPN10 - EXINT10 nG1INT2 */
		(2 << 22) | /* GPN11 - EXINT11 nIO1 */
		(2 << 24) | /* GPN12 - EXINT12 nONKEYWAKE */
		(0 << 26) | /* GPN13 - INPUT (iROM CFG0) */
		(0 << 28) | /* GPN14 - INPUT (iROM CFG1) */
		(0 << 30)   /* GPN15 - INPUT (iROM CFG2) */
	;

	__REG(GPNPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPNDAT) = 0;


	/* ---------------------------- Port O ---------------------------- */

	__REG(GPOCON) =
		(2 << 0)  | /* GPO0  - XM0CS2 (nNANDCS0) */
		(1 << 2)  | /* GPO1  - OUTPUT (nMODEM_RESET) */
		(1 << 4)  | /* GPO2  - OUTPUT  (NC) */
		(1 << 6)  | /* GPO3  - OUTPUT  (NC) */
		(1 << 8)  | /* GPO4  - OUTPUT  (NC) */
		(1 << 10) | /* GPO5  - OUTPUT  (NC) */
		(1 << 12) | /* GPO6  - OUTPUT  (NC) */
		(1 << 14) | /* GPO7  - OUTPUT  (NC) */
		(1 << 16) | /* GPO8  - OUTPUT  (NC) */
		(1 << 18) | /* GPO9  - OUTPUT  (NC) */
		(1 << 20) | /* GPO10 - OUTPUT  (NC) */
		(1 << 22) | /* GPO11 - OUTPUT  (NC) */
		(1 << 24) | /* GPO12 - OUTPUT  (NC) */
		(1 << 26) | /* GPO13 - OUTPUT  (NC) */
		(1 << 28) | /* GPO14 - OUTPUT  (NC) */
		(1 << 30)   /* GPO15 - OUTPUT  (NC) */
	;

	__REG(GPOPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPODAT) = (1 << 15); /* assert CAM_PWRDN */

	__REG(GPOCONSLP) =
		(3 << 0)  | /* GPO0  - hold state */
		(1 << 2)  | /* GPO1  - OUTPUT 1  (do not reset modem) */
		(0 << 4)  | /* GPO2  - OUTPUT 0 */
		(0 << 6)  | /* GPO3  - OUTPUT 0 */
		(0 << 8)  | /* GPO4  - OUTPUT 0 */
		(0 << 10) | /* GPO5  - OUTPUT 0 */
		(0 << 12) | /* GPO6  - OUTPUT 0 */
		(0 << 14) | /* GPO7  - OUTPUT 0 */
		(0 << 16) | /* GPO8  - OUTPUT 0 */
		(0 << 18) | /* GPO9  - OUTPUT 0 */
		(0 << 20) | /* GPO10 - OUTPUT 0 */
		(0 << 22) | /* GPO11 - OUTPUT 0 */
		(0 << 24) | /* GPO12 - OUTPUT 0 */
		(0 << 26) | /* GPO13 - OUTPUT 0 */
		(0 << 28) | /* GPO14 - OUTPUT 0 */
		(0 << 30)   /* GPO15 - OUTPUT 0 */
	;

	__REG(GPOPUDSLP) =
		(0 << 0)  | /* GPO0  - no pull up or down */
		(0 << 2)  | /* GPO1  - no pull up or down */
		(0 << 4)  | /* GPO2  - no pull up or down */
		(0 << 6)  | /* GPO3  - no pull up or down */
		(0 << 8)  | /* GPO4  - no pull up or down */
		(0 << 10) | /* GPO5  - no pull up or down */
		(0 << 12) | /* GPO6  - no pull up or down */
		(0 << 14) | /* GPO7  - no pull up or down */
		(0 << 16) | /* GPO8  - no pull up or down */
		(0 << 18) | /* GPO9  - no pull up or down */
		(0 << 20) | /* GPO10 - no pull up or down */
		(0 << 22) | /* GPO11 - no pull up or down */
		(0 << 24) | /* GPO12 - no pull up or down */
		(0 << 26) | /* GPO13 - no pull up or down */
		(0 << 28) | /* GPO14 - no pull up or down */
		(0 << 30)   /* GPO15 - no pull up or down */
	;

	/* ---------------------------- Port P ---------------------------- */

	__REG(GPPCON) =
		(1 << 0)  | /* GPP0  - OUTPUT  (NC) */
		(1 << 2)  | /* GPP1  - OUTPUT  (NC) */
		(1 << 4)  | /* GPP2  - OUTPUT  (NC) */
		(1 << 6)  | /* GPP3  - OUTPUT  (NC) */
		(1 << 8)  | /* GPP4  - OUTPUT  (NC) */
		(1 << 10) | /* GPP5  - OUTPUT  (NC) */
		(1 << 12) | /* GPP6  - OUTPUT  (NC) */
		(1 << 14) | /* GPP7  - OUTPUT  (NC) */
		(1 << 16) | /* GPP8  - OUTPUT  (NC) */
		(1 << 18) | /* GPP9  - OUTPUT  (NC) */
		(1 << 20) | /* GPP10 - OUTPUT  (NC) */
		(1 << 22) | /* GPP11 - OUTPUT  (NC) */
		(1 << 24) | /* GPP12 - OUTPUT  (NC) */
		(1 << 26) | /* GPP13 - OUTPUT  (NC) */
		(1 << 28) | /* GPP14 - OUTPUT  (NC) */
		(1 << 30)   /* GPP15 - OUTPUT  (NC) */
	;

	__REG(GPPPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPPDAT) = 0; /* assert CAM_PWRDN */

	__REG(GPPCONSLP) =
		(0 << 0)  | /* GPP0  - OUTPUT 0 */
		(0 << 2)  | /* GPP1  - OUTPUT 0 */
		(0 << 4)  | /* GPP2  - OUTPUT 0 */
		(0 << 6)  | /* GPP3  - OUTPUT 0 */
		(0 << 8)  | /* GPP4  - OUTPUT 0 */
		(0 << 10) | /* GPP5  - OUTPUT 0 */
		(0 << 12) | /* GPP6  - OUTPUT 0 */
		(0 << 14) | /* GPP7  - OUTPUT 0 */
		(0 << 16) | /* GPP8  - OUTPUT 0 */
		(0 << 18) | /* GPP9  - OUTPUT 0 */
		(0 << 20) | /* GPP10 - OUTPUT 0 */
		(0 << 22) | /* GPP11 - OUTPUT 0 */
		(0 << 24) | /* GPP12 - OUTPUT 0 */
		(0 << 26) | /* GPP13 - OUTPUT 0 */
		(0 << 28) | /* GPP14 - OUTPUT 0 */
		(0 << 30)   /* GPP15 - OUTPUT 0 */
	;

	__REG(GPPPUDSLP) =
		(0 << 0)  | /* GPP0  - no pull up or down */
		(0 << 2)  | /* GPP1  - no pull up or down */
		(0 << 4)  | /* GPP2  - no pull up or down */
		(0 << 6)  | /* GPP3  - no pull up or down */
		(0 << 8)  | /* GPP4  - no pull up or down */
		(0 << 10) | /* GPP5  - no pull up or down */
		(0 << 12) | /* GPP6  - no pull up or down */
		(0 << 14) | /* GPP7  - no pull up or down */
		(0 << 16) | /* GPP8  - no pull up or down */
		(0 << 18) | /* GPP9  - no pull up or down */
		(0 << 20) | /* GPP10 - no pull up or down */
		(0 << 22) | /* GPP11 - no pull up or down */
		(0 << 24) | /* GPP12 - no pull up or down */
		(0 << 26) | /* GPP13 - no pull up or down */
		(0 << 28) | /* GPP14 - no pull up or down */
		(0 << 30)   /* GPP15 - no pull up or down */
	;

	/* ---------------------------- Port Q ---------------------------- */

	__REG(GPQCON) =
		(1 << 0)  | /* GPQ0  - OUTPUT  (NC) */
		(1 << 2)  | /* GPQ1  - OUTPUT  (NC) */
		(1 << 4)  | /* GPQ2  - OUTPUT  (NC) */
		(1 << 6)  | /* GPQ3  - OUTPUT  (NC) */
		(1 << 8)  | /* GPQ4  - OUTPUT  (NC) */
		(1 << 10) | /* GPQ5  - OUTPUT  (NC) */
		(1 << 12) | /* GPQ6  - OUTPUT  (NC) */
		(1 << 14) | /* GPQ7  - OUTPUT  (NC) */
		(1 << 16) | /* GPQ8  - OUTPUT  (NC) */
		(1 << 18) | /* GPQ9  - OUTPUT  (NC) */
		(1 << 20) | /* GPQ10 - OUTPUT  (NC) */
		(1 << 22) | /* GPQ11 - OUTPUT  (NC) */
		(1 << 24) | /* GPQ12 - OUTPUT  (NC) */
		(1 << 26) | /* GPQ13 - OUTPUT  (NC) */
		(1 << 28) | /* GPQ14 - OUTPUT  (NC) */
		(1 << 30)   /* GPQ15 - OUTPUT  (NC) */
	;

	__REG(GPQPUD) = 0; /* all pullup and pulldown disabled */

	__REG(GPQDAT) = 0; /* assert CAM_PWRDN */

	__REG(GPQCONSLP) =
		(0 << 0)  | /* GPQ0  - OUTPUT 0 */
		(0 << 2)  | /* GPQ1  - OUTPUT 0 */
		(0 << 4)  | /* GPQ2  - OUTPUT 0 */
		(0 << 6)  | /* GPQ3  - OUTPUT 0 */
		(0 << 8)  | /* GPQ4  - OUTPUT 0 */
		(0 << 10) | /* GPQ5  - OUTPUT 0 */
		(0 << 12) | /* GPQ6  - OUTPUT 0 */
		(0 << 14) | /* GPQ7  - OUTPUT 0 */
		(0 << 16) | /* GPQ8  - OUTPUT 0 */
		(0 << 18) | /* GPQ9  - OUTPUT 0 */
		(0 << 20) | /* GPQ10 - OUTPUT 0 */
		(0 << 22) | /* GPQ11 - OUTPUT 0 */
		(0 << 24) | /* GPQ12 - OUTPUT 0 */
		(0 << 26) | /* GPQ13 - OUTPUT 0 */
		(0 << 28) | /* GPQ14 - OUTPUT 0 */
		(0 << 30)   /* GPQ15 - OUTPUT 0 */
	;

	__REG(GPQPUDSLP) =
		(0 << 0)  | /* GPQ0  - no pull up or down */
		(0 << 2)  | /* GPQ1  - no pull up or down */
		(0 << 4)  | /* GPQ2  - no pull up or down */
		(0 << 6)  | /* GPQ3  - no pull up or down */
		(0 << 8)  | /* GPQ4  - no pull up or down */
		(0 << 10) | /* GPQ5  - no pull up or down */
		(0 << 12) | /* GPQ6  - no pull up or down */
		(0 << 14) | /* GPQ7  - no pull up or down */
		(0 << 16) | /* GPQ8  - no pull up or down */
		(0 << 18) | /* GPQ9  - no pull up or down */
		(0 << 20) | /* GPQ10 - no pull up or down */
		(0 << 22) | /* GPQ11 - no pull up or down */
		(0 << 24) | /* GPQ12 - no pull up or down */
		(0 << 26) | /* GPQ13 - no pull up or down */
		(0 << 28) | /* GPQ14 - no pull up or down */
		(0 << 30)   /* GPQ15 - no pull up or down */
	;

	/*
	 * We have to talk to the PMU a little bit
	 */

	for (n = 0; n < ARRAY_SIZE(gta03_pcf50633_init); n++)
		i2c_write_sync(&bb_s3c6410, PCF50633_I2C_ADS,
			       gta03_pcf50633_init[n].index,
			       gta03_pcf50633_init[n].value);

}

int gta03_get_pcb_revision(void)
{
	u32 v = __REG(GPIDAT);

	return (v & (1 << 8)) >> (8 - 2) |
	       (v & (1 << 1)) >> 0 |
	       (v & (1 << 0)) >> 0
	;
}

const struct board_variant const * get_board_variant_gta03(void)
{
	return &board_variants[gta03_get_pcb_revision()];
}

