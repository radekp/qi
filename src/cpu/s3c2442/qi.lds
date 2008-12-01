/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
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

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
/*OUTPUT_FORMAT("elf32-arm", "elf32-arm", "elf32-arm")*/
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	__steppingstone_always = 0x40000000;
	__qi_sdram_copy = 0x33000000;

	. = __steppingstone_always;

	/* this is intended to take the first 4KBytes of stuff initially.
	 * We have to make sure we have .rodata* in there for everything
	 * because we do not compile PIC.
	 */

	.text __steppingstone_always : AT (0x0)
	{
	  src/cpu/s3c2442/start.o		(.text .rodata* .data .bss)
	  src/cpu/s3c2442/lowlevel_init.o	(.text .rodata* .data .bss)
	  src/cpu/s3c2442/start_qi.o		(.text .rodata* .data .bss)
	  src/cpu/s3c2442/nand_read.o		(.text .rodata* .data .bss)
	  src/cpu/s3c2442/serial-s3c24xx.o	(.text .rodata* .data .bss)
	  src/memory-test.o			(.text .rodata* .data .bss)
	  src/utils.o				(.text .rodata* .data .bss)
	  src/ctype.o				(.text .rodata* .data .bss)
	  *					(.steppingstone)
	}

	. = ALIGN(4);
	.everything_else  ADDR (.text) - __steppingstone_always + SIZEOF (.text) + __qi_sdram_copy :
		AT ( ADDR (.text) - __steppingstone_always + SIZEOF (.text) ) { *(.text .rodata* .data) }

	. = __qi_sdram_copy + 0x800000 ;
	__bss_start = .;
	.bss (NOLOAD) : 
    { 
        *(.bss) 
    }

	_end = .;
}
