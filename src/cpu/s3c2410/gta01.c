/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: Andy Green <andy@openmoko.com>
 *
 * Configuation settings for the OPENMOKO Neo GTA02 Linux GSM phone
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
#include <neo_gta01.h>
#include <serial-s3c24xx.h>
#include <ports-s3c24xx.h>
#include <s3c24xx-mci.h>
#include <i2c-bitbang-s3c24xx.h>
#include <pcf50606.h>

#define GTA01_DEBUG_UART 0
#define PCF50606_I2C_ADS 0x08


struct pcf50606_init {
        u8 index;
        u8 value;
};

/* initial register set for PCF50606 in Neo1973 devices */
const struct pcf50606_init pcf50606_initial_regs[] = {
	{ PCF50606_REG_OOCS,	0x00 },
	{ PCF50606_REG_INT1M,	0x00 },
	{ PCF50606_REG_INT2M,	0x00 },
	{ PCF50606_REG_INT3M,	PCF50606_INT3_TSCPRES },
	{ PCF50606_REG_OOCC1, 	PCF50606_OOCC1_RTCWAK | PCF50606_OOCC1_CHGWAK |
				  PCF50606_OOCC1_EXTONWAK_HIGH },
	{ PCF50606_REG_OOCC2,	PCF50606_OOCC2_ONKEYDB_14ms | PCF50606_OOCC2_EXTONDB_14ms },
	{ PCF50606_REG_PSSC,	0x00 },
	{ PCF50606_REG_PWROKM,	0x00 },
	{ PCF50606_REG_DCDC1,	0x18 },	/* GL_1V5: off */
	{ PCF50606_REG_DCDC2,	0x00 },
	{ PCF50606_REG_DCDC3,	0x00 },
	{ PCF50606_REG_DCDC4,	0x30 }, /* 1.25A */

	{ PCF50606_REG_DCDEC1,	0xe8 }, /* IO_3V3: on */
	{ PCF50606_REG_DCDEC2,	0x00 },

	{ PCF50606_REG_DCUDC1,	0xc4 }, /* CORE_1V8: 2.1V if PWREN2 = HIGH */
	{ PCF50606_REG_DCUDC2,	0x30 }, /* 1.25A current limit */

	{ PCF50606_REG_IOREGC,	0xf8 }, /* CODEC_3V3: on */
	{ PCF50606_REG_D1REGC1,	0x16 }, /* BT_3V15: off */

	{ PCF50606_REG_D2REGC1,	0x10 }, /* GL_2V5: off */

	{ PCF50606_REG_D3REGC1,	0xec }, /* STBY_1V8: 2.1V */

	{ PCF50606_REG_LPREGC1,	0xf8 }, /* LCM_3V3: on */
	{ PCF50606_REG_LPREGC2,	0x00 },

	{ PCF50606_REG_MBCC1,	0x01 }, /* CHGAPE */
	{ PCF50606_REG_MBCC2,	0x00 },	/* unlimited charging */
	{ PCF50606_REG_MBCC3,	0x1a }, /* 0.2*Ifast, 4.20V */
	{ PCF50606_REG_BBCC,	0x1f }, /* 400uA */
	{ PCF50606_REG_ADCC1,	0x00 },
	{ PCF50606_REG_ADCC2,	0x00 },
	{ PCF50606_REG_ACDC1,	0x86 },	/* ACD thresh 1.6V, enabled */
	{ PCF50606_REG_BVMC,	PCF50606_BVMC_THRSHLD_3V3 },
	{ PCF50606_REG_PWMC1,	0x00 },
	{ PCF50606_REG_LEDC1,	0x00 },
	{ PCF50606_REG_LEDC2,	0x00 },
	{ PCF50606_REG_GPOC1,	0x00 },
	{ PCF50606_REG_GPOC2,	0x00 },
	{ PCF50606_REG_GPOC3,	0x00 },
	{ PCF50606_REG_GPOC4,	0x00 },
	{ PCF50606_REG_GPOC5,	0x00 },
};


static const struct board_variant board_variants[] = {
	[0] = {
		.name = "Bv4",
		.machine_revision = 0x240,
	}
};


void port_init_gta01(void)
{
	int n;
	unsigned int * MPLLCON = (unsigned int *)0x4c000004;

	rGPACON = 0x005E0FFF;
	rGPADAT = 0x00010000; /* nNAND_WP set high */

	rGPBCON = 0x00045455;
	rGPBUP  = 0x000007FF; 
	rGPBDAT = 0x00000004; /* SD-card pwr off */

	rGPCCON = 0xAAAA12A9;
	rGPCUP  = 0x0000FFFF;

	rGPDCON = 0xAAAAAAAA;
	rGPDUP  = 0x0000FFFF;

	rGPECON = 0xAAAAAAAA;
	rGPEUP  = 0x0000FFFF;

	rGPFCON = 0x0000aa99;
	rGPFUP  = 0x000000FF;
	rGPFDAT = 0x00000004;

	rGPGCON = 0xFF14F0F8;
	rGPGUP  = 0x0000AFEF;

	rGPHCON = 0x0000FAAA;
	rGPHUP  = 0x000007FF;


	/* Load PMU with safe values */

	for (n = 0; n < ARRAY_SIZE(pcf50606_initial_regs); n++)
		i2c_write_sync(&bb_s3c24xx, PCF50606_I2C_ADS,
				pcf50606_initial_regs[n].index,
				pcf50606_initial_regs[n].value);

	/* Give a short vibrate notification */
	rGPBDAT |= (1 << 3);
	udelay(1000000);
	rGPBDAT &= ~(1 << 3);


	/* change CPU to 266MHz 1:2:4 */
	*MPLLCON = ((0x7d << 12) + (0x1 << 4) + 0x1);
	/* Delay after update of PLL: Page 7-19, seven nops */
	asm __volatile__ (
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
		"nop\n"
	);

	
	/* set debug UART working at 115kbps */
	serial_init_115200_s3c24xx(GTA01_DEBUG_UART, 66 /* 66.5MHz PCLK */);
}


int sd_card_init_gta01(void)
{
	int retval = -1;

	/* Check if AUX is held. Then skip SD-card kernels!
	 * FIXME: This would be nicer to do with an API.
	 */
	if (!(rGPFDAT & (1 << 6))) {
		return -1;
	}

	/* if SD card inserted, power it up and initialize*/
	if (!(rGPFDAT & (1 << 5)))
	{
		rGPBDAT &= ~(1 << 2);
		retval = s3c24xx_mmc_init(1);
	}
	return retval;
}

int sd_card_block_read_gta01(unsigned char * buf, unsigned long start512,
				int blocks512)
{
	return s3c24xx_mmc_bread(0, start512, blocks512, buf);
}

int is_this_board_gta01(void)
{
	/* FIXME: How to check for GTA01 ? */
        return 1;
}

const struct board_variant const * get_board_variant_gta01(void)
{
	return &board_variants[0];
}

static __attribute__ (( section (".steppingstone") )) void putc_gta01(char c)
{
	serial_putc_s3c24xx(GTA01_DEBUG_UART, c);
}

static void close_gta01(void)
{
	/* set I2C GPIO back to peripheral unit */
	(bb_s3c24xx.close)();
}

/* Here we will care only about AUX button as polling for PWR button
 * through i2c slows down the boot */

static u8 get_ui_keys_gta01(void)
{
	u8 keys;
	u8 ret = 0;
	static u8 old_keys = 0; /* previous state for debounce */
	static u8 older_keys = 0; /* previous debounced output for edge detect */

	/* GPF6 is AUX on GTA01, map to UI_ACTION_SKIPKERNEL, down = 0 */
	keys = ! (rGPFDAT & (1 << 6));

	/* edge action */
	if ((old_keys & 1) && !(older_keys & 1))
		ret |= UI_ACTION_SKIPKERNEL;

	older_keys = old_keys;
	old_keys = keys;

	return ret;
}

static u8 get_ui_debug_gta01(void)
{
	/* PWR button state can be seen in OOCS b0, down = 0, map to UI_ACTION_ADD_DEBUG */
	return !(i2c_read_sync(&bb_s3c24xx, PCF50606_I2C_ADS, PCF50606_REG_OOCS) & 1);
}

/*
 * API for bootloader on this machine
 */

const struct board_api board_api_gta01 = {
	.name = "Neo1973 GTA01",
	.linux_machine_id = 1182,
	.linux_mem_start = 0x30000000,
	.linux_mem_size = (128 * 1024 * 1024),
	.linux_tag_placement = 0x30000000 + 0x100,
	.get_board_variant = get_board_variant_gta01,
	.is_this_board = is_this_board_gta01,
	.port_init = port_init_gta01,
	.putc = putc_gta01,
	.close = close_gta01,
	.get_ui_keys = get_ui_keys_gta01,
	.get_ui_debug = get_ui_debug_gta01,

	.commandline_board = "mtdparts="
				"neo1973-nand:"
				 "0x00040000(qi),"
				 "0x00004000(u-boot_env),"
				 "0x00200000(kernel),"
				 "0x000a0000(splash),"
				 "0x03d1c000(rootfs) "
			       "loglevel=4 "
			       "console=tty0 "
			       "console=ttySAC0,115200 "
			       "init=/sbin/init "
			       "ro ",
	.commandline_board_debug = " loglevel=8 ",
	.noboot = "boot/noboot-GTA01",
	.append = "boot/append-GTA01",
	/* these are the ways we could boot GTA01 in order to try */
	.kernel_source = {
		[0] = {
			.name = "SD Card EXT2 Kernel p1",
			.block_init = sd_card_init_gta01,
			.block_read = sd_card_block_read_gta01,
			.partition_index = 1,
			.filesystem = FS_EXT2,
			.filepath = "boot/uImage-GTA01.bin",
			.commandline_append = "root=/dev/mmcblk0p1 rootdelay=1 ",
		},
		[1] = {
			.name = "SD Card EXT2 Kernel p2",
			.block_init = sd_card_init_gta01,
			.block_read = sd_card_block_read_gta01,
			.partition_index = 2,
			.filesystem = FS_EXT2,
			.filepath = "boot/uImage-GTA01.bin",
			.commandline_append = "root=/dev/mmcblk0p2 rootdelay=1 ",
		},
		[2] = {
			.name = "SD Card EXT2 Kernel p3",
			.block_init = sd_card_init_gta01,
			.block_read = sd_card_block_read_gta01,
			.partition_index = 3,
			.filesystem = FS_EXT2,
			.filepath = "boot/uImage-GTA01.bin",
			.commandline_append = "root=/dev/mmcblk0p3 rootdelay=1 ",
		},
		[3] = {
			.name = "NAND Kernel",
			.block_read = nand_read_ll,
			.offset_blocks512_if_no_partition = 0x44000 / 512,
			.filesystem = FS_RAW,
			.commandline_append = " rootfstype=ubifs " \
				       " ubi.mtd=4,512 " \
				       " root=ubi0:om-gta01-rootfs ",
		},
	},
};
