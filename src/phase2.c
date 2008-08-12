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

#include "kboot.h"
#include <neo_gta02.h>
#include "blink_led.h"
#include <string.h>
#define __ARM__
#include <image.h>
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
typedef unsigned int uint32_t;

#include <setup.h>
#include "nand_read.h"

#include <device_configuration.h>

#define C1_DC           (1<<2)          /* dcache off/on */
#define C1_IC           (1<<12)         /* icache off/on */

void bootloader_second_phase(void)
{
	image_header_t	*hdr;
        unsigned long i = 0;
	void	(*the_kernel)(int zero, int arch, uint params);
	struct tag * params_base = (struct tag *)CFG_LINUX_ATAG_ADDRESS;
	struct tag *params = params_base;
	const char * cmdline = CFG_LINUX_CMDLINE;
	const char *p = cmdline;
	void * kernel_nand = (void *)(TEXT_BASE - CFG_LINUX_BIGGEST_KERNEL);

	puts("Checking kernel... ");

	if (nand_read_ll(kernel_nand, CFG_NAND_OFFSET_FOR_KERNEL_PARTITION,
								    4096) < 0) {
		puts ("Kernel header read failed\n");
		goto unhappy;
	}

	hdr = (image_header_t *)kernel_nand;

	if (_ntohl(hdr->ih_magic) != IH_MAGIC) {
		puts("Unknown image magic ");
		print32(hdr->ih_magic);
		goto unhappy;
	}

	puts((const char *)hdr->ih_name);

	puts("Fetching kernel...");

	if (nand_read_ll(kernel_nand, CFG_NAND_OFFSET_FOR_KERNEL_PARTITION,
		((32 * 1024) + (_ntohl(hdr->ih_size) + sizeof(hdr) + 2048)) &
							     ~(2048 - 1)) < 0) {
		puts ("Kernel body read failed\n");
		goto unhappy;
	}

	puts(" Done");

	the_kernel = (void (*)(int, int, uint))
				       (((char *)hdr) + sizeof(image_header_t));

	/* first tag */
	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);
	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;
	params = tag_next (params);

	/* revision tag */
	params->hdr.tag = ATAG_REVISION;
	params->hdr.size = tag_size (tag_revision);
	params->u.revision.rev = CFG_MACHINE_REVISION;
	params = tag_next (params);

	/* memory tags */
	params->hdr.tag = ATAG_MEM;
	params->hdr.size = tag_size (tag_mem32);
	params->u.mem.start = CFG_MEMORY_REGION_START;
	params->u.mem.size = CFG_MEMORY_REGION_SIZE;
	params = tag_next (params);

	/* kernel commandline */
	/* eat leading white space */
	for (p = cmdline; *p == ' '; p++);

	if (*p) {
		params->hdr.tag = ATAG_CMDLINE;
		params->hdr.size =
			 (sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;
		strcpy (params->u.cmdline.cmdline, p);
		params = tag_next (params);
	}

	/* needs to always be the last tag */
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;

	puts ("Running Linux --->\n\n");

	/* trash the cache */

        /* turn off I/D-cache */
        asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));
        i &= ~(C1_DC | C1_IC);
        asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));

        /* flush I/D-cache */
        i = 0;
        asm ("mcr p15, 0, %0, c7, c7, 0": :"r" (i));

	/* ooh that's it, we're gonna try boot this image! */

	the_kernel(0, CFG_LINUX_MACHINE_ID, (unsigned int)params_base);

	/* that didn't quite pan out */

unhappy:
	while (1)
		blue_on(1);
}
