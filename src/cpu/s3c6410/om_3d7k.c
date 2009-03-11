#include <qi.h>
#include <neo_om_3d7k.h>
#include <s3c6410.h>
#include <serial-s3c64xx.h>
#include <i2c-bitbang-s3c6410.h>
#include <pcf50633.h>

#define PCF50633_I2C_ADS 0x73

const struct pcf50633_init om_3d7k_pcf50633_init[] = {

	{ PCF50633_REG_OOCWAKE,		0xd3 }, /* wake from ONKEY,EXTON!,RTC,USB,ADP */
	{ PCF50633_REG_OOCTIM1,		0xaa },	/* debounce 14ms everything */
	{ PCF50633_REG_OOCTIM2,		0x4a },
	{ PCF50633_REG_OOCMODE,		0x55 },
	{ PCF50633_REG_OOCCTL,		0x47 },

	{ PCF50633_REG_SVMCTL,		0x08 },	/* 3.10V SYS voltage thresh. */
	{ PCF50633_REG_BVMCTL,		0x02 },	/* 2.80V BAT voltage thresh. */

	{ PCF50633_REG_AUTOENA,		0x01 },	/* always on */

	{ PCF50633_REG_DOWN1OUT,	0x17 }, /* 1.2V (0x17 * .025V + 0.625V) */

	/* all of these are down in 3d7k suspend except MEMLDO */

	{ PCF50633_REG_DOWN1ENA,	0x02 }, /* enabled if GPIO1 = HIGH */
	{ PCF50633_REG_DOWN2ENA,	0x02 }, /* enabled if GPIO1 = HIGH */
	{ PCF50633_REG_HCLDOENA,	0x00 }, /* Camera 2.8V power off */
	{ PCF50633_REG_MEMLDOENA,	0x01 }, /* Memory LDO always ON */
	{ PCF50633_REG_LDO1ENA,		0x00 }, /* Gsensor power off */
	{ PCF50633_REG_LDO2ENA,		0x00 }, /* Camera 1.5V power off */
	{ PCF50633_REG_LDO3ENA,		0x02 }, /* Codec power ON */
	{ PCF50633_REG_LDO4ENA,		0x02 }, /* SD power ON */
	{ PCF50633_REG_LDO5ENA,		0x00 }, /* BT power off */
	{ PCF50633_REG_LDO6ENA,		0x00 }, /* LCM power off */

	{ PCF50633_REG_INT1M,		0x00 },
	{ PCF50633_REG_INT2M,		0x00 },
	{ PCF50633_REG_INT3M,		0x00 },
	{ PCF50633_REG_INT4M,		0x00 },
	{ PCF50633_REG_INT5M,		0x00 },

	{ PCF50633_REG_MBCC2,		0x28 },	/* Vbatconid=2.7V, Vmax=4.20V */
	{ PCF50633_REG_MBCC3,		0x19 },	/* 25/255 == 98mA pre-charge */
	{ PCF50633_REG_MBCC4,		0xff }, /* 255/255 == 1A adapter fast */
	{ PCF50633_REG_MBCC5,		0xff },	/* 255/255 == 1A USB fast */

	{ PCF50633_REG_MBCC6,		0x00 }, /* cutoff current 1/32 * Ichg */

	/* current prototype is pulling > 100mA at startup */
	{ PCF50633_REG_MBCC7,		0xc1 },	/* 2.2A max bat curr, USB 500mA */

	{ PCF50633_REG_MBCC8,		0x00 },
	{ PCF50633_REG_MBCC1,		0xff }, /* chgena */

	{ PCF50633_REG_BBCCTL,		0x19 },	/* 3V, 200uA, on */
	{ PCF50633_REG_OOCSHDWN,	0x04 },  /* defeat 8s death from lowsys on A5 */

};

static const struct board_variant board_variants[] = {
	[0] = {
		.name = "OM 3D7K unknown",
		.machine_revision = 0
	},
	[1] = {
		.name = "OM 3D7K A1",
		.machine_revision = 1
	},
	[2] = {
		.name = "OM 3D7K A2",
		.machine_revision = 2
	},
	[3] = {
		.name = "OM 3D7K A3",
		.machine_revision = 3
	},
	[4] = {
		.name = "OM 3D7K A4",
		.machine_revision = 4
	},
	[5] = {
		.name = "OM 3D7K A5",
		.machine_revision = 5
	},
	[6] = {
		.name = "OM 3D7K A6",
		.machine_revision = 6
	},
	[7] = {
		.name = "OM 3D7K A7",
		.machine_revision = 7
	}
};

#define S0    0
#define S1    1
#define SIN   2
#define SHOLD 3

#define SNP   0
#define SPD   1
#define SPU   2

void port_init_om_3d7k(void)
{
	int n;

	__REG(HCLK_GATE) =
		(0 << 31) | /* 3D unit */
		(1 << 30) | /* reserved */
		(0 << 29) | /* USB host */
		(0 << 28) | /* "security subsystem" */
		(1 << 27) | /* SDMA1 */
		(1 << 26) | /* SDMA0 */
		(0 << 25) | /* iROM */
		(1 << 24) | /* DDR 1 */
		(1 << 23) | /* reserved */
		(1 << 22) | /* DMC1 */
		(1 << 21) | /* SROM / NAND controller / NAND */
		(0 << 20) | /* USB OTG */
		(0 << 19) | /* HSMMC 2 */
		(0 << 18) | /* HSMMC 1 */
		(1 << 17) | /* HSMMC 0 */
		(0 << 16) | /* MDP */
		(0 << 15) | /* direct host */
		(0 << 14) | /* indirect host */
		(1 << 13) | /* DMA1 */
		(1 << 12) | /* DMA0 */
		(0 << 11) | /* JPEG */
		(0 << 10) | /* camera */
		(0 << 9) | /* scaler */
		(0 << 8) | /* 2D */
		(0 << 7) | /* TV */
		(1 << 6) | /* reserved */
		(0 << 5) | /* POST0 */
		(0 << 4) | /* rotator */
		(1 << 3) | /* LCD controller */
		(1 << 2) | /* TZICs */
		(1 << 1) | /* VICs */
		(0 << 0) /* MFC */
	;
	__REG(PCLK_GATE) =
		(0x1f << 28) | /* reserved */
		(0 << 27) | /* I2C1 */
		(0 << 26) | /* IIS2 */
		(1 << 25) | /* reserved */
		(0 << 24) | /* security key */
		(1 << 23) | /* chip ID */
		(0 << 22) | /* SPI1 */
		(0 << 21) | /* SPI0 */
		(0 << 20) | /* HSI RX */
		(0 << 19) | /* HSI TX */
		(1 << 18) | /* GPIO */
		(1 << 17) | /* I2C 0 */
		(1 << 16) | /* IIS1 */
		(1 << 15) | /* IIS0 */
		(0 << 14) | /* AC97 */
		(0 << 13) | /* TZPC */
		(0 << 12) | /* TS ADC */
		(0 << 11) | /* keypad */
		(0 << 10) | /* IRDA */
		(0 << 9) | /* PCM1 */
		(0 << 8) | /* PCM0 */
		(1 << 7) | /* PWM */
		(0 << 6) | /* RTC */
		(1 << 5) | /* WDC */
		(1 << 4) | /* UART3 */
		(1 << 3) | /* UART2 */
		(1 << 2) | /* UART1 */
		(1 << 1) | /* UART0 */
		(0 << 0)  /* MFC */
	;

	__REG(SCLK_GATE) =
		(1 << 31) |
		(0 << 30) | /* USB Host */
		(0 << 29) | /* HSMMC2 48MHz */
		(0 << 28) | /* HSMMC1 48MHz */
		(0 << 27) | /* HSMMC0 48MHz */
		(0 << 26) | /* HSMMC2 */
		(0 << 25) | /* HSMMC1 */
		(1 << 24) | /* HSMMC0 */
		(0 << 23) | /* SPI1 - 48MHz */
		(0 << 22) | /* SPI0 - 48MHz */
		(0 << 21) | /* SPI1 */
		(0 << 20) | /* SPI0 */
		(0 << 19) | /* TV DAC */
		(0 << 18) | /* TV encoder */
		(0 << 17) | /* scaler 27 */
		(0 << 16) | /* scaler */
		(0 << 15) | /* LCD 27MHz */
		(1 << 14) | /* LCD */
		(1 << 13) | /* camera and LCD */
		(0 << 12) | /* POST0 */
		(0 << 11) | /* AUDIO2 */
		(0 << 10) | /* POST0 again */
		(1 << 9) | /* IIS1 */
		(1 << 8) | /* IIS0 */
		(0 << 7) | /* security */
		(0 << 6) | /* IRDA */
		(1 << 5) | /* UART */
		(1 << 4) | /* reserved */
		(0 << 3) | /* MFC */
		(0 << 2) | /* Cam */
		(0 << 1) | /* JPEG */
		(1 << 0)  /* reserved */
	;

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

	__REG(GPAPUD) =  /* pullup inputs */
		0x2222
	;
	__REG(GPADAT) = 0; /* just for determinism */

	__REG(GPACONSLP) =
		(SIN << 0)  |	/* GPA0 bluetooth down in suspend*/
		(S0 << 2) | 	/* GPA1 */
		(SIN << 4)  |	/* GPA2 */
		(S0 << 6) |	/* GPA3 */
		(SIN << 8) |	/* GPA4 gsm */
		(SHOLD << 10) | /* GPA5 */
		(SIN << 12) |	/* GPA6 */
		(SHOLD << 14)	/* GPA7 */
	;
	__REG(GPAPUDSLP) =
		(SPD << 0)  | /* GPA0 */
		(SNP << 2)  | /* GPA1 */
		(SPD << 4)  | /* GPA2 */
		(SNP << 6)  | /* GPA3 */
		(SPU << 8)  | /* GPA4 */
		(SNP << 10) | /* GPA5 */
		(SPU << 12) | /* GPA6 */
		(SNP << 14)   /* GPA7 */
	;

	/* ---------------------------- Port B ---------------------------- */

	__REG(GPBCON) =
		(1 << 0)  | /* GPB0  - (NC) output low */
		(1 << 4)  | /* GPB1  - (NC) output low */
		(2 << 8)  | /* GPB2  - UART_RXD3 */
		(2 << 12) | /* GPB3  - UART_TXD3 */
		(1 << 16) | /* GPB4  - (NC) output low */
		(1 << 20) | /* GPB5  - (I2C BB SCL) OUTPUT */
		(1 << 24)   /* GPB6  - (I2C BB SDA) OUTPUT */
	;

	__REG(GPBPUD) =  /* all pullup and pulldown disabled */
		(SPU << (2 * 2)) /* pullup console rx */
	;

	__REG(GPBDAT) = 0; /* just for determinism */

	__REG(GPBCONSLP) =
		(SHOLD << 0) |	/* GPB0 */
		(SHOLD << 2) |	/* GPB1 */
		(SIN << 4) |	/* GPB2 */
		(SHOLD << 6) |	/* GPB3 */
		(SHOLD << 8) |	/* GPB4 */
		(SIN << 10) |	/* GPB5 ext pullup */
		(SIN << 12)	/* GPB6 ext pullup */
	;

	__REG(GPBPUDSLP) =
		(SNP << 0)  | /* GPB0 */
		(SNP << 2)  | /* GPB1 */
		(SPU << 4)  | /* GPB2 */
		(SNP << 6)  | /* GPB3 */
		(SNP << 8)  | /* GPB4 */
		(SNP << 10) | /* GPB5 */
		(SNP << 12)   /* GPB6 */
	;

	/* ---------------------------- Port C ---------------------------- */

	__REG(GPCCON) =
		(0 << 0)  | /* GPC0  - SPI_MISO0 INPUT   motion sensor spi */
		(1 << 4)  | /* GPC1  - SPI_CLK0  OUTPUT */
		(1 << 8)  | /* GPC2  - SPI_MOSI0 OUTPUT */
		(1 << 12) | /* GPC3  - SPI_CS0   OUTPUT */
		(1 << 16) | /* GPC4  - (NC)      OUTPUT lcm spi*/
		(1 << 20) | /* GPC5  - SPI_CLK1  OUTPUT */
		(1 << 24) | /* GPC6  - SPI_MOSI1 OUTPUT */
		(1 << 28)   /* GPC7  - SPI_CS1   OUTPUT */
	;

	__REG(GPCPUD) =
		(SPD << 0)
	;
	__REG(GPCDAT) = 0; /* just for determinism */

	__REG(GPCCONSLP) = /* both peripherals down in suspend */
		(SIN << 0)  | /* GPC0 */
		(S0 << 2)  | /* GPC1 */
		(S0 << 4)  | /* GPC2 */
		(S0 << 6)  | /* GPC3 */
		(SIN << 8)  | /* GPC4 */
		(S0 << 10) | /* GPC5 */
		(S0 << 12) | /* GPC6 */
		(S0 << 14)   /* GPC7 */
	;

	__REG(GPCPUDSLP) =
		(SPD << 0)  | /* GPC0 */
		(SNP << 2)  | /* GPC1 */
		(SNP << 4)  | /* GPC2 */
		(SNP << 6)  | /* GPC3 */
		(SPD << 8)  | /* GPC4 */
		(SNP << 10) | /* GPC5 */
		(SNP << 12) | /* GPC6 */
		(SNP << 14)   /* GPC7 */
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
		(S0 << 0)  | /* GPD0 */
		(S0 << 2)  | /* GPD1 */
		(S0 << 4)  | /* GPD2 */
		(S0 << 6)  | /* GPD3 */
		(S0 << 8)    /* GPD4 */
	;

	__REG(GPDPUDSLP) =
		(SNP << 0)  | /* GPD0 */
		(SNP << 2)  | /* GPD1 */
		(SNP << 4)  | /* GPD2 */
		(SNP << 6)  | /* GPD3 */
		(SNP << 8)    /* GPD4 */
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
		(S0 << 0)  | /* GPE0 */
		(S0 << 2)  | /* GPE1 */
		(S0 << 4)  | /* GPE2 */
		(S0 << 6)  | /* GPE3 */
		(S0 << 8)    /* GPE4 */
	;

	__REG(GPEPUDSLP) =
		(SNP << 0)  | /* GPE0 */
		(SNP << 2)  | /* GPE1 */
		(SNP << 4)  | /* GPE2 */
		(SNP << 6)  | /* GPE3 */
		(SNP << 8)    /* GPE4 */
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
		(1 << 28) | /* GPF14 - output not CLKOUT0 */
		(1 << 30)   /* GPF15 - OUTPUT CAM_PWRDN */
	;

	__REG(GPFPUD) =
		(SPD << (2 * 12)) |
		(SPD << (2 * 11)) |
		(SPD << (2 * 10)) |
		(SPD << (2 * 9)) |
		(SPD << (2 * 8)) |
		(SPD << (2 * 7)) |
		(SPD << (2 * 6)) |
		(SPD << (2 * 5)); /* all cam data pulldown */

	__REG(GPFDAT) = (1 << 15); /* assert CAM_PWRDN */

	__REG(GPFCONSLP) =
		(S0 << 0)  | /* GPF0 */
		(S0 << 2)  | /* GPF1 */
		(S0 << 4)  | /* GPF2 */
		(S0 << 6)  | /* GPF3 */
		(S0 << 8)  | /* GPF4 */
		(SIN << 10) | /* GPF5 */
		(SIN << 12) | /* GPF6 */
		(SIN << 14) | /* GPF7 */
		(SIN << 16) | /* GPF8 */
		(SIN << 18) | /* GPF9 */
		(SIN << 20) | /* GPF10 */
		(SIN << 22) | /* GPF11 */
		(SIN << 24) | /* GPF12 */
		(S0 << 26) | /* GPF13 */
		(S0 << 28) | /* GPF14 */
		(S0 << 30)   /* GPF15 */
	;

	__REG(GPFPUDSLP) =
		(1 << 10) | /* GPF5  - pull down */
		(1 << 12) | /* GPF6  - pull down */
		(1 << 14) | /* GPF7  - pull down */
		(1 << 16) | /* GPF8  - pull down */
		(1 << 18) | /* GPF9  - pull down */
		(1 << 20) | /* GPF10 - pull down */
		(1 << 22) | /* GPF11 - pull down */
		(1 << 24)   /* GPF12 - pull down */
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
		(S0 << 0)  | /* GPG0  - it's not powered*/
		(S0 << 2)  | /* GPG1 */
		(S0 << 4)  | /* GPG2 */
		(S0 << 6)  | /* GPG3 */
		(S0 << 8)  | /* GPG4 */
		(S0 << 10) | /* GPG5 */
		(S0 << 12)   /* GPG6 */
	;

	__REG(GPGPUDSLP) = 0;

	/* ---------------------------- Port H ---------------------------- */

	__REG(GPHCON0) =
		(1 << 0)  | /* GPH0  - NC OUT 0 */
		(1 << 4)  | /* GPH1  - NC OUT 0 */
		(1 << 8)  | /* GPH2  - NC OUT 0 */
		(1 << 12) | /* GPH3  - NC OUT 0 */
		(1 << 16) | /* GPH4  - NC OUT 0 */
		(1 << 20) | /* GPH5  - NC OUT 0 */
		(1 << 24) | /* GPH6  - OUTPUT nBT_RESET */
		(0 << 28)   /* GPH7  - INPUT HDQ */
	;
	__REG(GPHCON1) =
		(1 << 0) | /* GPH8  - OUTPUT BT PIO5 */
		(0 << 4)   /* GPH9  - INPUT LED INT */
	;

	__REG(GPHPUD) = (SPU << (9 * 2)) | (SPU << (7 * 2));

	__REG(GPHDAT) = 0;

	__REG(GPHCONSLP) =
		(S0 << 0)  | /* GPH0 */
		(S0 << 2)  | /* GPH1 */
		(S0 << 4)  | /* GPH2 */
		(S0 << 6)  | /* GPH3 */
		(S0 << 8)  | /* GPH4 */
		(S0 << 10) | /* GPH5 */
		(S0 << 12) | /* GPH6 */
		(SHOLD << 14) | /* GPH7  - INPUT (HDQ) */
		(S0 << 16) | /* GPH8 */
		(SIN << 18)   /* GPH9 */
	;

	__REG(GPHPUDSLP) = (SPU << (9 * 2));

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
		(SIN << 0)  | /* GPI0  - input */
		(SIN << 2)  | /* GPI1  - input */
		(S0 << 4)  | /* GPI2  - input */
		(S0 << 6)  | /* GPI3  - input */
		(S0 << 8)  | /* GPI4  - input */
		(S0 << 10) | /* GPI5  - input */
		(S0 << 12) | /* GPI6  - input */
		(S0 << 14) | /* GPI7  - input */
		(SIN << 16) | /* GPI8  - input */
		(S0 << 18) | /* GPI9  - input */
		(S0 << 20) | /* GPI10 - input */
		(S0 << 22) | /* GPI11 - input */
		(S0 << 24) | /* GPI12 - input */
		(S0 << 26) | /* GPI13 - input */
		(S0 << 28) | /* GPI14 - input */
		(S0 << 30)   /* GPI15 - input */
	;

	__REG(GPIPUDSLP) =
		(1 << 0)  | /* GPI0  - pull down */
		(1 << 2)  | /* GPI1  - pull down */
		(1 << 16)   /* GPI8  - pull down */
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
		(S0 << 0)  | /* GPJ0 */
		(S0 << 2)  | /* GPJ1 */
		(S0 << 4)  | /* GPJ2 */
		(S0 << 6)  | /* GPJ3 */
		(S0 << 8)  | /* GPJ4 */
		(S0 << 10) | /* GPJ5 */
		(S0 << 12) | /* GPJ6 */
		(S0 << 14) | /* GPJ7 */
		(S0 << 16) | /* GPJ8 */
		(S0 << 18) | /* GPJ9 */
		(S0 << 20) | /* GPJ10 */
		(S0 << 22)   /* GPJ11 */
	;

	__REG(GPJPUDSLP) =
		0
	;

	/* ---------------------------- Port K ---------------------------- */

	__REG(GPKCON0) =
		(1 << 0)  | /* GPK0  - OUTPUT NC */
		(1 << 4)  | /* GPK1  - OUTPUT NC */
		(1 << 8)  | /* GPK2  - OUTPUT  (nMODEM_ON) */

		(1 << 12) | /* GPK3  - OUTPUT  (LED_TRIG) */
		(1 << 16) | /* GPK4  - OUTPUT  (LED_EN) */
		(0 << 20) | /* GPK5  - OUTPUT NC */
		(1 << 24) | /* GPK6  - OUTPUT  (LCD_RESET) */
		(0 << 28)   /* GPK7  - OUTPUT NC */
	;
	__REG(GPKCON1) =
		(1 << 0)  | /* GPK8  - OUTPUT NC */
		(1 << 4)  | /* GPK9  - OUTPUT NC */
		(1 << 8)  | /* GPK10 - OUTPUT NC */
		(1 << 12) | /* GPK11 - OUTPUT NC */
		(1 << 16) | /* GPK12 - OUTPUT NC */
		(1 << 20) | /* GPK13 - OUTPUT NC */
		(1 << 24) | /* GPK14 - OUTPUT NC */
		(1 << 28)   /* GPK15 - OUTPUT NC */
	;

	__REG(GPKPUD) = 0;

	__REG(GPKDAT) = /* rest output 0 */
		(SHOLD << (2 * 2))  | /* nMODEM_ON */
		(SHOLD << (2 * 3))  | /* LED_TRIG */
		(SHOLD << (2 * 4))  | /* LED_EN */
		(S0 <<    (2 * 6))    /* LCD_RESET */
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
		(1 << 8)  | /* GPM2  - OUTPUT  (NC) */
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
		(1 << 14) | /* GPN7  - EXINT7 NC OUTPUT */
		(2 << 16) | /* GPN8  - EXINT8 nHOLD */
		(2 << 18) | /* GPN9  - EXINT9 WLAN_WAKEUP */
		(2 << 20) | /* GPN10 - EXINT10 nG1INT2 */
		(2 << 22) | /* GPN11 - EXINT11 nIO1 */
		(2 << 24) | /* GPN12 - EXINT12 nONKEYWAKE */
		(0 << 26) | /* GPN13 - INPUT (iROM CFG0) */
		(0 << 28) | /* GPN14 - INPUT (iROM CFG1) */
		(0 << 30)   /* GPN15 - INPUT (iROM CFG2) */
	;

	__REG(GPNPUD) =
		(SPD << 0)  | /* GPN0  - EXINT0 nG1INT1 */
		(SPU << 2)  | /* GPN1  - EXINT1 KEY_MINUS */
		(SPU << 4)  | /* GPN2  - EXINT2 KEY_PLUS */
		(SPU << 6)  | /* GPN3  - EXINT3 PWR_IND */
		(SNP << 8)  | /* GPN4  - EXINT4 PWR_IRQ */
		(SPU << 10) | /* GPN5  - EXINT5 nTOUCH */
		(SNP << 12) | /* GPN6  - EXINT6 nJACK_INSERT */
		(SNP << 14) | /* GPN7  - EXINT7 NC OP */
		(SPU << 16) | /* GPN8  - EXINT8 nHOLD */
		(SPU << 18) | /* GPN9  - EXINT9 BT_WAKEUP */
		(SPD << 20) | /* GPN10 - EXINT10 nG1INT2 */
		(SPD << 22) | /* GPN11 - EXINT11 nIO1 */
		(SPU << 24) | /* GPN12 - EXINT12 nONKEYWAKE */
		(SPD << 26) | /* GPN13 - INPUT (iROM CFG0) */
		(SPD << 28) | /* GPN14 - INPUT (iROM CFG1) */
		(SPD << 30)   /* GPN15 - INPUT (iROM CFG2) */
	;

	__REG(GPNDAT) = 0;


	/* ---------------------------- Port O ---------------------------- */

	__REG(GPOCON) =
		(2 << 0)  | /* GPO0  - XM0CS2 (nNANDCS0) */
		(1 << 2)  | /* GPO1  - OUTPUT (nMODEM_RESET) */
		(1 << 4)  | /* GPO2  - input  (NC) */
		(1 << 6)  | /* GPO3  - input  (NC) */
		(1 << 8)  | /* GPO4  - input  (NC) */
		(1 << 10) | /* GPO5  - input  (NC) */
		(1 << 12) | /* GPO6  - input  (NC) */
		(1 << 14) | /* GPO7  - input  (NC) */
		(1 << 16) | /* GPO8  - input  (NC) */
		(1 << 18) | /* GPO9  - input  (NC) */
		(1 << 20) | /* GPO10 - input  (NC) */
		(1 << 22) | /* GPO11 - input  (NC) */
		(1 << 24) | /* GPO12 - input  (NC) */
		(1 << 26) | /* GPO13 - input  (NC) */
		(1 << 28) | /* GPO14 - input  (NC) */
		(1 << 30)   /* GPO15 - input  (NC) */
	;

	__REG(GPOPUD) = 0; /* no pulling */

	__REG(GPODAT) = (1 << 15); /* assert CAM_PWRDN */

	__REG(GPOCONSLP) =
		(SHOLD << 0)  | /* GPO0  - hold state */
		(SHOLD << 2)  | /* GPO1  - OUTPUT 1  (do not reset modem) */
		(S0 << 4)  | /* GPO2  - OUTPUT 0 */
		(S0 << 6)  | /* GPO3  - OUTPUT 0 */
		(S0 << 8)  | /* GPO4  - OUTPUT 0 */
		(S0 << 10) | /* GPO5  - OUTPUT 0 */
		(S0 << 12) | /* GPO6  - OUTPUT 0 */
		(S0 << 14) | /* GPO7  - OUTPUT 0 */
		(S0 << 16) | /* GPO8  - OUTPUT 0 */
		(S0 << 18) | /* GPO9  - OUTPUT 0 */
		(S0 << 20) | /* GPO10 - OUTPUT 0 */
		(S0 << 22) | /* GPO11 - OUTPUT 0 */
		(S0 << 24) | /* GPO12 - OUTPUT 0 */
		(S0 << 26) | /* GPO13 - OUTPUT 0 */
		(S0 << 28) | /* GPO14 - OUTPUT 0 */
		(S0 << 30)   /* GPO15 - OUTPUT 0 */
	;

	__REG(GPOPUDSLP) =
		0
	;

	/* ---------------------------- Port P ---------------------------- */

	__REG(GPPCON) =
		(1 << 0)  | /* GPP0  - input  (NC) */
		(1 << 2)  | /* GPP1  - input  (NC) */
		(1 << 4)  | /* GPP2  - input  (NC) */
		(1 << 6)  | /* GPP3  - input  (NC) */
		(1 << 8)  | /* GPP4  - input  (NC) */
		(1 << 10) | /* GPP5  - input  (NC) */
		(1 << 12) | /* GPP6  - input  (NC) */
		(1 << 14) | /* GPP7  - input  (NC) */
		(1 << 16) | /* GPP8  - input  (NC) */
		(1 << 18) | /* GPP9  - input  (NC) */
		(1 << 20) | /* GPP10 - input  (NC) */
		(1 << 22) | /* GPP11 - input  (NC) */
		(1 << 24) | /* GPP12 - input  (NC) */
		(1 << 26) | /* GPP13 - input  (NC) */
		(1 << 28) | /* GPP14 - input  (NC) */
		(1 << 30)   /* GPP15 - input  (NC) */
	;

	__REG(GPPPUD) = 0; /* no pull */

	__REG(GPPDAT) = 0;

	__REG(GPPCONSLP) =
		(S0 << 0)  | /* GPP0  - OUTPUT 0 */
		(S0 << 2)  | /* GPP1  - OUTPUT 0 */
		(S0 << 4)  | /* GPP2  - OUTPUT 0 */
		(S0 << 6)  | /* GPP3  - OUTPUT 0 */
		(S0 << 8)  | /* GPP4  - OUTPUT 0 */
		(S0 << 10) | /* GPP5  - OUTPUT 0 */
		(S0 << 12) | /* GPP6  - OUTPUT 0 */
		(S0 << 14) | /* GPP7  - OUTPUT 0 */
		(S0 << 16) | /* GPP8  - OUTPUT 0 */
		(S0 << 18) | /* GPP9  - OUTPUT 0 */
		(S0 << 20) | /* GPP10 - OUTPUT 0 */
		(S0 << 22) | /* GPP11 - OUTPUT 0 */
		(S0 << 24) | /* GPP12 - OUTPUT 0 */
		(S0 << 26) | /* GPP13 - OUTPUT 0 */
		(S0 << 28) | /* GPP14 - OUTPUT 0 */
		(S0 << 30)   /* GPP15 - OUTPUT 0 */
	;

	__REG(GPPPUDSLP) = 0;

	/* ---------------------------- Port Q ---------------------------- */

	__REG(GPQCON) =
		(1 << 0)  | /* GPQ0  - input  (NC) */
		(1 << 2)  | /* GPQ1  - input  (NC) */
		(1 << 4)  | /* GPQ2  - input  (NC) */
		(1 << 6)  | /* GPQ3  - input  (NC) */
		(1 << 8)  | /* GPQ4  - input  (NC) */
		(1 << 10) | /* GPQ5  - input  (NC) */
		(1 << 12) | /* GPQ6  - input  (NC) */
		(1 << 14) | /* GPQ7  - input  (NC) */
		(1 << 16)   /* GPQ8  - input  (NC) */
	;

	__REG(GPQPUD) = 0; /* all pulldown */

	__REG(GPQDAT) = 0; /* assert CAM_PWRDN */

	__REG(GPQCONSLP) =
		(S0 << 0)  | /* GPQ0  - OUTPUT 0 */
		(S0 << 2)  | /* GPQ1  - OUTPUT 0 */
		(S0 << 4)  | /* GPQ2  - OUTPUT 0 */
		(S0 << 6)  | /* GPQ3  - OUTPUT 0 */
		(S0 << 8)  | /* GPQ4  - OUTPUT 0 */
		(S0 << 10) | /* GPQ5  - OUTPUT 0 */
		(S0 << 12) | /* GPQ6  - OUTPUT 0 */
		(S0 << 14) | /* GPQ7  - OUTPUT 0 */
		(S0 << 16)   /* GPQ8  - OUTPUT 0 */
	;

	__REG(GPQPUDSLP) = 0;

	/* LCD Controller enable */

	__REG(0x7410800c) = 0;
	__REG(0x7f0081a0) = 0xbfc11501;

	/*
	 * We have to talk to the PMU a little bit
	 */
	for (n = 0; n < ARRAY_SIZE(om_3d7k_pcf50633_init); n++)
		i2c_write_sync(&bb_s3c6410, PCF50633_I2C_ADS,
			       om_3d7k_pcf50633_init[n].index,
			       om_3d7k_pcf50633_init[n].value);

}

int om_3d7k_get_pcb_revision(void)
{
	u32 v = __REG(GPIDAT);
        /*
         * PCB rev is 3 bit code (info from Dkay)
         * (b2, b1, b0) = (0,0,1) => pcb rev A1
         * maximum rev = A7
         * bit0 = GPI8
         * bit1 = GPI1
         * bit2 = GPI0
         */

        return (
                ((v & (1 << 8)) ? 1 : 0) |
                ((v & (1 << 1)) ? 2 : 0) |
                ((v & (1 << 0)) ? 4 : 0)
                );
}

const struct board_variant const * get_board_variant_om_3d7k(void)
{
	return &board_variants[om_3d7k_get_pcb_revision()];
}

