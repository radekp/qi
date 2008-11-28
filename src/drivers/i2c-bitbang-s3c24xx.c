/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: Andy Green <andy@openmoko.com>
 *
 * s3c24xx-specific i2c shared by, eg, GTA02 and GTA03
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
 *
 */

#include <qi.h>
#include <i2c-bitbang.h>
#include <ports-s3c24xx.h>

static char i2c_read_sda_s3c24xx(void)
{
	return (rGPEDAT & 0x8000) != 0;
}

static void i2c_set_s3c24xx(char clock, char data)
{
	if (clock) /* SCL <- input */
		rGPECON = (rGPECON & ~0x30000000);
	else { /* SCL <- output 0 */
		rGPEDAT = (rGPEDAT & ~0x4000);
		rGPECON = (rGPECON & ~0x30000000) | 0x10000000;
	}
	if (data) /* SDA <- input */
		rGPECON = (rGPECON & ~0xc0000000);
	else { /* SDA <- output 0 */
		rGPEDAT = (rGPEDAT & ~0x8000);
		rGPECON = (rGPECON & ~0xc0000000) | 0x40000000;
	}
}

static void i2c_close_s3c24xx(void)
{
	/* set back to hardware I2C ready for Linux */
	rGPECON = (rGPECON & ~0xf0000000) | 0xa0000000;
}

static void i2c_spin_s3c24xx(void)
{
	int n;

	for (n = 0; n < 1000; n++)
		rGPJDAT |= (1 << 5);
}

struct i2c_bitbang bb_s3c24xx = {
	.read_sda = i2c_read_sda_s3c24xx,
	.set = i2c_set_s3c24xx,
	.spin = i2c_spin_s3c24xx,
	.close = i2c_close_s3c24xx,
};
