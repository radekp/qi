/*
 * (C) Copyright 2008 Openmoko, Inc.
 * Author: Andy Green <andy@openmoko.org>
 *
 * Little utils for print and strings
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

#include "kboot.h"
#include <string.h>

#define DEBUG_CONSOLE_UART UART2

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

int puts(const char *string)
{
	while (*string)
		serial_putc(DEBUG_CONSOLE_UART, *string++);
	serial_putc(DEBUG_CONSOLE_UART, '\n');

	return 1;
}

/* done like this to avoid needing statics in steppingstone */
void printnybble(unsigned char n)
{
	if (n < 10)
		serial_putc(DEBUG_CONSOLE_UART, '0' + n);
	else
		serial_putc(DEBUG_CONSOLE_UART, 'a' + n - 10);
}

void printhex(unsigned char n)
{
	printnybble((n >> 4) & 15);
	printnybble(n & 15);
}

void print32(unsigned int u)
{
	printhex(u >> 24);
	printhex(u >> 16);
	printhex(u >> 8);
	printhex(u);
}

void hexdump(unsigned char *start, int len)
{
	int n;

	while (len > 0) {
		print32((int)start);
		serial_putc(DEBUG_CONSOLE_UART, ':');
		serial_putc(DEBUG_CONSOLE_UART, ' ');
		for (n = 0; n < 16; n++) {
			printhex(*start++);
			serial_putc(DEBUG_CONSOLE_UART, ' ');
		}
		serial_putc(DEBUG_CONSOLE_UART, '\n');
		len -= 16;
	}
}

