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

/*#include <common.h>
#include <linux/mtd/nand.h>
*/
#include "nand_read.h"
#include "blink_led.h"

#define NAND_CMD_READ0 0
#define NAND_CMD_READSTART 0x30

#define __REGb(x)	(*(volatile unsigned char *)(x))
#define __REGw(x)	(*(volatile unsigned short *)(x))
#define __REGi(x)	(*(volatile unsigned int *)(x))
#define NF_BASE		0x4e000000
#define NFCONF		__REGi(NF_BASE + 0x0)
#define NFCONT		__REGi(NF_BASE + 0x4)
#define NFCMD		__REGb(NF_BASE + 0x8)
#define NFADDR		__REGb(NF_BASE + 0xc)
#define NFDATA		__REGb(NF_BASE + 0x10)
#define NFDATA16	__REGw(NF_BASE + 0x10)
#define NFSTAT		__REGb(NF_BASE + 0x20)
#define NFSTAT_BUSY	1
#define nand_select()	(NFCONT &= ~(1 << 1))
#define nand_deselect()	(NFCONT |= (1 << 1))
#define nand_clear_RnB()	(NFSTAT |= (1 << 2))

static inline void nand_wait(void)
{
	int i;

	while (!(NFSTAT & NFSTAT_BUSY))
		for (i=0; i<10; i++);
}

/* configuration for 2440 with 2048byte sized flash */
#define NAND_5_ADDR_CYCLE
#define NAND_PAGE_SIZE		2048
#define BAD_BLOCK_OFFSET	NAND_PAGE_SIZE
#define	NAND_BLOCK_MASK		(NAND_PAGE_SIZE - 1)
#define NAND_BLOCK_SIZE		(NAND_PAGE_SIZE * 64)

static int is_bad_block(unsigned long i)
{
	unsigned char data;
	unsigned long page_num;

	nand_clear_RnB();
	page_num = i >> 11; /* addr / 2048 */
	NFCMD = NAND_CMD_READ0;
	NFADDR = BAD_BLOCK_OFFSET & 0xff;
	NFADDR = (BAD_BLOCK_OFFSET >> 8) & 0xff;
	NFADDR = page_num & 0xff;
	NFADDR = (page_num >> 8) & 0xff;
	NFADDR = (page_num >> 16) & 0xff;
	NFCMD = NAND_CMD_READSTART;
	nand_wait();
	data = (NFDATA & 0xff);
	if (data != 0xff)
		return 1;

	return 0;
}

static int nand_read_page_ll(unsigned char *buf, unsigned long addr)
{
	unsigned short *ptr16 = (unsigned short *)buf;
	unsigned int i, page_num;

	nand_clear_RnB();

	NFCMD = NAND_CMD_READ0;

	page_num = addr >> 11; /* addr / 2048 */
	/* Write Address */
	NFADDR = 0;
	NFADDR = 0;
	NFADDR = page_num & 0xff;
	NFADDR = (page_num >> 8) & 0xff;
	NFADDR = (page_num >> 16) & 0xff;
	NFCMD = NAND_CMD_READSTART;
	nand_wait();

	for (i = 0; i < NAND_PAGE_SIZE/2; i++) {
		*ptr16 = NFDATA16;
		ptr16++;
	}

	return NAND_PAGE_SIZE;
}

/* low level nand read function */
int nand_read_ll(unsigned char *buf, unsigned long start_addr, int size)
{
  int i, j;

  if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK))
    return -1;	/* invalid alignment */

  /* chip Enable */
  nand_select();
  nand_clear_RnB();
  for (i=0; i<10; i++);

  for (i=start_addr; i < (start_addr + size);) {
    if (i % NAND_BLOCK_SIZE == 0) {
      if (is_bad_block(i) ||
	  is_bad_block(i + NAND_PAGE_SIZE)) {
	orange_on(1);    
	i += NAND_BLOCK_SIZE;
	size += NAND_BLOCK_SIZE;
	continue;
      }
    }
    blue_on(1);
    j = nand_read_page_ll(buf, i);
    i += j;
    /*    buf += j;*/
  }

  /* chip Disable */
  nand_deselect();
  
  return 0;
}

