/*
 * nand_read.c: Simple NAND read functions for booting from NAND
 *
 * This is used by cpu/arm920/start.S assembler code,
 * and the board-specific linker script must make sure this
 * file is linked within the first 4kB of NAND flash.
 *
 * Taken from GPLv2 licensed vivi bootloader,
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Hwang, Chideok <hwang@mizi.com>
 * Date  : $Date: 2004/02/04 10:37:37 $
 *
 * u-boot integration and bad-block skipping (C) 2006 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 */

/* NOTE this stuff runs in steppingstone context! */

/* the API refers to 512-byte blocks */

#include <qi.h>
#include "nand_read.h"

#define NAND_CMD_READ0          0
#define NAND_CMD_READOOB        0x50

#define __REGb(x)	(*(volatile unsigned char *)(x))
#define __REGw(x)	(*(volatile unsigned short *)(x))
#define __REGi(x)	(*(volatile unsigned int *)(x))
#define NF_BASE		0x4e000000
#define NFCONF          __REGi(NF_BASE + 0x0)
#define NFCMD           __REGb(NF_BASE + 0x4)
#define NFADDR          __REGb(NF_BASE + 0x8)
#define NFDATA          __REGb(NF_BASE + 0xc)
#define NFSTAT          __REGb(NF_BASE + 0x10)
#define NFSTAT_BUSY     1
#define nand_select()   (NFCONF &= ~0x800)
#define nand_deselect() (NFCONF |= 0x800)
#define nand_clear_RnB()        do {} while (0)


static inline void nand_wait(void)
{
	int i;

	while (!(NFSTAT & NFSTAT_BUSY))
		for (i=0; i<10; i++);
}

/* configuration for 2410 with 512byte sized flash */
#define NAND_PAGE_SIZE		512
#define BAD_BLOCK_OFFSET	5
#define	NAND_BLOCK_MASK		(NAND_PAGE_SIZE - 1)
#define NAND_BLOCK_SIZE		0x4000

static int is_bad_block(unsigned long block_index)
{
	unsigned char data;

	nand_clear_RnB();

        NFCMD = NAND_CMD_READOOB; /* 0x50 */
        NFADDR = BAD_BLOCK_OFFSET & 0xf;
        NFADDR = (block_index      ) & 0xff;
        NFADDR = (block_index >> 8 ) & 0xff;
        NFADDR = (block_index >> 16) & 0xff;

	nand_wait();
	data = (NFDATA & 0xff);

	if (data != 0xff)
		return 1;

	return 0;
}

static int nand_read_page_ll(unsigned char *buf, unsigned long block512)
{
	unsigned int i;

	nand_clear_RnB();

	NFCMD = NAND_CMD_READ0;

        /* Write Address */
        NFADDR = 0;
        NFADDR = (block512      ) & 0xff;
        NFADDR = (block512 >> 8 ) & 0xff;
        NFADDR = (block512 >> 16) & 0xff;

	nand_wait();

        for (i = 0; i < NAND_PAGE_SIZE; i++) {
                *buf = (NFDATA & 0xff);
                buf++;
        }

	return 1;
}

/* low level nand read function */
int nand_read_ll(unsigned char *buf, unsigned long start_block512, int blocks512)
{
	int i, j;
	int bad_count = 0;

	/* chip Enable */
	nand_select();
	nand_clear_RnB();

	for (i = 0; i < 10; i++)
		;

	while (blocks512 > 0) {
		if (is_bad_block(start_block512) ||
				is_bad_block(start_block512 + 1)) {
			start_block512 += 1;
			blocks512 += 1;
			if (bad_count++ == 4)
				return -1;
			continue;
		}

		j = nand_read_page_ll(buf, start_block512);
		start_block512 += j;
		buf += j << 9;
		blocks512 -= j;
	}

	/* chip Disable */
	nand_deselect();

	return 0;
}

