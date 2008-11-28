#include <qi.h>
#include <neo_gta03.h>
#include <serial-s3c64xx.h>
//#include <ports-s3c24xx.h>
//#include <i2c-bitbang-s3c24xx.h>
#include <pcf50633.h>

#define GTA03_DEBUG_UART 0

#define PCF50633_I2C_ADS 0x73


static const struct board_variant board_variants[] = {
	[0] = {
		.name = "SMDK",
		.machine_revision = 0,
	},
	[1] = {
		.name = "GTA03 EVT1",
		.machine_revision = 1
	}
};

void port_init_gta03(void)
{
#if 0
	unsigned int * MPLLCON = (unsigned int *)0x4c000004;
	unsigned int * UPLLCON = (unsigned int *)0x4c000008;
	unsigned int * CLKDIVN = (unsigned int *)0x4c000014;

	//CAUTION:Follow the configuration order for setting the ports.
	// 1) setting value(GPnDAT)
	// 2) setting control register  (GPnCON)
	// 3) configure pull-up resistor(GPnUP)

	/* 32bit data bus configuration   */
	/*
	 * === PORT A GROUP
	 *     Ports  : GPA22 GPA21  GPA20 GPA19 GPA18 GPA17 GPA16 GPA15 GPA14 GPA13 GPA12
	 *     Signal : nFCE nRSTOUT nFRE   nFWE  ALE   CLE  nGCS5 nGCS4 nGCS3 nGCS2 nGCS1
	 *     Binary :  1     1      1  , 1   1   1    1   ,  1     1     1     1
	 *     Ports  : GPA11   GPA10  GPA9   GPA8   GPA7   GPA6   GPA5   GPA4   GPA3   GPA2   GPA1  GPA0
	 *     Signal : ADDR26 ADDR25 ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0
	 *     Binary :  1       1      1      1   , 1       1      1      1   ,  1       1     1      1
	 */
	rGPACON = 0x007F8FFF;
	/*
	 * ===* PORT B GROUP
	 *      Ports  : GPB10    GPB9    GPB8    GPB7    GPB6     GPB5    GPB4   GPB3   GPB2     GPB1      GPB0
	 *      Signal : nXDREQ0 nXDACK0 nXDREQ1 nXDACK1 nSS_KBD nDIS_OFF L3CLOCK L3DATA L3MODE nIrDATXDEN Keyboard
	 *      Setting: INPUT  OUTPUT   INPUT  OUTPUT   INPUT   OUTPUT   OUTPUT OUTPUT OUTPUT   OUTPUT    OUTPUT
	 *      Binary :   00  ,  01       00  ,   01      00   ,  01       01  ,   01     01   ,  01        01
	 */
	rGPBCON = 0x00145554;
	rGPBDAT |= (1 <<9 );	/* USB_PULLUP */
	rGPBUP = 0x000007FF;
	/*
	 * === PORT C GROUP
	 *     Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8  GPC7   GPC6   GPC5 GPC4 GPC3  GPC2  GPC1 GPC0
	 *     Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND
	 *     Binary :  10   10  , 10    10  , 10    10  , 10   10  , 10     10  ,  10   10 , 10     10 , 10   10
	 */
	rGPCCON = 0xAAA776E9;
	rGPCUP = 0x0000FFFF;
	rGPCDAT |= (1 << 9); /* WLAN_nRESET pull high */
	/*
	 * === PORT D GROUP
	 *     Ports  : GPD15 GPD14 GPD13 GPD12 GPD11 GPD10 GPD9 GPD8 GPD7 GPD6 GPD5 GPD4 GPD3 GPD2 GPD1 GPD0
	 *     Signal : VD23  VD22  VD21  VD20  VD19  VD18  VD17 VD16 VD15 VD14 VD13 VD12 VD11 VD10 VD9  VD8
	 *     Binary : 10    10  , 10    10  , 10    10  , 10   10 , 10   10 , 10   10 , 10   10 ,10   10
	 */
	rGPDCON = 0xAAA0AAA5;
	rGPDUP = 0x0000FFFF;
	/*
	 * === PORT E GROUP
	 *     Ports  : GPE15  GPE14 GPE13   GPE12   GPE11   GPE10   GPE9    GPE8     GPE7  GPE6  GPE5   GPE4
	 *     Signal : IICSDA IICSCL SPICLK SPIMOSI SPIMISO SDDATA3 SDDATA2 SDDATA1 SDDATA0 SDCMD SDCLK I2SSDO
	 *     Binary :  10     10  ,  10      10  ,  10      10   ,  10      10   ,   10    10  , 10     10  ,
	 *     -------------------------------------------------------------------------------------------------------
	 *     Ports  :  GPE3   GPE2  GPE1    GPE0
	 *     Signal : I2SSDI CDCLK I2SSCLK I2SLRCK
	 *     Binary :  10     10  ,  10      10
	 */
	rGPECON = 0xAAAAAAAA;
	rGPEUP = 0x0000FFFF;
	/*
	 * === PORT F GROUP
	 *     Ports  : GPF7   GPF6   GPF5   GPF4      GPF3     GPF2  GPF1   GPF0
	 *     Signal : nLED_8 nLED_4 nLED_2 nLED_1 nIRQ_PCMCIA EINT2 KBDINT EINT0
	 *     Setting: Output Output Output Output    EINT3    EINT2 EINT1  EINT0
	 *     Binary :  01      01 ,  01     01  ,     10       10  , 10     10
	 */
	rGPFCON = 0x0000AAAA;
	rGPFUP = 0x000000FF;

	/*
	 * === PORT G GROUP
	 *     Ports  : GPG15 GPG14 GPG13 GPG12 GPG11    GPG10    GPG9     GPG8     GPG7      GPG6
	 *     Signal : nYPON  YMON nXPON XMON  EINT19 DMAMODE1 DMAMODE0 DMASTART KBDSPICLK KBDSPIMOSI
	 *     Setting: nYPON  YMON nXPON XMON  EINT19  Output   Output   Output   SPICLK1    SPIMOSI1
	 *     Binary :   11    11 , 11    11  , 10      01    ,   01       01   ,    11         11
	 *     -----------------------------------------------------------------------------------------
	 *     Ports  :    GPG5       GPG4    GPG3    GPG2    GPG1    GPG0
	 *     Signal : KBDSPIMISO LCD_PWREN EINT11 nSS_SPI IRQ_LAN IRQ_PCMCIA
	 *     Setting:  SPIMISO1  LCD_PWRDN EINT11   nSS0   EINT9    EINT8
	 *     Binary :     11         11   ,  10      11  ,  10        10
	 */
	rGPGCON = 0x02A9FE5A;
	rGPGUP = 0x0000FFFF;

	/*
	 * === PORT H GROUP
	 *     Ports  :  GPH10    GPH9  GPH8 GPH7  GPH6  GPH5 GPH4 GPH3 GPH2 GPH1  GPH0
	 *     Signal : CLKOUT1 CLKOUT0 UCLK RXD2 TXD2 RXD1 TXD1 RXD0 TXD0 nRTS0 nCTS0
	 *     Binary :   10   ,  10     10 , 11    11  , 10   10 , 10   10 , 10    10
	 */
	/* pulldown on GPH08: UEXTCLK, just floats!
	 * pulldown GPH0 -- nCTS0 / RTS_MODEM -- floats when GSM off
	 * pulldown GPH3 -- RXD[0] / TX_MODEM -- floats when GSM off
	 */
	rGPHCON = 0x0019A0AA;
	rGPHUP = 0x000007FF;

	/* pulldown on GPJ00: input, just floats! */
	/* pulldown on GPJ07: WLAN module WLAN_GPIO0, no ext pull */
	rGPJCON = 0x02AAAAAA;
	rGPJUP = 0x1FFFF;

	/*
	 * We have to talk to the PMU a little bit
	 */

	/* We need SD Card rail (HCLDO) at 3.0V */
	i2c_write_sync(&bb_s3c24xx, PCF50633_I2C_ADS, PCF50633_REG_HCLDOOUT,
									    21);

	/* switch HCLDO on */
	i2c_write_sync(&bb_s3c24xx, PCF50633_I2C_ADS, PCF50633_REG_HCLDOENA, 1);

	/* push DOWN1 (CPU Core rail) to 1.7V, allowing 533MHz */
	i2c_write_sync(&bb_s3c24xx, PCF50633_I2C_ADS, PCF50633_REG_DOWN1OUT,
									  0x2b);
#endif

}

/**
 * returns PCB revision information in b0, d8, d9
 * GTA03 EVB returns 0x000
 * GTA03 returns 0x001
 */

int gta03_get_pcb_revision(void)
{
	return 0; /* always SMDK right now */
}

const struct board_variant const * get_board_variant_gta03(void)
{
	return &board_variants[gta03_get_pcb_revision()];
}

int is_this_board_gta03(void)
{
	/* FIXME: find something gta03 specific */
	return 1;
}

static  __attribute__ (( section (".steppingstone") )) void putc_gta03(char c)
{
	serial_putc_s3c64xx(GTA03_DEBUG_UART, c);
}

int sd_card_init_gta03(void)
{
	extern int s3c6410_mmc_init(int verbose);

	return s3c6410_mmc_init(1);
}

int sd_card_block_read_gta03(unsigned char * buf, unsigned long start512,
							       int blocks512)
{
unsigned long s3c6410_mmc_bread(int dev_num, unsigned long blknr, unsigned long blkcnt,
								     void *dst);

	return s3c6410_mmc_bread(0, start512, blocks512, buf);
}

/*
 * our API for bootloader on this machine
 */
const struct board_api board_api_gta03 = {
	.name = "GTA03",
	.linux_machine_id = 1626 /*1866*/,
	.linux_mem_start = 0x50000000,
	.linux_mem_size = (128 * 1024 * 1024),
	.linux_tag_placement = 0x50000000 + 0x100,
	.get_board_variant = get_board_variant_gta03,
	.is_this_board = is_this_board_gta03,
	.port_init = port_init_gta03,
	.putc = putc_gta03,
	.kernel_source = {
		[0] = {
			.name = "SD Card rootfs",
			.block_read = sd_card_block_read_gta03,
			.filesystem = FS_EXT2,
			.partition_index = 2,
			.filepath = "boot/uImage.bin",
			.initramfs_filepath = "boot/initramfs.gz",
			.commandline = "console=ttySAC0,115200 " \
				       "loglevel=8 init=/bin/sh root=/dev/ram ramdisk_size=6000000"
		},
		[1] = {
			.name = "SD Card backup rootfs",
			.block_read = sd_card_block_read_gta03,
			.filesystem = FS_EXT2,
			.partition_index = 3,
			.filepath = "boot/uImage.bin",
			.initramfs_filepath = "boot/initramfs.gz",
			.commandline = "console=ttySAC0,115200 " \
				       "loglevel=8 init=/bin/sh "
		},	},
};

