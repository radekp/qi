#include <qi.h>
#include <neo_gta02.h>
#include "nand_read.h"

static const struct board_variant board_variants[] = {
	[0] = {
		.name = "A5 PCB",
		.machine_revision = 0x350,
	},
	[1] = {
		.name = "A6 PCB",
		.machine_revision = 0x360,
	}
};

void port_init_gta02(void)
{
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
	rGPACON = 0x007E5FFF;
	rGPADAT |= (1 << 16);      /* Set GPA16 to high (nNAND_WP) */
	/*
	 * ===* PORT B GROUP
	 *      Ports  : GPB10    GPB9    GPB8    GPB7    GPB6     GPB5    GPB4   GPB3   GPB2     GPB1      GPB0
	 *      Signal : nXDREQ0 nXDACK0 nXDREQ1 nXDACK1 nSS_KBD nDIS_OFF L3CLOCK L3DATA L3MODE nIrDATXDEN Keyboard
	 *      Setting: INPUT  OUTPUT   INPUT  OUTPUT   INPUT   OUTPUT   OUTPUT OUTPUT OUTPUT   OUTPUT    OUTPUT
	 *      Binary :   00  ,  01       00  ,   01      00   ,  01       01  ,   01     01   ,  01        01
	 */
	rGPBCON = 0x00155555;
	rGPBUP = 0x000007FF;
	/*
	 * === PORT C GROUP
	 *     Ports  : GPC15 GPC14 GPC13 GPC12 GPC11 GPC10 GPC9 GPC8  GPC7   GPC6   GPC5 GPC4 GPC3  GPC2  GPC1 GPC0
	 *     Signal : VD7   VD6   VD5   VD4   VD3   VD2   VD1  VD0 LCDVF2 LCDVF1 LCDVF0 VM VFRAME VLINE VCLK LEND
	 *     Binary :  10   10  , 10    10  , 10    10  , 10   10  , 10     10  ,  10   10 , 10     10 , 10   10
	 */
	rGPCCON = 0x55555155;
	rGPCUP = 0x0000FFFF & ~(1 << 5);
	rGPCDAT |= (1 << 13) | (1 << 15); /* index detect -> hi */
	/*
	 * === PORT D GROUP
	 *     Ports  : GPD15 GPD14 GPD13 GPD12 GPD11 GPD10 GPD9 GPD8 GPD7 GPD6 GPD5 GPD4 GPD3 GPD2 GPD1 GPD0
	 *     Signal : VD23  VD22  VD21  VD20  VD19  VD18  VD17 VD16 VD15 VD14 VD13 VD12 VD11 VD10 VD9  VD8
	 *     Binary : 10    10  , 10    10  , 10    10  , 10   10 , 10   10 , 10   10 , 10   10 ,10   10
	 */
	rGPDCON = 0x55555555;
	rGPDUP = 0x0000FFFF;
	rGPDDAT |= (1 << 0) | (1 << 3) | (1 << 4); /* index detect -> hi */
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
	rGPEUP = 0x0000FFFF & ~(1 << 11);
	/*
	 * === PORT F GROUP
	 *     Ports  : GPF7   GPF6   GPF5   GPF4      GPF3     GPF2  GPF1   GPF0
	 *     Signal : nLED_8 nLED_4 nLED_2 nLED_1 nIRQ_PCMCIA EINT2 KBDINT EINT0
	 *     Setting: Output Output Output Output    EINT3    EINT2 EINT1  EINT0
	 *     Binary :  01      01 ,  01     01  ,     10       10  , 10     10
	 */
	/* pulldown on GPF03: TP-4705+debug - debug conn will float */
	rGPFCON = 0x0000AAAA;
	rGPFUP = 0x000000FF & ~(1 << 3);


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
	rGPGCON = 0x01AAFE79;
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
	rGPHCON = 0x001AAAAA;
	rGPHUP = 0x000007FF & ~(1 << 8) & ~(1 << 0) & ~(1 << 3);

	/* pulldown on GPJ00: input, just floats! */
	/* pulldown on GPJ07: WLAN module WLAN_GPIO0, no ext pull */
	rGPJCON = 0x1551544;
	rGPJUP = 0x1ffff & ~(1 << 0) & ~(1 << 7);
	rGPJDAT |= (1 << 4) | (1 << 6);
					/* Set GPJ4 to high (nGSM_EN) */
					/* Set GPJ6 to high (nDL_GSM) */
	rGPJDAT &= ~(1 << 5);	/* Set GPJ5 to low 3D RST */

	/* leaving Glamo forced to Reset# active here killed
	 * U-Boot when you touched the memory region
	 */

	rGPJDAT |= (1 << 5);	/* Set GPJ5 to high 3D RST */

	serial_init(UART2, 0x11);
}

/**
 * returns PCB revision information in b9,b8 and b2,b1,b0
 * Pre-GTA02 A6 returns 0x000
 *     GTA02 A6 returns 0x001
 */

int gta02_get_pcb_revision(void)
{
	int n;
	u32 u;

	/* make C13 and C15 pulled-down inputs */
	rGPCCON &= ~0xcc000000;
	rGPCUP  &= ~((1 << 13) | (1 << 15));
	/* D0, D3 and D4 pulled-down inputs */
	rGPDCON &= ~0x000003c3;
	rGPDUP  &= ~((1 << 0) | (1 << 3) | (1 << 4));

	/* delay after changing pulldowns */
	u = rGPCDAT;
	u = rGPDDAT;

	/* read the version info */
	u = rGPCDAT;
	n =  (u >> (13 - 0)) & 0x001;
	n |= (u >> (15 - 1)) & 0x002;
	u = rGPDDAT;
	n |= (u << (0 + 2))  & 0x004;

	n |= (u << (8 - 3))  & 0x100;
	n |= (u << (9 - 4))  & 0x200;

	/*
	 * when not being interrogated, all of the revision GPIO
	 * are set to output HIGH without pulldown so no current flows
	 * if they are NC or pulled up.
	 */
	/* make C13 and C15 high ouputs with no pulldowns */
	rGPCCON |= 0x44000000;
	rGPCUP  |= (1 << 13) | (1 << 15);
	rGPCDAT |= (1 << 13) | (1 << 15);
	/* D0, D3 and D4 high ouputs with no pulldowns */
	rGPDCON |= 0x00000141;
	rGPDUP  |= (1 << 0) | (1 << 3) | (1 << 4);
	rGPDDAT |= (1 << 0) | (1 << 3) | (1 << 4);

	return n;
}



/* return nonzero if we believe we run on GTA02 */

int is_this_board_gta02(void)
{
	/* look for GTA02 NOR */

	*(volatile unsigned short *)(0x18000000) = 0x98;

	return !!(*(volatile unsigned short *)(0x18000000) == 0x0020);
}

const struct board_variant const * get_board_variant_gta02(void)
{
	return &board_variants[gta02_get_pcb_revision()];
}

/*
 * our API for bootloader on this machine
 */

const struct board_api board_api_gta02 = {
	.name = "Freerunner / GTA02",
	.debug_serial_port = 2,
	.linux_machine_id = 1304,
	.linux_mem_start = 0x30000000,
	.linux_mem_size = (128 * 1024 * 1024),
	.linux_tag_placement = 0x30000000 + 0x100,
	.get_board_variant = get_board_variant_gta02,
	.is_this_board = is_this_board_gta02,
	.port_init = port_init_gta02,
	/* these are the ways we could boot GTA02 in order to try */
	.kernel_source = {
		[0] = {
			.name = "NAND Kernel",
			.block_read = nand_read_ll,
			.partition_index = -1,
			.offset_if_no_partition = 0x80000,
			.filesystem = FS_RAW,
			.commandline = "mtdparts=physmap-flash:-(nor);" \
					"neo1973-nand:" \
					 "0x00040000(qi)," \
					 "0x00040000(cmdline)," \
					 "0x00800000(backupkernel)," \
					 "0x000a0000(extra)," \
					 "0x00040000(identity)," \
					 "0x0f6a0000(backuprootfs) " \
				       "rootfstype=jffs2 " \
				       "root=/dev/mtdblock6 " \
				       "console=ttySAC2,115200 " \
				       "loglevel=4 " \
				       "init=/sbin/init "\
				       "ro"
		},
	},
};
