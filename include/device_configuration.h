/* define the easily changed settings for the device here */

#define CFG_NAND_OFFSET_FOR_KERNEL_PARTITION 0x80000
#define CFG_LINUX_MACHINE_ID 1304
#define CFG_LINUX_ATAG_ADDRESS 0x30000100
#define CFG_LINUX_CMDLINE 	"rootfstype=ext2 " \
				"root=/dev/mmcblk0p1 " \
				"console=ttySAC2,115200 " \
				"loglevel=8 " \
				"init=/sbin/init ro"
#define CFG_LINUX_BIGGEST_KERNEL (4 * 1024 * 1024)
#define CFG_MACHINE_REVISION 0x350
#define CFG_MEMORY_REGION_START 0x30000000
#define CFG_MEMORY_REGION_SIZE (128 * 1024 * 1024)

