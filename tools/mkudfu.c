/*
 * USB DFU file trailer tool
 * (C) Copyright by OpenMoko, Inc.
 * Author: Harald Welte <laforge@openmoko.org>
 *
 * based on mkimage.c, copyright information as follows:
 *
 * (C) Copyright 2000-2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 * All rights reserved.
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __WIN32__
#include <netinet/in.h>		/* for host / network byte order conversions	*/
#endif
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if defined(__BEOS__) || defined(__NetBSD__) || defined(__APPLE__)
#include <inttypes.h>
#endif

#ifdef __WIN32__
typedef unsigned int __u32;

#define SWAP_LONG(x) \
	((__u32)( \
		(((__u32)(x) & (__u32)0x000000ffUL) << 24) | \
		(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) | \
		(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) | \
		(((__u32)(x) & (__u32)0xff000000UL) >> 24) ))
typedef		unsigned char	uint8_t;
typedef		unsigned short	uint16_t;
typedef		unsigned int	uint32_t;

#define     ntohl(a)	SWAP_LONG(a)
#define     htonl(a)	SWAP_LONG(a)
#endif	/* __WIN32__ */

#ifndef	O_BINARY		/* should be define'd on __WIN32__ */
#define O_BINARY	0
#endif

#include "usb_dfu_trailer.h"

extern int errno;

#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

static char *cmdname;

static char *datafile;
static char *imagefile;


static void usage()
{
	fprintf (stderr, "%s - create / display u-boot DFU trailer\n", cmdname);
	fprintf (stderr, "Usage: %s -l image\n"
			 "          -l ==> list image header information\n"
			 "       %s -v VID -p PID -r REV -d data_file image\n",
		cmdname, cmdname);
	fprintf (stderr, "          -v ==> set vendor ID to 'VID'\n"
			 "          -p ==> set product ID system to 'PID'\n"
			 "          -r ==> set hardware revision to 'REV'\n"
			 "          -d ==> use 'data_file' as input file\n"
		);
	exit (EXIT_FAILURE);
}

static void print_trailer(struct uboot_dfu_trailer *trailer)
{
	printf("===> DFU Trailer information:\n");
	printf("Trailer Vers.:	%d\n", trailer->version);
	printf("Trailer Length:	%d\n", trailer->length);
	printf("VendorID:	0x%04x\n", trailer->vendor);
	printf("ProductID:	0x%04x\n", trailer->product);
	printf("HW Revision:	0x%04x\n", trailer->revision);
}

static void copy_file (int ifd, const char *datafile, int pad)
{
	int dfd;
	struct stat sbuf;
	unsigned char *ptr;
	int tail;
	int zero = 0;
	int offset = 0;
	int size;

	if ((dfd = open(datafile, O_RDONLY|O_BINARY)) < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
			cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size,
				    PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		fprintf (stderr, "%s: Can't read %s: %s\n",
			cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	size = sbuf.st_size - offset;
	if (write(ifd, ptr + offset, size) != size) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (pad && ((tail = size % 4) != 0)) {

		if (write(ifd, (char *)&zero, 4-tail) != 4-tail) {
			fprintf (stderr, "%s: Write error on %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}
	}

	(void) munmap((void *)ptr, sbuf.st_size);
	(void) close (dfd);
}


int main(int argc, char **argv)
{
	int ifd;
	int lflag = 0;
	struct stat sbuf;
	u_int16_t opt_vendor, opt_product, opt_revision;
	struct uboot_dfu_trailer _hdr, _mirror, *hdr = &_hdr;

	opt_vendor = opt_product = opt_revision = 0;

	cmdname = *argv;

	while (--argc > 0 && **++argv == '-') {
		while (*++*argv) {
			switch (**argv) {
			case 'l':
				lflag = 1;
				break;
			case 'v':
				if (--argc <= 0)
					usage ();
				opt_vendor = strtoul(*++argv, NULL, 16);
				goto NXTARG;
			case 'p':
				if (--argc <= 0)
					usage ();
				opt_product = strtoul(*++argv, NULL, 16);
				goto NXTARG;
			case 'r':
				if (--argc <= 0)
					usage ();
				opt_revision = strtoul(*++argv, NULL, 16);
				goto NXTARG;
			case 'd':
				if (--argc <= 0)
					usage ();
				datafile = *++argv;
				goto NXTARG;
			case 'h':
				usage();
				break;
			default:
				usage();
			}
		}
NXTARG:		;
	}

	if (argc != 1)
		usage();

	imagefile = *argv;

	if (lflag)
		ifd = open(imagefile, O_RDONLY|O_BINARY);
	else
		ifd = open(imagefile, O_RDWR|O_CREAT|O_TRUNC|O_BINARY, 0666);

	if (ifd < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (lflag) {
		unsigned char *ptr;
		/* list header information of existing image */
		if (fstat(ifd, &sbuf) < 0) {
			fprintf (stderr, "%s: Can't stat %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		if ((unsigned)sbuf.st_size < sizeof(struct uboot_dfu_trailer)) {
			fprintf (stderr,
				"%s: Bad size: \"%s\" is no valid image\n",
				cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		ptr = (unsigned char *)mmap(0, sbuf.st_size,
					    PROT_READ, MAP_SHARED, ifd, 0);
		if ((caddr_t)ptr == (caddr_t)-1) {
			fprintf (stderr, "%s: Can't read %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		dfu_trailer_mirror(hdr, ptr+sbuf.st_size);

		if (hdr->magic != UBOOT_DFU_TRAILER_MAGIC) {
			fprintf (stderr,
				"%s: Bad Magic Number: \"%s\" is no valid image\n",
				cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		/* for multi-file images we need the data part, too */
		print_trailer(hdr);

		(void) munmap((void *)ptr, sbuf.st_size);
		(void) close (ifd);

		exit (EXIT_SUCCESS);
	}

	/* if we're not listing: */

	copy_file (ifd, datafile, 0);

	memset (hdr, 0, sizeof(struct uboot_dfu_trailer));

	/* Build new header */
	hdr->version	= UBOOT_DFU_TRAILER_V1;
	hdr->magic	= UBOOT_DFU_TRAILER_MAGIC;
	hdr->length	= sizeof(struct uboot_dfu_trailer);
	hdr->vendor	= opt_vendor;
	hdr->product	= opt_product;
	hdr->revision	= opt_revision;

	print_trailer(hdr);
	dfu_trailer_mirror(&_mirror, (unsigned char *)hdr+sizeof(*hdr));

	if (write(ifd, &_mirror, sizeof(struct uboot_dfu_trailer))
					!= sizeof(struct uboot_dfu_trailer)) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (fstat(ifd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (close(ifd)) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	exit (EXIT_SUCCESS);
}
