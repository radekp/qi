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
#include <ext2.h>

unsigned long partition_offset_blocks = 0;
unsigned long partition_length_blocks = 0;

struct kernel_source const * this_kernel = 0;

const int INITRD_OFFSET = (8 * 1024 * 1024);

int raise(int n)
{
	return 0;
}

int read_file(const char * filepath, u8 * destination, int size)
{
	int len = size;

	switch (this_kernel->filesystem) {
	case FS_EXT2:
		if (!ext2fs_mount()) {
			puts("Unable to mount ext2 filesystem\n");
			return -1;
		}
		puts("    EXT2 open: ");
		puts(filepath);
		len = ext2fs_open(filepath);
		if (len < 0) {
			puts(" Open failed\n");
			return -1;
		}
		puts(" OK\n");
		ext2fs_read((char *)destination, size);
		break;

	case FS_FAT:
		/* FIXME */
	case FS_RAW:
		/* any filename-related request in raw filesystem will fail */
		if (filepath)
			return -1;
		puts("     RAW open: +");
		printdec(partition_offset_blocks);
		puts(" 512-byte blocks\n");
		if (this_kernel->block_read(destination,
				      partition_offset_blocks, size >> 9) < 0) {
			puts ("Bad kernel header\n");
			return -1;
		}
		break;
	}

	return len;
}

void bootloader_second_phase(void)
{
	void	(*the_kernel)(int zero, int arch, uint params);
	int kernel = 0;
	const struct board_variant * board_variant =
					      (this_board->get_board_variant)();
	unsigned int initramfs_len = 0;
	static char commandline_rootfs_append[512] = "";

	/* we try the possible kernels for this board in order */

	this_kernel = &this_board->kernel_source[kernel++];

	while (this_kernel->name) {
		const char *p;
		char * cmdline;
		struct tag *params = (struct tag *)this_board->linux_tag_placement;
		void * kernel_dram = (void *)this_board->linux_mem_start + 0x8000;
		unsigned long crc;
		image_header_t	*hdr;
		u32 kernel_size;

		partition_offset_blocks = 0;
		partition_length_blocks = 0;

		/* eat leading white space */
		for (p = this_board->commandline_board; *p == ' '; p++);

		puts("\nTrying kernel: ");
		puts(this_kernel->name);
		puts("\n");

		/* if this device needs initializing, try to init it */
		if (this_kernel->block_init)
			if ((this_kernel->block_init)()) {
				puts("block device init failed\n");
				this_kernel = &this_board->
							kernel_source[kernel++];
				continue;
			}

		/* if there's a partition table implied, parse it, otherwise
		 * just use a fixed offset
		 */
		if (this_kernel->partition_index) {
			unsigned char *p = kernel_dram;

			if ((int)this_kernel->block_read(kernel_dram, 0, 4)
									  < 0) {
				puts("Bad partition read\n");
				this_kernel = &this_board->
							kernel_source[kernel++];
				continue;
			}

			if ((p[0x1fe] != 0x55) || (p[0x1ff] != 0xaa)) {
				puts("partition signature missing\n");
				this_kernel = &this_board->
							kernel_source[kernel++];
				continue;
			}

			p += 0x1be + 8 + (0x10 *
					    (this_kernel->partition_index - 1));

			partition_offset_blocks = (((u32)p[3]) << 24) |
						  (((u32)p[2]) << 16) |
						  (((u32)p[1]) << 8) |
						  p[0];
			partition_length_blocks = (((u32)p[7]) << 24) |
						  (((u32)p[6]) << 16) |
						  (((u32)p[5]) << 8) |
						  p[4];

			puts("    Partition: ");
			printdec(this_kernel->partition_index);
			puts(" start +");
			printdec(partition_offset_blocks);
			puts(" 512-byte blocks, size ");
			printdec(partition_length_blocks / 2048);
			puts(" MiB\n");

		} else
			partition_offset_blocks =
				  this_kernel->offset_blocks512_if_no_partition;

		/* does he want us to skip this? */

		if (read_file(this_board->noboot, kernel_dram, 512) >= 0) {
			puts("    (Skipping on finding ");
			puts(this_board->noboot);
			puts(")\n");
			this_kernel = &this_board->kernel_source[kernel++];
			continue;
		}

		/* is there a commandline append file? */

		commandline_rootfs_append[0] = '\0';
		read_file(this_board->append, (u8 *)commandline_rootfs_append,
									   512);

		/* pull the kernel image */

		if (read_file(this_kernel->filepath, kernel_dram, 4096) < 0) {
			this_kernel = &this_board->kernel_source[kernel++];
			continue;
		}

		hdr = (image_header_t *)kernel_dram;

		if (__be32_to_cpu(hdr->ih_magic) != IH_MAGIC) {
			puts("bad magic ");
			print32(hdr->ih_magic);
			puts("\n");
			this_kernel = &this_board->kernel_source[kernel++];
			continue;
		}

		puts("        Found: \"");
		puts((const char *)hdr->ih_name);
		puts("\"\n         Size: ");
		printdec(__be32_to_cpu(hdr->ih_size) >> 10);
		puts(" KiB\n");

		kernel_size = ((__be32_to_cpu(hdr->ih_size) +
				  sizeof(image_header_t) + 2048) & ~(2048 - 1));

		if (read_file(this_kernel->filepath, kernel_dram,
							     kernel_size) < 0) {
			this_kernel = &this_board->kernel_source[kernel++];
			continue;
		}

		/* initramfs if needed */

		if (this_kernel->initramfs_filepath) {
			initramfs_len = read_file(this_kernel->initramfs_filepath,
			    (u8 *)this_board->linux_mem_start + INITRD_OFFSET, 16 * 1024 * 1024);
			if (initramfs_len < 0) {
				puts("initramfs load failed\n");
				this_kernel = &this_board->kernel_source[kernel++];
				continue;
			}
		}

		/*
		 * It's good for now to know that our kernel is intact from
		 * the storage before we jump into it and maybe crash silently
		 * even though it costs us some time
		 */
		crc = crc32(0, kernel_dram + sizeof(image_header_t),
						   __be32_to_cpu(hdr->ih_size));
		if (crc != __be32_to_cpu(hdr->ih_dcrc)) {
			puts("\nKernel CRC ERROR: read 0x");
			print32(crc);
			puts(" vs hdr CRC 0x");
			print32(__be32_to_cpu(hdr->ih_dcrc));
			puts("\n");
			this_kernel = &this_board->kernel_source[kernel++];
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

		if (this_kernel->initramfs_filepath) {
			/* INITRD2 tag */
			params->hdr.tag = ATAG_INITRD2;
			params->hdr.size = tag_size (tag_initrd);
			params->u.initrd.start = this_board->linux_mem_start +
							      INITRD_OFFSET;
			params->u.initrd.size = initramfs_len;
			params = tag_next(params);
		}

		/* kernel commandline */

		if (*p) {
			cmdline = params->u.cmdline.cmdline;
			cmdline += strlen(strcpy(cmdline, p));
			if (this_kernel->commandline_append)
				cmdline += strlen(strcpy(cmdline,
					      this_kernel->commandline_append));
			if (commandline_rootfs_append[0])
				cmdline += strlen(strcpy(cmdline,
					      commandline_rootfs_append));

			params->hdr.tag = ATAG_CMDLINE;
			params->hdr.size = (sizeof (struct tag_header) +
				strlen(params->u.cmdline.cmdline) + 1 + 4) >> 2;

			puts("      Cmdline: ");
			puts(params->u.cmdline.cmdline);
			puts("\n");

			params = tag_next (params);
		}

		/* needs to always be the last tag */
		params->hdr.tag = ATAG_NONE;
		params->hdr.size = 0;

		/* give board implementation a chance to shut down
		 * anything it may have going on, leave GPIO set for Linux
		 */
		if (this_board->close)
			(this_board->close)();

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

	puts("\nNo usable kernel image found\n");

	/*
	 * sit there doing a memory test in this case.
	 *
	 * This phase 2 code will get destroyed but it's OK, we won't be
	 * coming back and the whole memory test and dependency functions are
	 * in phase 1 / steppingstone, so we can test entire memory range.
	 *
	 * It means we just boot with SD Card with kernel(s) renamed or removed
	 * to provoke memory test.
	 */

	memory_test((void *)this_board->linux_mem_start,
						    this_board->linux_mem_size);

}
