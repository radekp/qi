/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: xiangfu liu <xiangfu@openmoko.org>
 *         Andy Green <andy@openmoko.com>
 *
 * Configuation settings for the OPENMOKO Neo GTA02 Linux GSM phone
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/* NOTE this stuff runs in steppingstone context! */


#include <qi.h>
#include "blink_led.h"
#include "nand_read.h"
#include <neo_gta02.h>

#define stringify2(s) stringify1(s)
#define stringify1(s) #s

extern void bootloader_second_phase(void);

void port_init(void)
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
	 *     Signal : CLKOUT1 CLKOUT0 UCLK nCTS1 nRTS1 RXD1 TXD1 RXD0 TXD0 nRTS0 nCTS0
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
}



void start_kboot(void)
{
	void (*phase2)(void) = (void (*)(void))((int)bootloader_second_phase +
								     TEXT_BASE);

	port_init();
	serial_init(UART2);

	puts("Qi Bootloader  "stringify2(BUILD_HOST)" "
			      stringify2(BUILD_VERSION)" "
			      stringify2(BUILD_DATE));
	puts("Copyright (C) 2008 Openmoko, Inc.");
	puts("This is free software; see the source for copying conditions.\n"
	     "There is NO warranty; not even for MERCHANTABILITY or\n"
	     "FITNESS FOR A PARTICULAR PURPOSE.\n");

	/*
	 * We got the first 4KBytes of the bootloader pulled into the
	 * steppingstone SRAM for free.  Now we pull the whole bootloader
	 * image into SDRAM.
	 *
	 * So this doesn't trash position-dependent code, we took care in the
	 * linker script to arrange all rodata* segment content to be in the
	 * first 4K region.
	 */

	/* We randomly pull 24KBytes of bootloader */
	if (nand_read_ll((unsigned char *)TEXT_BASE, 0, 24 * 1024) < 0)
		while(1)
			blink_led();
	/*
	 * jump to bootloader_second_phase() running from DRAM copy
	 */
	(phase2)();
}
