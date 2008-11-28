/*
 *  linux/drivers/mmc/host/glamo-mmc.c - Glamo MMC driver
 *
 *  Copyright (C) 2007 OpenMoko, Inc,  Andy Green <andy@openmoko.com>
 *  Based on the Glamo MCI driver that was -->
 *
 *  Copyright (C) 2007 OpenMoko, Inc,  Andy Green <andy@openmoko.com>
 *  Based on S3C MMC driver that was:
 *  Copyright (C) 2004-2006 maintech GmbH, Thomas Kleffel <tk@maintech.de>
 *
 *  and
 *
 *  Based on S3C MMC driver that was (original copyright notice ---->)
 *
 * (C) Copyright 2006 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on u-boot pxa MMC driver and linux/drivers/mmc/s3c2410mci.c
 * (C) 2005-2005 Thomas Kleffel
 *
 *  Copyright (C) 2004-2006 maintech GmbH, Thomas Kleffel <tk@maintech.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <qi.h>
#include <mmc.h>

#include <glamo-regs.h>
#include <glamo-mmc.h>

#define CONFIG_GLAMO_BASE 0x08000000

#define MMC_BLOCK_SIZE_BITS 9

#define GLAMO_REG(x) (*(volatile u16 *)(CONFIG_GLAMO_BASE + x))
#define GLAMO_INTRAM_OFFSET (8 * 1024 * 1024)
#define GLAMO_FB_SIZE ((8 * 1024 * 1024) - 0x10000)
#define GLAMO_START_OF_MMC_INTMEM ((volatile u16 *)(CONFIG_GLAMO_BASE + \
				  GLAMO_INTRAM_OFFSET + GLAMO_FB_SIZE))

static int ccnt;
//static mmc_csd_t mmc_csd;
static int mmc_ready = 0;
//static int wide = 0;
static enum card_type card_type = CARDTYPE_NONE;


#define MULTI_READ_BLOCKS_PER_COMMAND 64

int mmc_read(unsigned long src, u8 *dst, int size);

#define UNSTUFF_BITS(resp,start,size)                                   \
        ({                                                              \
                const int __size = size;                                \
                const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1; \
                const int __off = 3 - ((start) / 32);                   \
                const int __shft = (start) & 31;                        \
                u32 __res;                                              \
                                                                        \
                __res = resp[__off] >> __shft;                          \
                if (__size + __shft > 32)                               \
                        __res |= resp[__off-1] << ((32 - __shft) & 31); \
                __res & __mask;                                         \
        })


static void
glamo_reg_write(u16 val, u16 reg)
{
	GLAMO_REG(reg) = val;
}

static u16
glamo_reg_read(u16 reg)
{
	return GLAMO_REG(reg);
}

unsigned char CRC7(u8 * pu8, int cnt)
{
	u8 crc = 0;

	while (cnt--) {
		int n;
		u8 d = *pu8++;
		for (n = 0; n < 8; n++) {
			crc <<= 1;
			if ((d & 0x80) ^ (crc & 0x80))
				crc ^= 0x09;
			d <<= 1;
		}
	}
	return (crc << 1) | 1;
}

unsigned long mmc_bread(int dev_num, unsigned long blknr, unsigned long blkcnt,
								      void *dst)
{
	int ret;

	if (!blkcnt)
		return 0;

/*	printf("mmc_bread(%d, %ld, %ld, %p)\n", dev_num, blknr, blkcnt, dst); */
	ret = mmc_read(blknr, dst, blkcnt);
	if (ret)
		return ret;

	return blkcnt;
}

/* MMC_DEFAULT_RCA should probably be just 1, but this may break other code
   that expects it to be shifted. */
static u16 rca = MMC_DEFAULT_RCA >> 16;

static void do_pio_read(u16 *buf, int count_words)
{
	volatile u16 *from_ptr = GLAMO_START_OF_MMC_INTMEM;

	while (count_words--)
		*buf++ = *from_ptr++;
}

static void do_pio_write(u16 *buf, int count_words)
{
	volatile u16 *to_ptr = GLAMO_START_OF_MMC_INTMEM;

	while (count_words--)
		*to_ptr++ = *buf++;
}


static int mmc_cmd(int opcode, int arg, int flags,
		   int data_size, int data_blocks,
		   int will_stop, u16 *resp)
{
	u16 * pu16 = (u16 *)&resp[0];
	u16 * reg_resp = (u16 *)(CONFIG_GLAMO_BASE + GLAMO_REGOFS_MMC +
				 GLAMO_REG_MMC_CMD_RSP1);
	u16 status;
	int n;
	u8 u8a[6];
	u16 fire = 0;
	int cmd_is_stop = 0;
	int error = 0;

#if 0
	printf("mmc_cmd(opcode=%d, arg=0x%08X, flags=0x%x, "
	       "data_size=%d, data_blocks=%d, will_stop=%d, resp=%p)\n",
	       opcode, arg, flags, data_size, data_blocks, will_stop, resp);
#endif
	switch (opcode) {
	case MMC_STOP_TRANSMISSION:
		cmd_is_stop = 1;
		break;
	default:
		break;
	}

	ccnt++;

	 /* this guy has data to read/write? */
	if ((!cmd_is_stop) && (flags & (MMC_DATA_WRITE | MMC_DATA_READ))) {
		/*
		* the S-Media-internal RAM offset for our MMC buffer
		*/
		glamo_reg_write((u16)GLAMO_FB_SIZE,
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_WDATADS1);
		glamo_reg_write((u16)(GLAMO_FB_SIZE >> 16),
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_WDATADS2);
		glamo_reg_write((u16)GLAMO_FB_SIZE,
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_RDATADS1);
		glamo_reg_write((u16)(GLAMO_FB_SIZE >> 16),
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_RDATADS2);

		/* set up the block info */
		glamo_reg_write(data_size, GLAMO_REGOFS_MMC +
					   GLAMO_REG_MMC_DATBLKLEN);
		glamo_reg_write(data_blocks, GLAMO_REGOFS_MMC +
					     GLAMO_REG_MMC_DATBLKCNT);
	}

	/* if we can't do it, reject as busy */
	if (!glamo_reg_read(GLAMO_REGOFS_MMC + GLAMO_REG_MMC_RB_STAT1) &
	     GLAMO_STAT1_MMC_IDLE)
		return -1;

	/* create an array in wire order for CRC computation */
	u8a[0] = 0x40 | (opcode & 0x3f);
	u8a[1] = (arg >> 24);
	u8a[2] = (arg >> 16);
	u8a[3] = (arg >> 8);
	u8a[4] = arg;
	u8a[5] = CRC7(&u8a[0], 5); /* CRC7 on first 5 bytes of packet */

	/* issue the wire-order array including CRC in register order */
	glamo_reg_write((u8a[4] << 8) | u8a[5],
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_CMD_REG1);
	glamo_reg_write((u8a[2] << 8) | u8a[3],
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_CMD_REG2);
	glamo_reg_write((u8a[0] << 8) | u8a[1],
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_CMD_REG3);

	/* command index toggle */
	fire |= (ccnt & 1) << 12;

	/* set type of command */
	switch (mmc_cmd_type(flags)) {
	case MMC_CMD_BC:
		fire |= GLAMO_FIRE_MMC_CMDT_BNR;
		break;
	case MMC_CMD_BCR:
		fire |= GLAMO_FIRE_MMC_CMDT_BR;
		break;
	case MMC_CMD_AC:
		fire |= GLAMO_FIRE_MMC_CMDT_AND;
		break;
	case MMC_CMD_ADTC:
		fire |= GLAMO_FIRE_MMC_CMDT_AD;
		break;
	}
	/*
	 * if it expects a response, set the type expected
	 *
	 * R1, Length  : 48bit, Normal response
	 * R1b, Length : 48bit, same R1, but added card busy status
	 * R2, Length  : 136bit (really 128 bits with CRC snipped)
	 * R3, Length  : 48bit (OCR register value)
	 * R4, Length  : 48bit, SDIO_OP_CONDITION, Reverse SDIO Card
	 * R5, Length  : 48bit, IO_RW_DIRECTION, Reverse SDIO Card
	 * R6, Length  : 48bit (RCA register)
	 * R7, Length  : 48bit (interface condition, VHS(voltage supplied),
	 *                     check pattern, CRC7)
	 */
	switch (mmc_resp_type(flags)) {
	case MMC_RSP_R6: /* same index as R7 and R1 */
		fire |= GLAMO_FIRE_MMC_RSPT_R1;
		break;
	case MMC_RSP_R1B:
		fire |= GLAMO_FIRE_MMC_RSPT_R1b;
		break;
	case MMC_RSP_R2:
		fire |= GLAMO_FIRE_MMC_RSPT_R2;
		break;
	case MMC_RSP_R3:
		fire |= GLAMO_FIRE_MMC_RSPT_R3;
		break;
	/* R4 and R5 supported by chip not defined in linux/mmc/core.h (sdio) */
	}
	/*
	 * From the command index, set up the command class in the host ctrllr
	 *
	 * missing guys present on chip but couldn't figure out how to use yet:
	 *     0x0 "stream read"
	 *     0x9 "cancel running command"
	 */
	switch (opcode) {
	case MMC_READ_SINGLE_BLOCK:
		fire |= GLAMO_FIRE_MMC_CC_SBR; /* single block read */
		break;
	case MMC_SWITCH: /* 64 byte payload */
	case 0x33: /* observed issued by MCI */
	case MMC_READ_MULTIPLE_BLOCK:
		/* we will get an interrupt off this */
		if (!will_stop)
			/* multiblock no stop */
			fire |= GLAMO_FIRE_MMC_CC_MBRNS;
		else
			 /* multiblock with stop */
			fire |= GLAMO_FIRE_MMC_CC_MBRS;
		break;
	case MMC_WRITE_BLOCK:
		fire |= GLAMO_FIRE_MMC_CC_SBW; /* single block write */
		break;
	case MMC_WRITE_MULTIPLE_BLOCK:
		if (will_stop)
			 /* multiblock with stop */
			fire |= GLAMO_FIRE_MMC_CC_MBWS;
		else
			 /* multiblock NO stop-- 'RESERVED'? */
			fire |= GLAMO_FIRE_MMC_CC_MBWNS;
		break;
	case MMC_STOP_TRANSMISSION:
		fire |= GLAMO_FIRE_MMC_CC_STOP; /* STOP */
		break;
	default:
		fire |= GLAMO_FIRE_MMC_CC_BASIC; /* "basic command" */
		break;
	}
	/* enforce timeout */
	glamo_reg_write(0xfff, GLAMO_REGOFS_MMC + GLAMO_REG_MMC_TIMEOUT);

	/* Generate interrupt on txfer; drive strength max */
	glamo_reg_write((glamo_reg_read(GLAMO_REGOFS_MMC +
					GLAMO_REG_MMC_BASIC) & 0xfe) |
			 0x0800 | GLAMO_BASIC_MMC_NO_CLK_RD_WAIT |
			 GLAMO_BASIC_MMC_EN_COMPL_INT |
			 GLAMO_BASIC_MMC_EN_DR_STR0 |
			 GLAMO_BASIC_MMC_EN_DR_STR1,
			GLAMO_REGOFS_MMC + GLAMO_REG_MMC_BASIC);

	/* send the command out on the wire */
	/* dev_info(&host->pdev->dev, "Using FIRE %04X\n", fire); */
	glamo_reg_write(fire, GLAMO_REGOFS_MMC + GLAMO_REG_MMC_CMD_FIRE);

	/*
	 * we must spin until response is ready or timed out
	 * -- we don't get interrupts unless there is a bulk rx
	 */
	do
		status = glamo_reg_read(GLAMO_REGOFS_MMC +
					GLAMO_REG_MMC_RB_STAT1);
	while ((((status >> 15) & 1) != (ccnt & 1)) ||
		(!(status & (GLAMO_STAT1_MMC_RB_RRDY |
			     GLAMO_STAT1_MMC_RTOUT |
			     GLAMO_STAT1_MMC_DTOUT |
			     GLAMO_STAT1_MMC_BWERR |
			     GLAMO_STAT1_MMC_BRERR))));

	if (status & (GLAMO_STAT1_MMC_RTOUT | GLAMO_STAT1_MMC_DTOUT))
		error = -4;
	if (status & (GLAMO_STAT1_MMC_BWERR | GLAMO_STAT1_MMC_BRERR))
		error = -5;

	if (cmd_is_stop)
		return 0;

	if (error) {
#if 0
		puts("cmd 0x");
		print8(opcode);
		puts(", arg 0x");
		print8(arg);
		puts(", flags 0x");
		print32(flags);
		puts("\n");
		puts("Error after cmd: 0x");
		print32(error);
		puts("\n");
#endif
		goto done;
	}
	/*
	 * mangle the response registers in two different exciting
	 * undocumented ways discovered by trial and error
	 */
	if (mmc_resp_type(flags) == MMC_RSP_R2)
		/* grab the response */
		for (n = 0; n < 8; n++) /* super mangle power 1 */
			pu16[n ^ 6] = reg_resp[n];
	else
		for (n = 0; n < 3; n++) /* super mangle power 2 */
			pu16[n] = (reg_resp[n] >> 8) |
				  (reg_resp[n + 1] << 8);
	/*
	 * if we don't have bulk data to take care of, we're done
	 */
	if (!(flags & (MMC_DATA_READ | MMC_DATA_WRITE)))
		goto done;

	/* enforce timeout */
	glamo_reg_write(0xfff, GLAMO_REGOFS_MMC + GLAMO_REG_MMC_TIMEOUT);
	/*
	 * spin
	 */
	while (!(glamo_reg_read(GLAMO_REG_IRQ_STATUS) & GLAMO_IRQ_MMC))
		;
	/* ack this interrupt source */
	glamo_reg_write(GLAMO_IRQ_MMC, GLAMO_REG_IRQ_CLEAR);

	if (status & GLAMO_STAT1_MMC_DTOUT)
		error = -1;
	if (status & (GLAMO_STAT1_MMC_BWERR | GLAMO_STAT1_MMC_BRERR))
		error = -2;
	if (status & GLAMO_STAT1_MMC_RTOUT)
		error = -5;
	if (error) {
//		printf("cmd 0x%x, arg 0x%x flags 0x%x\n", opcode, arg, flags);
#if 0
		puts("Error after resp: 0x");
		print32(status);
		puts("\n");
#endif
		goto done;
	}
#if 0
	if (flags & MMC_DATA_READ) {
		volatile u8 * pu8 = (volatile u8 *)GLAMO_START_OF_MMC_INTMEM;
		for (n = 0; n < 512; n += 16) {
			int n1;
			for (n1 = 0; n1 < 16; n1++) {
				printf("%02X ", pu8[n + n1]);
			}
			printf("\n");
		}
	}
#endif
	return 0;

done:
	return error;
}

static void glamo_mci_reset(void)
{
	/* reset MMC controller */
	glamo_reg_write(GLAMO_CLOCK_MMC_RESET | GLAMO_CLOCK_MMC_DG_TCLK |
		   GLAMO_CLOCK_MMC_EN_TCLK | GLAMO_CLOCK_MMC_DG_M9CLK |
		   GLAMO_CLOCK_MMC_EN_M9CLK,
		  GLAMO_REG_CLOCK_MMC);
	/* and disable reset */
	glamo_reg_write(GLAMO_CLOCK_MMC_DG_TCLK |
		   GLAMO_CLOCK_MMC_EN_TCLK | GLAMO_CLOCK_MMC_DG_M9CLK |
		   GLAMO_CLOCK_MMC_EN_M9CLK,
		   GLAMO_REG_CLOCK_MMC);
}



int mmc_read(unsigned long src, u8 *dst, int size)
{
	int resp;
	u8 response[16];
	int size_original = size;
	int lump;

	if (((int)dst) & 1) {
		puts("Bad align on dst\n");
		return 0;
	}

	resp = mmc_cmd(MMC_SET_BLOCKLEN, MMC_BLOCK_SIZE,
		       MMC_CMD_AC | MMC_RSP_R1, 0, 0, 0,
		       (u16 *)&response[0]);
	if (resp)
		return resp;

	while (size) {
		/* glamo mmc times out as this increases too much */
		lump = MULTI_READ_BLOCKS_PER_COMMAND;
		if (lump > size)
			lump = size;

		switch (card_type) {
		case CARDTYPE_SDHC: /* block addressing */
			resp = mmc_cmd(MMC_READ_MULTIPLE_BLOCK,
				       src,
				       MMC_CMD_ADTC | MMC_RSP_R1 |
				       MMC_DATA_READ, MMC_BLOCK_SIZE, lump, 1,
				       (u16 *)&response[0]);
			break;
		default: /* byte addressing */
			resp = mmc_cmd(MMC_READ_MULTIPLE_BLOCK, src * MMC_BLOCK_SIZE,
				MMC_CMD_ADTC | MMC_RSP_R1 | MMC_DATA_READ,
				MMC_BLOCK_SIZE, lump, 1,
				(u16 *)&response[0]);
			break;
		}

		if (resp)
			return resp;

		/* final speed 16MHz */
		glamo_reg_write((glamo_reg_read(GLAMO_REG_CLOCK_GEN8) &
					     0xff00) | 2, GLAMO_REG_CLOCK_GEN8);


		do_pio_read((u16 *)dst, lump * MMC_BLOCK_SIZE >> 1);

		if (size)
			size -= lump;

		dst += lump * MMC_BLOCK_SIZE;
		src += lump;

		resp = mmc_cmd(MMC_STOP_TRANSMISSION, 0,
			MMC_CMD_AC | MMC_RSP_R1B, 0, 0, 0,
			(u16 *)&response[0]);
		if (resp)
			return resp;

	}

	return size_original;
}

int mmc_write(u8 *src, unsigned long dst, int size)
{
	int resp;
	u8 response[16];
	int size_original = size;

	if ((!size) || (size & (MMC_BLOCK_SIZE - 1))) {
		puts("Bad size 0x");
		print32(size);
		return 0;
	}

	if (((int)dst) & 1) {
		puts("Bad align on dst\n");
		return 0;
	}

	resp = mmc_cmd(MMC_SET_BLOCKLEN, MMC_BLOCK_SIZE,
		       MMC_CMD_AC | MMC_RSP_R1, 0, 0, 0,
		       (u16 *)&response[0]);

	while (size) {
		do_pio_write((u16 *)src, MMC_BLOCK_SIZE >> 1);
		switch (card_type) {
		case CARDTYPE_SDHC: /* block addressing */
			resp = mmc_cmd(MMC_WRITE_BLOCK,
				       dst >> MMC_BLOCK_SIZE_BITS,
				       MMC_CMD_ADTC | MMC_RSP_R1 |
								MMC_DATA_WRITE,
				       MMC_BLOCK_SIZE, 1, 0,
				       (u16 *)&response[0]);
			break;
		default: /* byte addressing */
			resp = mmc_cmd(MMC_WRITE_BLOCK, dst,
				       MMC_CMD_ADTC | MMC_RSP_R1 |
								MMC_DATA_WRITE,
				       MMC_BLOCK_SIZE, 1, 0,
				       (u16 *)&response[0]);
			break;
		}
		if (size >= MMC_BLOCK_SIZE)
			size -= MMC_BLOCK_SIZE;
		else
			size = 0;
		dst += MMC_BLOCK_SIZE;
		src += MMC_BLOCK_SIZE;
	}
	return size_original;
}

#if 0
static void print_mmc_cid(mmc_cid_t *cid)
{
	puts("MMC found. Card desciption is:\n");
	puts("Manufacturer ID = ");
	print8(cid->id[0]);
	print8(cid->id[1]);
	print8(cid->id[2]);
/*
	puts("HW/FW Revision = %x %x\n",cid->hwrev, cid->fwrev);
	cid->hwrev = cid->fwrev = 0;
	puts("Product Name = %s\n",cid->name);
	printf("Serial Number = %02x%02x%02x\n",
		cid->sn[0], cid->sn[1], cid->sn[2]);
	printf("Month = %d\n",cid->month);
	printf("Year = %d\n",1997 + cid->year);
*/
}
#endif
static void print_sd_cid(const struct sd_cid *cid)
{
	puts("    Card Type: ");
	switch (card_type) {
	case CARDTYPE_NONE:
		puts("(None) / ");
		break;
	case CARDTYPE_MMC:
		puts("MMC / ");
		break;
	case CARDTYPE_SD:
		puts("SD / ");
		break;
	case CARDTYPE_SD20:
		puts("SD 2.0 / ");
		break;
	case CARDTYPE_SDHC:
		puts("SD 2.0 SDHC / ");
		break;
	}

	puts("Mfr: 0x");
	print8(cid->mid);
	puts(", OEM \"");
	this_board->putc(cid->oid_0);
	this_board->putc(cid->oid_1);
	puts("\" / ");

	this_board->putc(cid->pnm_0);
	this_board->putc(cid->pnm_1);
	this_board->putc(cid->pnm_2);
	this_board->putc(cid->pnm_3);
	this_board->putc(cid->pnm_4);
	puts("\", rev ");
	printdec(cid->prv >> 4);
	puts(".");
	printdec(cid->prv & 15);
	puts(" / s/n: ");
	printdec(cid->psn_0 << 24 | cid->psn_1 << 16 | cid->psn_2 << 8 |
	    cid->psn_3);
	puts(" / date: ");
	printdec(cid->mdt_1 & 15);
	puts("/");
	printdec(2000 + ((cid->mdt_0 & 15) << 4)+((cid->mdt_1 & 0xf0) >> 4));
	puts("\n");

/*	printf("CRC:                0x%02x, b0 = %d\n",
	    cid->crc >> 1, cid->crc & 1); */
}


int mmc_init(int verbose)
{
	int retries = 50, rc = -1;
	int resp;
	u8 response[16];
//	mmc_cid_t *mmc_cid = (mmc_cid_t *)response;
	struct sd_cid *sd_cid = (struct sd_cid *)response;
	u32 hcs = 0;

	card_type = CARDTYPE_NONE;

	/* enable engine */

	glamo_reg_write(GLAMO_CLOCK_MMC_EN_M9CLK |
			GLAMO_CLOCK_MMC_EN_TCLK |
			GLAMO_CLOCK_MMC_DG_M9CLK |
			GLAMO_CLOCK_MMC_DG_TCLK, GLAMO_REG_CLOCK_MMC);
	glamo_reg_write(glamo_reg_read(GLAMO_REG_HOSTBUS(2)) |
			GLAMO_HOSTBUS2_MMIO_EN_MMC, GLAMO_REG_HOSTBUS(2));

	/* controller reset */

	glamo_mci_reset();

	/* start the clock -- slowly (50MHz / 250 == 195kHz */

	glamo_reg_write((glamo_reg_read(GLAMO_REG_CLOCK_GEN8) & 0xff00) | 250,
			 GLAMO_REG_CLOCK_GEN8);

	/* enable clock to divider input */

	glamo_reg_write(glamo_reg_read(
		GLAMO_REG_CLOCK_GEN5_1) | GLAMO_CLOCK_GEN51_EN_DIV_TCLK,
		GLAMO_REG_CLOCK_GEN5_1);

	udelay(100000);

	/* set bus width to 1 */

	glamo_reg_write((glamo_reg_read(GLAMO_REGOFS_MMC +
			 GLAMO_REG_MMC_BASIC) &
			 (~GLAMO_BASIC_MMC_EN_4BIT_DATA)),
					GLAMO_REGOFS_MMC + GLAMO_REG_MMC_BASIC);

	/* reset */

	resp = mmc_cmd(MMC_GO_IDLE_STATE, 0, MMC_CMD_BCR, 0, 0, 0,
		       (u16 *)&response[0]);

	udelay(100000);
	udelay(100000);
	udelay(100000);
	udelay(100000);

	/* SDHC card? */

	resp = mmc_cmd(SD_SEND_IF_COND, 0x000001aa,
		MMC_CMD_BCR | MMC_RSP_R7, 0, 0, 0,
		(u16 *)&response[0]);
	if (!resp && (response[0] == 0xaa)) {
		card_type = CARDTYPE_SD20; /* 2.0 SD, may not be SDHC */
		hcs = 0x40000000;
	}

	/* Well, either way let's say hello in SD card protocol */

	while (retries--) {

		udelay(100000);
		udelay(100000);
		udelay(100000);

		resp = mmc_cmd(MMC_APP_CMD, 0x00000000,
			MMC_CMD_AC | MMC_RSP_R1, 0, 0, 0,
			(u16 *)&response[0]);
		if (resp)
			continue;
		resp = mmc_cmd(SD_APP_OP_COND, hcs | 0x00300000,
			MMC_CMD_BCR | MMC_RSP_R3, 0, 0, 0,
			(u16 *)&response[0]);
		if (resp)
			continue;

		if (response[3] & (1 << 6)) { /* asserts block addressing */
			retries = -2;
			card_type = CARDTYPE_SDHC;
		}
		if (response[3] & (1 << 7)) { /* not busy */
			if (card_type == CARDTYPE_NONE)
				card_type = CARDTYPE_SD;
			retries = -2;
			break;
		}
	}
	if (retries == -1) {
		puts("no response\n");
		return 1;
	}

	if (card_type == CARDTYPE_NONE) {
		retries = 10;
		puts("failed to detect SD Card, trying MMC\n");
		do {
			resp = mmc_cmd(MMC_SEND_OP_COND, 0x00ffc000,
				       MMC_CMD_BCR | MMC_RSP_R3, 0, 0, 0,
				       (u16 *)&response[0]);
			udelay(50);
		} while (retries-- && !(response[3] & 0x80));
		if (retries >= 0)
			card_type = CARDTYPE_MMC;
		else
			return 1;
	}

	/* fill in device description */
#if 0
	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = 0;
	mmc_dev.removable = 0;
	mmc_dev.block_read = mmc_bread;
	mmc_dev.blksz = 512;
	mmc_dev.lba = 1 << 16; /* 64K x 512 blocks = 32MB default */
#endif
	/* try to get card id */
	resp = mmc_cmd(MMC_ALL_SEND_CID, hcs,
			MMC_CMD_BCR | MMC_RSP_R2, 0, 0, 0,
			(u16 *)&response[0]);
	if (resp)
		return 1;

	switch (card_type) {
	case CARDTYPE_MMC:
		/* TODO configure mmc driver depending on card
			attributes */
#if 0
		if (verbose)
			print_mmc_cid(mmc_cid);
		sprintf((char *) mmc_dev.vendor,
			"Man %02x%02x%02x Snr %02x%02x%02x",
			mmc_cid->id[0], mmc_cid->id[1], mmc_cid->id[2],
			mmc_cid->sn[0], mmc_cid->sn[1], mmc_cid->sn[2]);
		sprintf((char *) mmc_dev.product, "%s", mmc_cid->name);
		sprintf((char *) mmc_dev.revision, "%x %x",
			mmc_cid->hwrev, mmc_cid->fwrev);
#endif
		/* MMC exists, get CSD too */
		resp = mmc_cmd(MMC_SET_RELATIVE_ADDR, MMC_DEFAULT_RCA,
				MMC_CMD_AC | MMC_RSP_R1, 0, 0, 0,
				(u16 *)&response[0]);
		break;

	case CARDTYPE_SD:
	case CARDTYPE_SD20:
	case CARDTYPE_SDHC:

		if (verbose)
			print_sd_cid(sd_cid);
#if 0
		sprintf((char *) mmc_dev.vendor,
			"Man %02 OEM %c%c \"%c%c%c%c%c\"",
			sd_cid->mid, sd_cid->oid_0, sd_cid->oid_1,
			sd_cid->pnm_0, sd_cid->pnm_1, sd_cid->pnm_2,
			sd_cid->pnm_3, sd_cid->pnm_4);
		sprintf((char *) mmc_dev.product, "%d",
			sd_cid->psn_0 << 24 | sd_cid->psn_1 << 16 |
			sd_cid->psn_2 << 8 | sd_cid->psn_3);
		sprintf((char *) mmc_dev.revision, "%d.%d",
			sd_cid->prv >> 4, sd_cid->prv & 15);
#endif
		resp = mmc_cmd(SD_SEND_RELATIVE_ADDR, MMC_DEFAULT_RCA,
				MMC_CMD_BCR | MMC_RSP_R6, 0, 0, 0,
				(u16 *)&response[0]);
		rca = response[2] | (response[3] << 8);
		break;

	default:
		return 1;
	}

	/* grab the CSD */

	resp = mmc_cmd(MMC_SEND_CSD, rca << 16,
			MMC_CMD_AC | MMC_RSP_R2, 0, 0, 0,
			(u16 *)&response[0]);
	if (!resp) {
		mmc_csd_t *csd = (mmc_csd_t *)response;

//		memcpy(&mmc_csd, csd, sizeof(csd));
		rc = 0;
		mmc_ready = 1;
		/* FIXME add verbose printout for csd */
		/* printf("READ_BL_LEN=%u, C_SIZE_MULT=%u, C_SIZE=%u\n",
			csd->read_bl_len, csd->c_size_mult1,
			csd->c_size); */
//		mmc_dev.blksz = 512;
//		mmc_dev.lba = (((unsigned long)1 << csd->c_size_mult1) *
//				(unsigned long)csd->c_size) >> 9;

		switch (card_type) {
		case CARDTYPE_SDHC:
			puts("    SDHC size: ");
			printdec((UNSTUFF_BITS(((u32 *)&response[0]), 48, 22)
								    + 1) / 2);
			break;
		default:
			puts("  MMC/SD size: ");
			printdec((((unsigned long)1 << csd->c_size_mult1) *
					(unsigned long)(csd->c_size)) >> 10);
		}
		puts(" MiB\n");
	}

	resp = mmc_cmd(MMC_SELECT_CARD, rca<<16, MMC_CMD_AC | MMC_RSP_R1,
		       0, 0, 0, (u16 *)&response[0]);
	if (resp)
		return 1;

#ifdef CONFIG_MMC_WIDE
	/* yay 4-bit! */
	if (card_type == CARDTYPE_SD || card_type == CARDTYPE_SDHC) {
		resp = mmc_cmd(MMC_APP_CMD, rca<<16, MMC_CMD_AC | MMC_RSP_R1,
		       0, 0, 0, (u16 *)&response[0]);
		resp = mmc_cmd(MMC_SWITCH, 0x02, MMC_CMD_AC | MMC_RSP_R1B,
		       0, 0, 0, (u16 *)&response[0]);
		wide = 1;
		glamo_reg_write(glamo_reg_read(GLAMO_REGOFS_MMC +
			 GLAMO_REG_MMC_BASIC) | GLAMO_BASIC_MMC_EN_4BIT_DATA,
					GLAMO_REGOFS_MMC + GLAMO_REG_MMC_BASIC);
	}
#endif

	/* set the clock to slow until first bulk completes (for slow SDHC)  */

	glamo_reg_write((glamo_reg_read(GLAMO_REG_CLOCK_GEN8) & 0xff00) | 32,
			 GLAMO_REG_CLOCK_GEN8);

	return rc;
}



