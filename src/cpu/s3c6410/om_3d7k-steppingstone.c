#include <qi.h>
#include <neo_om_3d7k.h>
#include <s3c6410.h>
#include <serial-s3c64xx.h>

#define OM_3D7K_DEBUG_UART 3

/* out of steppingstone */
extern const struct board_variant const * get_board_variant_om_3d7k(void);
extern void port_init_om_3d7k(void);


int is_this_board_om_3d7k(void)
{
	/* FIXME: find something om_3d7k specific */
	return 1;
}

static void putc_om_3d7k(char c)
{
	serial_putc_s3c64xx(OM_3D7K_DEBUG_UART, c);
}

int sd_card_init_om_3d7k(void)
{
	extern int s3c6410_mmc_init(int verbose);

	return s3c6410_mmc_init(1);
}

int sd_card_block_read_om_3d7k(unsigned char * buf, unsigned long start512,
							       int blocks512)
{
unsigned long s3c6410_mmc_bread(int dev_num, unsigned long blknr, unsigned long blkcnt,
								     void *dst);

	return s3c6410_mmc_bread(0, start512, blocks512, buf);
}

/*
 * our API for bootloader on this machine
 */

/* for initrd:
 * 			.initramfs_filepath = "boot/initramfs.gz",
 * and
 *  "root=/dev/ram ramdisk_size=6000000"
 */

static u8 get_ui_keys_om_3d7k(void)
{
	u8 keys;
	u8 ret;
	static u8 old_keys = 0; /* previous state for debounce */
	static u8 old_ret = 0; /* previous debounced output for edge detect */

	/* GPN1 is MINUS on OM_3D7K, map to UI_ACTION_ADD_DEBUG, down = 1 */
	keys = !!(__REG(GPMDAT) & (1 << 1));

	if (keys == old_keys)
		ret = keys;
	else
		ret = old_keys;

	/* edge action */
	if ((ret & 1) && !(old_ret & 1))
		ret |= UI_ACTION_SKIPKERNEL;

	old_keys = keys;
	old_ret = ret;

	return ret;
}

const struct board_api board_api_om_3d7k = {
	.name = "OM_3D7K",
	.linux_machine_id = 2120,
	.linux_mem_start = 0x50000000,
	.linux_mem_size = (128 * 1024 * 1024),
	.linux_tag_placement = 0x50000000 + 0x100,
	.get_board_variant = get_board_variant_om_3d7k,
	.is_this_board = is_this_board_om_3d7k,
	.port_init = port_init_om_3d7k,
	.putc = putc_om_3d7k,
	.noboot = "boot/noboot-OM_3D7K",
	.append = "boot/append-OM_3D7K",
	.get_ui_keys = get_ui_keys_om_3d7k,
	.commandline_board = "console=tty0 "
			     "console=ttySAC3,115200 "
			     "init=/sbin/init "
			     "rootdelay=1 no_console_suspend "
			     "ro ",
	.commandline_board_debug =  " loglevel=8",
	.kernel_source = {
		[0] = {
			.name = "SD Card rootfs",
			.block_read = sd_card_block_read_om_3d7k,
			.filesystem = FS_EXT2,
			.partition_index = 2,
			.filepath = "boot/uImage-OM_3D7K.bin",
			.commandline_append = "root=/dev/mmcblk0p2 ",
		},
		[1] = {
			.name = "SD Card backup rootfs",
			.block_read = sd_card_block_read_om_3d7k,
			.filesystem = FS_EXT2,
			.partition_index = 3,
			.filepath = "boot/uImage-OM_3D7K.bin",
			.commandline_append = "root=/dev/mmcblk0p3 ",
		},
	},
};

