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
	. = 0x00000000;

	/* this is intended to take the first 4KBytes of stuff initially.
	 * We have to make sure we have .rodata* in there for everything
	 * because we do not compile PIC.
	 */

	. = ALIGN(4);
	.text      :
	{
	  src/start.o	(.text .rodata* .data)
	  src/lowlevel_init.o(.text .rodata* .data)
	  src/start_kboot.o	(.text .rodata* .data)
	  src/serial.o	(.text .rodata* .data)
	  src/blink_led.o	(.text .rodata* .data)
	  * (.rodata* .data)
	}

	. = ALIGN(4);
	.everything_else : { *(.text) }

	. = ALIGN(4);
	.got : 
    { 
        *(.got) 
    }

	. = 0x32000000 ;
	__bss_start = .;
	.bss (NOLOAD) : 
    { 
        *(.bss) 
    }
	_end = .;
}
