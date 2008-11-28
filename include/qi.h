/*
 * (C) Copyright 2008 Openmoko, Inc.
 * Author: Andy Green <andy@openmoko.org>
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

#ifndef __QI_H__
#define __QI_H__

#include <stdarg.h>
#include <qi-serial.h>
#include <qi-ctype.h>

#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
typedef unsigned int uint32_t;

enum filesystem {
	FS_RAW,
	FS_FAT,
	FS_EXT2
};

/* describes a source for getting kernel image */

struct kernel_source {
	const char *name; /* NULL name means invalid */
	int (*block_init)(void);
	int (*block_read)(unsigned char * buf, unsigned long byte_start,
							       int count_bytes);
	int partition_index; /* -1 means no partition table */
	int offset_if_no_partition; /* used if partition_index is -1 */
	enum filesystem filesystem;
	const char * commandline;
};

/* describes a board variant, eg, PCB revision */

struct board_variant {
	const char * name;
	int machine_revision; /* passed in revision tag to linux */
};

/* describes a "board", ie, a device like GTA02 including revisions */

struct board_api {
	const char * name;
	int debug_serial_port;
	int linux_machine_id;
	unsigned long linux_mem_start;
	unsigned long linux_mem_size;
	unsigned long linux_tag_placement;
	const struct board_variant const * (*get_board_variant)(void);
	int (*is_this_board)(void);
	void (*port_init)(void);
	struct kernel_source kernel_source[8];
};

/* this is the board we are running on */

extern struct board_api const * this_board;

int printk(const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int puts(const char *string);
void printhex(unsigned char v);
void print32(unsigned int u);
void hexdump(unsigned char *start, int len);
unsigned int _ntohl(unsigned int n);
unsigned long crc32(unsigned long crc, const unsigned char *buf,
							      unsigned int len);
int nand_read_ll(unsigned char *buf, unsigned long start_addr, int size);

#endif

