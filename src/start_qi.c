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
#include <neo_gta03.h>

#define stringify2(s) stringify1(s)
#define stringify1(s) #s

extern void bootloader_second_phase(void);

const struct board_api * boards[] = {
	&board_api_gta02,
	&board_api_gta03,
};

struct board_api const * this_board;

void start_qi(void)
{
	int n = 0;
	int board = 0;
	const struct board_variant * board_variant;
	/*
	 * We got the first 4KBytes of the bootloader pulled into the
	 * steppingstone SRAM for free.  Now we pull the whole bootloader
	 * image into SDRAM.
	 *
	 * This code and the .S files are arranged by the linker script to
	 * expect to run from 0x0.  But the linker script has told everything
	 * else to expect to run from 0x33000000+.  That's why we are going to
	 * be able to copy this code and not have it crash when we run it from
	 * there.
	 */

	/* We randomly pull 24KBytes of bootloader */
	if (nand_read_ll((unsigned char *)TEXT_BASE, 0, 24 * 1024) < 0)
		goto unhappy;

	/* ask all the boards we support in turn if they recognize this
	 * hardware we are running on, accept the first positive answer
	 */

	this_board = boards[board];
	while (!n) {
		if (board >= ARRAY_SIZE(boards))
			/* can't put diagnostic on serial... too early */
			goto unhappy;

		if (this_board->is_this_board())
			n = 1;
		else 
			this_board = boards[board++];
	}

	/* okay, do the critical port and serial init for our board */

	this_board->port_init();

	/* stick some hello messages on debug console */

	puts("\n\n\nQi Bootloader  "stringify2(BUILD_HOST)" "
				    stringify2(BUILD_VERSION)" "
				    stringify2(BUILD_DATE)"\n");

	puts("Copyright (C) 2008 Openmoko, Inc.\n");
	puts("This is free software; see the source for copying conditions.\n"
	     "There is NO warranty; not even for MERCHANTABILITY or "
	     "FITNESS FOR A PARTICULAR PURPOSE.\n\n     Detected: ");

	puts(this_board->name);
	puts(", ");
	board_variant = (this_board->get_board_variant)();
	puts(board_variant->name);
	puts("\n");

	/*
	 * jump to bootloader_second_phase() running from DRAM copy
	 */
	bootloader_second_phase();

unhappy:
	while(1)
		blink_led();

}
