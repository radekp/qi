/*
 * (C) Copyright 2008 Openmoko, Inc.
 * Author: Andy Green <andy@openmoko.org>
 *
 * Parse the U-Boot header and Boot Linux
 * based on various code from U-Boot
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

#include <qi.h>
#include <neo_gta02.h>
#include "blink_led.h"
#include <string.h>
#define __ARM__
#include <image.h>

#include <setup.h>
#include "nand_read.h"


void bootloader_second_phase(void)
{
	void	(*the_kernel)(int zero, int arch, uint params);
	int kernel = 0;
	const struct board_variant * board_variant =
					      (this_board->get_board_variant)();

	/* we try the possible kernels for this board in order */

	while (this_board->kernel_source[kernel].name) {
		const char * cmdline = this_board->kernel_source[kernel].commandline;
		const char *p = cmdline;
		struct tag *params = (struct tag *)this_board->linux_tag_placement;
		void * kernel_dram = (void *)(TEXT_BASE - (8 * 1024 * 1024));
		unsigned long crc;
		image_header_t	*hdr;

		/* eat leading white space */
		for (p = cmdline; *p == ' '; p++);

		puts("\nTrying kernel: ");
		puts(this_board->kernel_source[kernel].name);
		puts("\n");

		/* if this device needs initializing, try to init it */
		if (this_board->kernel_source[kernel].block_init)
			if ((this_board->kernel_source[kernel].block_init)()) {
				puts("block device init failed\n");
				kernel++;
				continue;
			}

		/* if there's a partition table implied, parse it, otherwise
		 * just use a fixed offset
		 */
		if (this_board->kernel_source[kernel].partition_index != -1) {

			puts("partitions not supported yet\n");
			kernel++;
			continue;

		} else {
			if (this_board->kernel_source[kernel].block_read(
				kernel_dram, this_board->kernel_source[kernel].
					    offset_if_no_partition, 4096) < 0) {
				puts ("Bad kernel header\n");
				kernel++;
				continue;
			}

			hdr = (image_header_t *)kernel_dram;

			if (_ntohl(hdr->ih_magic) != IH_MAGIC) {
				puts("bad magic ");
				print32(hdr->ih_magic);
				kernel++;
				continue;
			}

			puts("        Found: ");
			puts((const char *)hdr->ih_name);
			puts("\n         Size: ");
			printdec(_ntohl(hdr->ih_size) >> 10);
			puts(" KiB\n");

			if (nand_read_ll(kernel_dram,
				this_board->kernel_source[kernel].
				offset_if_no_partition, (_ntohl(hdr->ih_size) +
						sizeof(image_header_t) + 2048) &
							     ~(2048 - 1)) < 0) {
				puts ("Bad kernel read\n");
				kernel++;
				continue;
			}
		}

		puts("      Cmdline: ");
		puts(p);
		puts("\n");

		/*
		 * It's good for now to know that our kernel is intact from
		 * the storage before we jump into it and maybe crash silently
		 * even though it costs us some time
		 */
		crc = crc32(0, kernel_dram + sizeof(image_header_t),
							_ntohl(hdr->ih_size));
		if (crc != _ntohl(hdr->ih_dcrc)) {
			puts("\nKernel CRC ERROR: read 0x");
			print32(crc);
			puts(" vs hdr CRC 0x");
			print32(_ntohl(hdr->ih_dcrc));
			puts("\n");
			kernel++;
			continue;
		}

		the_kernel = (void (*)(int, int, uint))
					(((char *)hdr) + sizeof(image_header_t));

		/* first tag */
		params->hdr.tag = ATAG_CORE;
		params->hdr.size = tag_size (tag_core);
		params->u.core.flags = 0;
		params->u.core.pagesize = 0;
		params->u.core.rootdev = 0;
		params = tag_next(params);

		/* revision tag */
		params->hdr.tag = ATAG_REVISION;
		params->hdr.size = tag_size (tag_revision);
		params->u.revision.rev = board_variant->machine_revision;
		params = tag_next(params);

		/* memory tags */
		params->hdr.tag = ATAG_MEM;
		params->hdr.size = tag_size (tag_mem32);
		params->u.mem.start = this_board->linux_mem_start;
		params->u.mem.size = this_board->linux_mem_size;
		params = tag_next(params);

		/* kernel commandline */

		if (*p) {
			params->hdr.tag = ATAG_CMDLINE;
			params->hdr.size = (sizeof (struct tag_header) +
						       strlen (p) + 1 + 4) >> 2;
			strcpy (params->u.cmdline.cmdline, p);
			params = tag_next (params);
		}

		/* needs to always be the last tag */
		params->hdr.tag = ATAG_NONE;
		params->hdr.size = 0;

		puts ("Starting --->\n\n");

		/*
		* ooh that's it, we're gonna try boot this image!
		* never mind the cache, Linux will take care of it
		*/
		the_kernel(0, this_board->linux_machine_id,
						this_board->linux_tag_placement);

		/* we won't come back here no matter what */
	}

	/* none of the kernels worked out */

	puts("No usable kernel image found, we've had it  :-(\n");
	while (1)
		blue_on(1);
}
