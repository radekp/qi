/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: xiangfu liu <xiangfu@openmoko.org>
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


#include "blink_led.h"
#include "nand_read.h"
#include "kboot.h"
#include <neo_gta02.h>

/*
unsigned char buf[]={
0x0d,0xc0,0xa0,0xe1,0x00,0xd8,0x2d,0xe9,0x04,0xb0,0x4c,0xe2,0x4c,0x20,0x9f,0xe5,
0x05,0x30,0xa0,0xe3,0x00,0x30,0x82,0xe5,0x44,0x20,0x9f,0xe5,0x44,0x30,0x9f,0xe5,
0x00,0x30,0x82,0xe5,0x40,0x20,0x9f,0xe5,0x3c,0x30,0x9f,0xe5,0x00,0x30,0x93,0xe5,
0x01,0x30,0xc3,0xe3,0x00,0x30,0x82,0xe5,0x28,0x00,0x9f,0xe5,0x0b,0x00,0x00,0xeb,
0x24,0x20,0x9f,0xe5,0x20,0x30,0x9f,0xe5,0x00,0x30,0x93,0xe5,0x01,0x30,0x83,0xe3,
0x00,0x30,0x82,0xe5,0x0c,0x00,0x9f,0xe5,0x04,0x00,0x00,0xeb,0xf0,0xff,0xff,0xea,
0x10,0x00,0x00,0x56,0x18,0x00,0x00,0x56,0xff,0xff,0x00,0x00,0x14,0x00,0x00,0x56,
0x01,0x00,0x50,0xe2,0xfd,0xff,0xff,0x1a,0x0e,0xf0,0xa0,0xe1,0x0a};
*/
#define stringify(x) #x

extern void bootloader_second_phase(void);

void start_kboot(void)
{
	void (*phase2)(void) = bootloader_second_phase + TEXT_BASE;

	port_init();
	serial_init(0x11, UART2);

	puts("Openmoko KBOOT BUILD_HOST BUILD_VERSION BUILD_DATE\n");

#if 0
	while(1) {
		serial_putc(2, '0');
		serial_putc(2, 'x');
		print32((unsigned int)&n);
		serial_putc(2, ' ');
		serial_putc(2, '0');
		serial_putc(2, 'x');
		print32((unsigned int)p);
		serial_putc(2, ' ');
		serial_putc(2, '0');
		serial_putc(2, 'x');
		print32((unsigned int)hello);

		serial_putc(2, '\n');

		blue_on(1);
		n++;
	}

#endif

	/*
	 * pull the whole U-Boot image into SDRAM
	 */

	if (nand_read_ll((unsigned char *)TEXT_BASE, 0, 256 * 1024) < 0)
		while(1)
			blink_led();

		serial_putc(2, '0');
		serial_putc(2, 'x');
		print32((unsigned int)bootloader_second_phase);
		serial_putc(2, ' ');
		serial_putc(2, '\n');


	(phase2)();
}
