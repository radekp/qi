/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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

#ifndef _S3C24XX_MMC_H_
#define _S3C24XX_MMC_H_
#include <mmc.h>

int s3c24xx_mmc_init(int verbose);
u32 s3c24xx_mmc_bread(int dev_num, u32 blknr, u32 blkcnt, void *dst);
int s3c24xx_mmc_read(u32 src, u8 *dst, int size);
int s3c24xx_mmc_write(u8 *src, u32 dst, int size);

#endif /* _MMC_H_ */
