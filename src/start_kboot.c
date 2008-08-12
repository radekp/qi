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

#define stringify2(s) stringify1(s)
#define stringify1(s) #s

extern void bootloader_second_phase(void);

void start_kboot(void)
{
	void (*phase2)(void) = (void (*)(void))((int)bootloader_second_phase +
								     TEXT_BASE);

	port_init();
	serial_init(0x11, UART2);

	puts("Openmoko KBOOT "stringify2(BUILD_HOST)" "
			      stringify2(BUILD_VERSION)" "
			      stringify2(BUILD_DATE)"\n");
	/*
	 * pull the whole bootloader image into SDRAM
	 */

	if (nand_read_ll((unsigned char *)TEXT_BASE, 0, 24 * 1024) < 0)
		while(1)
			blink_led();
	/*
	 * jump to bootloader_second_phase() running from DRAM copy
	 */
	(phase2)();
}
