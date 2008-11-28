#include <qi.h>
#include <neo_smdk6410.h>
#include <serial-s3c64xx.h>

#define SMDK6410_DEBUG_UART 0

extern const struct board_variant const * get_board_variant_smdk6410(void);

int is_this_board_smdk6410(void)
{
	/* FIXME: find something smdk6410 specific */
	return 1;
}

static void putc_smdk6410(char c)
{
	serial_putc_s3c64xx(SMDK6410_DEBUG_UART, c);
}

int sd_card_init_smdk6410(void)
{
	extern int s3c6410_mmc_init(int verbose);

	return s3c6410_mmc_init(1);
}

int sd_card_block_read_smdk6410(unsigned char * buf, unsigned long start512,
								  int blocks512)
{
unsigned long s3c6410_mmc_bread(int dev_num, unsigned long blknr,
					       unsigned long blkcnt, void *dst);

	return s3c6410_mmc_bread(0, start512, blocks512, buf);
}

/*
 * our API for bootloader on this machine
 */
const struct board_api board_api_smdk6410 = {
	.name = "SMDK6410",
	.linux_machine_id = 1866 /* 1626 */,
	.linux_mem_start = 0x50000000,
	.linux_mem_size = (128 * 1024 * 1024),
	.linux_tag_placement = 0x50000000 + 0x100,
	.get_board_variant = get_board_variant_smdk6410,
	.is_this_board = is_this_board_smdk6410,
	.putc = putc_smdk6410,
	.kernel_source = {
		[0] = {
			.name = "SD Card rootfs",
			.block_read = sd_card_block_read_smdk6410,
			.filesystem = FS_EXT2,
			.partition_index = 2,
			.filepath = "boot/uImage.bin",
			.commandline = "console=ttySAC0,115200 " \
				       "loglevel=3 init=/bin/sh " \
				       "root=/dev/mmcblk0p2"
		},
		[1] = {
			.name = "SD Card backup rootfs",
			.block_read = sd_card_block_read_smdk6410,
			.filesystem = FS_EXT2,
			.partition_index = 3,
			.filepath = "boot/uImage.bin",
			.commandline = "console=ttySAC0,115200 " \
				       "loglevel=8 init=/bin/sh " \
				       "root=/dev/mmcblk0p3"
		},	},
};

