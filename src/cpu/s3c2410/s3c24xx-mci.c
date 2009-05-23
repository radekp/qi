/*
 * qi s3c24xx SD card driver
 * Author: Andy Green <andy@openmoko.com>
 * based on ---->
 *
 * u-boot S3C2410 MMC/SD card driver
 * (C) Copyright 2006 by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on u-boot pxa MMC driver and linux/drivers/mmc/s3c2410mci.c
 * (C) 2005-2005 Thomas Kleffel
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
#include <mmc.h>
#include <s3c24xx-regs-sdi.h>
#include <string.h>

#define SDICON (*(u32 *)0x5a000000)
#define SDIPRE (*(u32 *)0x5a000004)
#define SDICARG (*(u32 *)0x5a000008)
#define SDICCON (*(u32 *)0x5a00000c)
#define SDICSTA (*(u32 *)0x5a000010)
#define SDIRSP0 (*(u32 *)0x5a000014)
#define SDIRSP1 (*(u32 *)0x5a000018)
#define SDIRSP2 (*(u32 *)0x5a00001c)
#define SDIRSP3 (*(u32 *)0x5a000020)
#define SDIDTIMER (*(u32 *)0x5a000024)
#define SDIBSIZE (*(u32 *)0x5a000028)
#define SDIDCON (*(u32 *)0x5a00002c)
#define SDIDCNT (*(u32 *)0x5a000030)
#define SDIDSTA (*(u32 *)0x5a000034)
#define SDIFSTA (*(u32 *)0x5a000038)
/* s3c2410 in GTA01 has these two last ones the other way around!!! */
#define SDIIMSK (*(u32 *)0x5a00003c)
#define SDIDAT (*(u32 *)0x5a000040)
#define SDIDAT2410 (*(u32 *)0x5a00003c)
#define SDIIMSK2410 (*(u32 *)0x5a000040)

#define CFG_MMC_BASE		0xff000000

int am_i_s3c2410(void)
{
	return 1;
}

#define CONFIG_MMC_WIDE
#define MMC_BLOCK_SIZE 512

/*
 * FIXME needs to read cid and csd info to determine block size
 * and other parameters
 */
static u8 mmc_buf[MMC_BLOCK_SIZE];
static mmc_csd_t mmc_csd;
static int mmc_ready = 0;
static int wide = 0;
static int is_sdhc = 0;


#define CMD_F_RESP	0x01
#define CMD_F_RESP_LONG	0x02

static u32 *mmc_cmd(u16 cmd, u32 arg, u16 flags)
{
	static u32 resp[5];

	u32 ccon, csta;
	u32 csta_rdy_bit = S3C2410_SDICMDSTAT_CMDSENT;

	memset(resp, 0, sizeof(resp));

//	debug("mmc_cmd CMD%d arg=0x%08x flags=%x\n", cmd, arg, flags);

	SDICSTA = 0xffffffff;
	SDIDSTA = 0xffffffff;
	SDIFSTA = 0xffffffff;

	SDICARG = arg;

	ccon = cmd & S3C2410_SDICMDCON_INDEX;
	ccon |= S3C2410_SDICMDCON_SENDERHOST|S3C2410_SDICMDCON_CMDSTART;

	if (flags & CMD_F_RESP) {
		ccon |= S3C2410_SDICMDCON_WAITRSP;
		csta_rdy_bit = S3C2410_SDICMDSTAT_RSPFIN; /* 1 << 9 */
	}

	if (flags & CMD_F_RESP_LONG)
		ccon |= S3C2410_SDICMDCON_LONGRSP;

	SDICCON = ccon;

	while (1) {
		csta = SDICSTA;
		if (csta & csta_rdy_bit)
			break;
		if (csta & S3C2410_SDICMDSTAT_CMDTIMEOUT) {
			puts("===============> MMC CMD Timeout\n");
			SDICSTA |= S3C2410_SDICMDSTAT_CMDTIMEOUT;
			break;
		}
	}

//	debug("final MMC CMD status 0x%x\n", csta);

	SDICSTA |= csta_rdy_bit;

	if (flags & CMD_F_RESP) {
		resp[0] = SDIRSP0;
		resp[1] = SDIRSP1;
		resp[2] = SDIRSP2;
		resp[3] = SDIRSP3;
	}

	return resp;
}

#define FIFO_FILL() ((SDIFSTA & S3C2410_SDIFSTA_COUNTMASK) >> 2)

static int mmc_block_read(u8 *dst, u32 src, u32 len)
{
	u32 dcon, fifo;
	u32 *dst_u32 = (u32 *)dst;
	u32 *resp;

	if (len == 0)
		return 0;

//	debug("mmc_block_rd dst %lx src %lx len %d\n", (u32)dst, src, len);

	/* set block len */
	resp = mmc_cmd(MMC_CMD_SET_BLOCKLEN, len, CMD_F_RESP);
	SDIBSIZE = len;

	//SDIPRE = 0xff;

	/* setup data */
	dcon = (len >> 9) & S3C2410_SDIDCON_BLKNUM;
	dcon |= S3C2410_SDIDCON_BLOCKMODE;
	dcon |= S3C2410_SDIDCON_RXAFTERCMD|S3C2410_SDIDCON_XFER_RXSTART;
	if (wide)
		dcon |= S3C2410_SDIDCON_WIDEBUS;

	if (!am_i_s3c2410())
		dcon |= S3C2440_SDIDCON_DS_WORD | S3C2440_SDIDCON_DATSTART;

	SDIDCON = dcon;

	/* send read command */
	if (!is_sdhc)
		resp = mmc_cmd(MMC_CMD_READ_BLOCK, src, CMD_F_RESP);
	else
		resp = mmc_cmd(MMC_CMD_READ_BLOCK, src / MMC_BLOCK_SIZE, CMD_F_RESP);

	while (len > 0) {
		u32 sdidsta = SDIDSTA;
		fifo = FIFO_FILL();
		if (sdidsta & (S3C2410_SDIDSTA_FIFOFAIL|
				S3C2410_SDIDSTA_CRCFAIL|
				S3C2410_SDIDSTA_RXCRCFAIL|
				S3C2410_SDIDSTA_DATATIMEOUT)) {
			puts("mmc_block_read: err SDIDSTA=0x");
			print32(sdidsta);
			puts("\n");
			return -1;
		}

		if (am_i_s3c2410()) {
			while (fifo--) {
				//debug("dst_u32 = 0x%08x\n", dst_u32);
				*(dst_u32++) = SDIDAT2410;
				if (len >= 4)
					len -= 4;
				else {
					len = 0;
					break;
				}
			}
		} else {
			while (fifo--) {
				//debug("dst_u32 = 0x%08x\n", dst_u32);
				*(dst_u32++) = SDIDAT;
				if (len >= 4)
					len -= 4;
				else {
					len = 0;
					break;
				}
			}
		}
	}

//	debug("waiting for SDIDSTA  (currently 0x%08x\n", SDIDSTA);
	while (!(SDIDSTA & (1 << 4))) {}
//	debug("done waiting for SDIDSTA (currently 0x%08x\n", SDIDSTA);

	SDIDCON = 0;

	if (!(SDIDSTA & S3C2410_SDIDSTA_XFERFINISH))
		puts("mmc_block_read; transfer not finished!\n");

	return 0;
}

static int mmc_block_write(u32 dst, u8 *src, int len)
{
	puts("MMC block write not yet supported on S3C2410!\n");
	return -1;
}


int  s3c24xx_mmc_read(u32 src, u8 *dst, int size)
{
	u32 end, part_start, part_end, part_len, aligned_start, aligned_end;
	u32 mmc_block_size, mmc_block_address;

	if (size == 0)
		return 0;

	if (!mmc_ready) {
		puts("Please initialize the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	src -= CFG_MMC_BASE;
	end = src + size;
	part_start = ~mmc_block_address & src;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & src;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
//	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//	src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
//		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//		src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0)
			return -1;

		memcpy(dst, mmc_buf+part_start, part_len);
		dst += part_len;
		src += part_len;
	}
//	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//	src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; src < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
//		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//		src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read((u8 *)(dst), src, mmc_block_size)) < 0)
			return -1;
	}
//	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//	src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && src < end) {
//		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//		src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0)
			return -1;

		memcpy(dst, mmc_buf, part_end);
	}
	return 0;
}

int s3c24xx_mmc_write(u8 *src, u32 dst, int size)
{
	u32 end, part_start, part_end, part_len, aligned_start, aligned_end;
	u32 mmc_block_size, mmc_block_address;

	if (size == 0)
		return 0;

	if (!mmc_ready) {
		puts("Please initialize the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;
	mmc_block_address = ~(mmc_block_size - 1);

	dst -= CFG_MMC_BASE;
	end = dst + size;
	part_start = ~mmc_block_address & dst;
	part_end = ~mmc_block_address & end;
	aligned_start = mmc_block_address & dst;
	aligned_end = mmc_block_address & end;

	/* all block aligned accesses */
//	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//	src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_start) {
		part_len = mmc_block_size - part_start;
//		debug("ps src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//		(u32)src, dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_start, mmc_block_size)) < 0)
			return -1;

		memcpy(mmc_buf+part_start, src, part_len);
		if ((mmc_block_write(aligned_start, mmc_buf, mmc_block_size)) < 0)
			return -1;

		dst += part_len;
		src += part_len;
	}
//	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//	src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
	for (; dst < aligned_end; src += mmc_block_size, dst += mmc_block_size) {
//		debug("al src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//		src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_write(dst, (u8 *)src, mmc_block_size)) < 0)
			return -1;

	}
//	debug("src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//	src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
	if (part_end && dst < end) {
//		debug("pe src %lx dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
//		src, (u32)dst, end, part_start, part_end, aligned_start, aligned_end);
		if ((mmc_block_read(mmc_buf, aligned_end, mmc_block_size)) < 0)
			return -1;

		memcpy(mmc_buf, src, part_end);
		if ((mmc_block_write(aligned_end, mmc_buf, mmc_block_size)) < 0)
			return -1;

	}
	return 0;
}

u32 s3c24xx_mmc_bread(int dev_num, u32 blknr, u32 blkcnt, void *dst)
{
	int mmc_block_size = MMC_BLOCK_SIZE;
	u32 src = blknr * mmc_block_size + CFG_MMC_BASE;

	s3c24xx_mmc_read(src, dst, blkcnt*mmc_block_size);
	return blkcnt;
}

/* MMC_DEFAULT_RCA should probably be just 1, but this may break other code
   that expects it to be shifted. */
static u_int16_t rca = MMC_DEFAULT_RCA >> 16;

#if 0
static u32 mmc_size(const struct mmc_csd *csd)
{
	u32 block_len, mult, blocknr;

	block_len = csd->read_bl_len << 12;
	mult = csd->c_size_mult1 << 8;
	blocknr = (csd->c_size+1) * mult;

	return blocknr * block_len;
}
#endif

struct sd_cid {
	char		pnm_0;	/* product name */
	char		oid_1;	/* OEM/application ID */
	char		oid_0;
	uint8_t		mid;	/* manufacturer ID */
	char		pnm_4;
	char		pnm_3;
	char		pnm_2;
	char		pnm_1;
	uint8_t		psn_2;	/* product serial number */
	uint8_t		psn_1;
	uint8_t		psn_0;	/* MSB */
	uint8_t		prv;	/* product revision */
	uint8_t		crc;	/* CRC7 checksum, b0 is unused and set to 1 */
	uint8_t		mdt_1;	/* manufacturing date, LSB, RRRRyyyy yyyymmmm */
	uint8_t		mdt_0;	/* MSB */
	uint8_t		psn_3;	/* LSB */
};

static void print_mmc_cid(mmc_cid_t *cid)
{
	puts("MMC found. Card desciption is:\n");
	puts("Manufacturer ID = ");
	print8(cid->id[0]);
	print8(cid->id[1]);
	print8(cid->id[2]);
	puts("\nHW/FW Revision = ");
	print8(cid->hwrev);
	print8(cid->fwrev);
	cid->hwrev = cid->fwrev = 0;	/* null terminate string */
	puts("Product Name = ");
	puts((char *)cid->name);
	puts("\nSerial Number = ");
	print8(cid->sn[0]);
	print8(cid->sn[1]);
	print8(cid->sn[2]);
	puts("\nMonth = ");
	printdec(cid->month);
	puts("\nYear = ");
	printdec(1997 + cid->year);
	puts("\n");
}

static void print_sd_cid(const struct sd_cid *cid)
{
	puts("Manufacturer:       0x");
	print8(cid->mid);
	puts("OEM \"");
	this_board->putc(cid->oid_0);
	this_board->putc(cid->oid_1);
	puts("\"\nProduct name:       \"");
	this_board->putc(cid->pnm_0);
	this_board->putc(cid->pnm_1);
	this_board->putc(cid->pnm_2);
	this_board->putc(cid->pnm_3);
	this_board->putc(cid->pnm_4);
	puts("\", revision ");
	printdec(cid->prv >> 4);
	puts(".");
	printdec(cid->prv & 15);
	puts("\nSerial number:      ");
	printdec(cid->psn_0 << 24 | cid->psn_1 << 16 | cid->psn_2 << 8 |
	    cid->psn_3);
	puts("\nManufacturing date: ");
	printdec(cid->mdt_1 & 15);
	puts("/");
	printdec(2000+((cid->mdt_0 & 15) << 4)+((cid->mdt_1 & 0xf0) >> 4));
	puts("\nCRC:                0x");
	print8(cid->crc >> 1);
	puts(" b0 = ");
	print8(cid->crc & 1);
	puts("\n");
}

int s3c24xx_mmc_init(int verbose)
{
 	int retries, rc = -2;
	int is_sd = 0;
	u32 *resp;
	u32 hcs = 0;

	SDICON = S3C2410_SDICON_FIFORESET | S3C2410_SDICON_CLOCKTYPE;
	SDIBSIZE = 512;
	if (am_i_s3c2410()) {
		/* S3C2410 has some bug that prevents reliable operation at higher speed */
		//SDIPRE = 0x3e;  /* SDCLK = PCLK/2 / (SDIPRE+1) = 396kHz */
		SDIPRE = 0x02;  /* 2410: SDCLK = PCLK/2 / (SDIPRE+1) = 11MHz */
		SDIDTIMER = 0xffff;
		SDIIMSK2410 = 0x0;
	} else {
		SDIPRE = 0x05;  /* 2410: SDCLK = PCLK / (SDIPRE+1) = 11MHz */
		SDIDTIMER = 0x7fffff;
		SDIIMSK = 0x0;
	}

	udelay(1250000); /* FIXME: 74 SDCLK cycles */

	mmc_csd.c_size = 0;

	puts("Sending reset...\n");

	/* reset */
	retries = 10;
	resp = mmc_cmd(MMC_CMD_RESET, 0, 0);

	resp = mmc_cmd(8, 0x000001aa, CMD_F_RESP);
	if ((resp[0] & 0xff) == 0xaa) {
		puts("The card is either SD2.0 or SDHC\n");
		hcs = 0x40000000;
	}
 
	puts("trying to detect SD Card...\n");
	while (retries--) {
		udelay(1000000);
		resp = mmc_cmd(55, 0x00000000, CMD_F_RESP);
		resp = mmc_cmd(41, hcs | 0x00300000, CMD_F_RESP);

		if (resp[0] & (1 << 30))
			is_sdhc = 1;

		if (resp[0] & (1 << 31)) {
			is_sd = 1;
			break;
		}
	}

	if (retries < 0 && !is_sd)
		return -3;

	/* try to get card id */
	resp = mmc_cmd(MMC_CMD_ALL_SEND_CID, hcs, CMD_F_RESP|CMD_F_RESP_LONG);
	if (resp) {
		if (!is_sd) {
			/* TODO configure mmc driver depending on card
			   attributes */
			mmc_cid_t *cid = (mmc_cid_t *)resp;

			if (verbose)
				print_mmc_cid(cid);
#if 0
			sprintf((char *) mmc_dev.vendor,
				"Man %02x%02x%02x Snr %02x%02x%02x",
				cid->id[0], cid->id[1], cid->id[2],
				cid->sn[0], cid->sn[1], cid->sn[2]);
			sprintf((char *) mmc_dev.product,"%s",cid->name);
			sprintf((char *) mmc_dev.revision,"%x %x",
				cid->hwrev, cid->fwrev);
#endif
		}
		else {
			struct sd_cid *cid = (struct sd_cid *) resp;

			if (verbose)
				print_sd_cid(cid);
#if 0
			sprintf((char *) mmc_dev.vendor,
			    "Man %02 OEM %c%c \"%c%c%c%c%c\"",
			    cid->mid, cid->oid_0, cid->oid_1,
			    cid->pnm_0, cid->pnm_1, cid->pnm_2, cid->pnm_3,
			    cid->pnm_4);
			sprintf((char *) mmc_dev.product, "%d",
			    cid->psn_0 << 24 | cid->psn_1 << 16 |
			    cid->psn_2 << 8 | cid->psn_3);
			sprintf((char *) mmc_dev.revision, "%d.%d",
			    cid->prv >> 4, cid->prv & 15);
#endif
		}


		/* MMC exists, get CSD too */
		resp = mmc_cmd(MMC_CMD_SET_RCA, MMC_DEFAULT_RCA, CMD_F_RESP);
		if (is_sd)
			rca = resp[0] >> 16;

		resp = mmc_cmd(MMC_CMD_SEND_CSD, rca<<16, CMD_F_RESP|CMD_F_RESP_LONG);
		if (resp) {
			mmc_csd_t *csd = (mmc_csd_t *)resp;
			memcpy(&mmc_csd, csd, sizeof(csd));
			rc = 0;
			mmc_ready = 1;
#if 0
			/* FIXME add verbose printout for csd */
			printf("READ_BL_LEN=%u, C_SIZE_MULT=%u, C_SIZE=%u\n",
				csd->read_bl_len, csd->c_size_mult1, csd->c_size);
			printf("size = %u\n", mmc_size(csd));
#endif
		}
	}

	resp = mmc_cmd(MMC_CMD_SELECT_CARD, rca<<16, CMD_F_RESP);

#ifdef CONFIG_MMC_WIDE
	if (is_sd) {
		resp = mmc_cmd(55, rca<<16, CMD_F_RESP);
		resp = mmc_cmd(6, 0x02, CMD_F_RESP);
		wide = 1;
	}
#endif

	return rc;
}


