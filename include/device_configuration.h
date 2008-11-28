/* define the easily changed settings for the device here */

#define CFG_LINUX_MACHINE_ID 1304
#define CFG_LINUX_ATAG_ADDRESS 0x30000100
#define CFG_NAND_OFFSET_FOR_KERNEL_PARTITION 0x80000
/*
 * we follow GTA02 layout actually to make life easier allowing NOR DFU during
 * our initial development on GTA02
 */
#define CFG_LINUX_CMDLINE_BACKUP "mtdparts=neo1973-nand:" \
					"0x00040000(kboot)," \
					"0x00040000(cmdline)," \
					"0x00800000(backupkernel)," \
					"0x000a0000(extra)," \
					"0x00040000(identity)," \
					"0x0f6a0000(backuprootfs) " \
				"rootfstype=jffs2 " \
				"root=/dev/mtdblock5 " \
				"console=ttySAC2,115200 " \
				"loglevel=4 " \
				"init=/sbin/init ro"

#define CFG_LINUX_CMDLINE 	"rootfstype=ext2 " \
				"root=/dev/mmcblk0p1 " \
				"console=ttySAC2,115200 " \
				"loglevel=8 " \
				"init=/sbin/init ro"
#define CFG_LINUX_BIGGEST_KERNEL (4 * 1024 * 1024)
#define CFG_MACHINE_REVISION 0x350
#define CFG_MEMORY_REGION_START 0x30000000
#define CFG_MEMORY_REGION_SIZE (128 * 1024 * 1024)

