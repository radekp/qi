#include "kboot.h"
#include <neo_gta02.h>
#include "blink_led.h"
#include <string.h>
#define __ARM__
#include <image.h>
#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
typedef unsigned int uint32_t;

#include <setup.h>
#include "nand_read.h"

/* See also ARM920T    Technical reference Manual */
#define C1_MMU          (1<<0)          /* mmu off/on */
#define C1_ALIGN        (1<<1)          /* alignment faults off/on */
#define C1_DC           (1<<2)          /* dcache off/on */

#define C1_BIG_ENDIAN   (1<<7)          /* big endian off/on */
#define C1_SYS_PROT     (1<<8)          /* system protection */
#define C1_ROM_PROT     (1<<9)          /* ROM protection */
#define C1_IC           (1<<12)         /* icache off/on */
#define C1_HIGH_VECTORS (1<<13)         /* location of vectors: low/high addresses */

size_t strlen(const char *s)
{
	size_t n = 0;

	while (*s++)
		n++;

	return n;
}

char *strcpy(char *dest, const char *src)
{
	char * dest_orig = dest;

	while (*src)
		*dest++ = *src++;

	return dest_orig;
}

unsigned int _ntohl(unsigned int n) {
	return ((n & 0xff) << 24) | ((n & 0xff00) << 8) |
			       ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24);
}

void bootloader_second_phase(void)
{
	image_header_t	*hdr;
        unsigned long i = 0;
	int	machid = 1304; /* GTA02 */
	void	(*theKernel)(int zero, int arch, uint params);
	struct tag * params_base = (struct tag *)0x30000100; /* atags need to live here */
	struct tag *params = params_base;
	const char * cmdline = "rootfstype=ext2 root=/dev/mmcblk0p1 console=ttySAC2,115200 loglevel=8 init=/sbin/init ro";
	const char *p = cmdline;
	void * kernel_nand = (void *)(TEXT_BASE - 4 * 1024 * 1024);

	puts("Checking kernel... ");

	if (nand_read_ll(kernel_nand, 0x80000, 4096) < 0) {
		puts ("Kernel header read failed\n");
		goto unhappy;
	}

	hdr = (image_header_t *)kernel_nand;

	if (_ntohl(hdr->ih_magic) != IH_MAGIC) {
		puts("Unknown image magic ");
		print32(hdr->ih_magic);
		goto unhappy;
	}

	puts((const char *)hdr->ih_name);

	puts("Fetching kernel...");

	if (nand_read_ll(kernel_nand, 0x80000, ((32 * 1024) +
						(_ntohl(hdr->ih_size) +
						sizeof(hdr) + 2048)) &
					       ~(2048 - 1)) < 0) {
		puts ("Kernel body read failed\n");
		goto unhappy;
	}

	puts(" Done");

	theKernel = (void (*)(int, int, uint))
				(((char *)hdr) + sizeof(image_header_t));

	/* first tag */
	params->hdr.tag = ATAG_CORE;
	params->hdr.size = tag_size (tag_core);
	params->u.core.flags = 0;
	params->u.core.pagesize = 0;
	params->u.core.rootdev = 0;
	params = tag_next (params);

	/* revision tag */
	params->hdr.tag = ATAG_REVISION;
	params->hdr.size = tag_size (tag_revision);
	params->u.revision.rev = 0x350;
	params = tag_next (params);

	/* memory tags */
	params->hdr.tag = ATAG_MEM;
	params->hdr.size = tag_size (tag_mem32);
	params->u.mem.start = 0x30000000;
	params->u.mem.size = 128 * 1024 * 1024;
	params = tag_next (params);

	/* kernel commandline */
	/* eat leading white space */
	for (p = cmdline; *p == ' '; p++);

	if (*p) {
		params->hdr.tag = ATAG_CMDLINE;
		params->hdr.size =
			 (sizeof (struct tag_header) + strlen (p) + 1 + 4) >> 2;
		strcpy (params->u.cmdline.cmdline, p);
		params = tag_next (params);
	}

	/* needs to always be the last tag */
	params->hdr.tag = ATAG_NONE;
	params->hdr.size = 0;

	puts ("Running Linux...\n\n");

	/* trash the cache */

        /* turn off I/D-cache */
        asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));
        i &= ~(C1_DC | C1_IC);
        asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));

        /* flush I/D-cache */
        i = 0;
        asm ("mcr p15, 0, %0, c7, c7, 0": :"r" (i));

	/* ooh that's it, we're gonna try boot this image! */

	theKernel(0, machid, (unsigned int)params_base);

	/* that didn't quite pan out */

unhappy:
	while (1)
		blue_on(1);
}
