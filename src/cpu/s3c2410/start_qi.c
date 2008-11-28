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
#include "nand_read.h"
#include <neo_gta01.h>

#define stringify2(s) stringify1(s)
#define stringify1(s) #s


extern void bootloader_second_phase(void);

const struct board_api *boards[] = {
			&board_api_gta01,
			NULL /* always last */
};


struct board_api const * this_board;
extern int is_jtag;

void start_qi(void)
{
	int flag = 0;
	int board = 0;

	/*
	 * well, we can be running on this CPU two different ways.
	 *
	 * 1) We were copied into steppingstone and TEXT_BASE already
	 *    by JTAG.  We don't have to do anything else.  JTAG script
	 *    then sets data at address 0x4 to 0xffffffff as a signal we
	 *    are running by JTAG.
	 *
	 * 2) We only got our first 4K into steppingstone, we need to copy
	 *    the rest of ourselves into TEXT_BASE.
	 *
	 * So we do the copy out of NAND only if we see we did not come up
	 * under control of JTAG.
	 */

	if (!is_jtag)
		/*
		* We got the first 4KBytes of the bootloader pulled into the
		* steppingstone SRAM for free.  Now we pull the whole bootloader
		* image into SDRAM.
		*
		* This code and the .S files are arranged by the linker script
		* to expect to run from 0x0.  But the linker script has told
		* everything else to expect to run from 0x33000000+.  That's
		* why we are going to be able to copy this code and not have it
		* crash when we run it from there.
		*/

		/* We randomly pull 32KBytes of bootloader */
		if (nand_read_ll((u8 *)TEXT_BASE, 0, 32 * 1024 / 512) < 0)
			goto unhappy;

	/* ask all the boards we support in turn if they recognize this
	 * hardware we are running on, accept the first positive answer
	 */

	this_board = boards[board];
	while (!flag && this_board) {

		/* check if it is the right board... */
		if (this_board->is_this_board()) {
			flag = 1;
			continue;
		}

		this_board = boards[board++];
	}

	/* No valid board found */
	if (!this_board)
		goto unhappy;

	this_board->port_init();
	set_putc_func(this_board->putc);

	/* stick some hello messages on debug console */

	puts("\n\n\nQi Bootloader "stringify2(QI_CPU)"  "
				   stringify2(BUILD_HOST)" "
				   stringify2(BUILD_VERSION)" "
				   "\n");

	puts(stringify2(BUILD_DATE) "  Copyright (C) 2008 Openmoko, Inc.\n");
	puts("\n     Detected: ");

	puts(this_board->name);
	puts(", ");
	puts((this_board->get_board_variant)()->name);
	puts("\n");

	
	/*
	 * jump to bootloader_second_phase() running from DRAM copy
	 */
	bootloader_second_phase();

unhappy:
	while(1)
		;

}
