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

#include <qi.h>
#include <string.h>

static void (*putc_func)(char) = NULL;


void set_putc_func(void (*p)(char))
{
	putc_func = p;
}

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
	*dest = '\0';

	return dest_orig;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char * dest_orig = dest;

	while (*src && n--)
		*dest++ = *src++;

	if (n)
		*dest = '\0';

	return dest_orig;
}


int strcmp(const char *s1, const char *s2)
{
	while (1) {
		if (*s1 != *s2)
			return *s1 - *s2;
		if (!*s1)
			return 0;
		s1++;
		s2++;
	}
}

char *strchr(const char *s, int c)
{
	while ((*s) && (*s != c))
		s++;

	if (*s == c)
		return (char *)s;

	return NULL;
}

int puts(const char *string)
{
	while (*string)
		(putc_func)(*string++);

	return 1;
}

/* done like this to avoid needing statics in steppingstone */
void printnybble(unsigned char n)
{
	if (n < 10)
		(putc_func)('0' + n);
	else
		(putc_func)('a' + n - 10);
}

void print8(unsigned char n)
{
	printnybble((n >> 4) & 15);
	printnybble(n & 15);
}

void print32(unsigned int u)
{
	print8(u >> 24);
	print8(u >> 16);
	print8(u >> 8);
	print8(u);
}

void hexdump(unsigned char *start, int len)
{
	int n;

	while (len > 0) {
		print32((int)start);
		(putc_func)(':');
		(putc_func)(' ');
		for (n = 0; n < 16; n++) {
			print8(*start++);
			(putc_func)(' ');
		}
		(putc_func)('\n');
		len -= 16;
	}
}

void printdec(int n)
{
	int d[] = {
		1 * 1000 * 1000 * 1000,
		     100 * 1000 * 1000,
		      10 * 1000 * 1000,
		       1 * 1000 * 1000,
			    100 * 1000,
			     10 * 1000,
			      1 * 1000,
				   100,
				    10,
				     1,
				     0
	};
	int flag = 0;
	int div = 0;

	if (n < 0) {
		(putc_func)('-');
		n = -n;
	}

	while (d[div]) {
		int r = 0;
		while (n >= d[div]) {
			r++;
			n -= d[div];
		}
		if (r || flag || (d[div] == 1)) {
			(putc_func)('0' + r);
			flag = 1;
		}
		div++;
	}
}

void *memcpy(void *dest, const void *src, size_t n)
{
	u8 const * ps = src;
	u8 * pd = dest;

	while (n--)
		*pd++ = *ps++;

	return dest;
}

void *memset(void *s, int c, size_t n)
{
	u8 * p = s;

	while (n--)
		*p++ = c;

	return s;
}

/* improbably simple malloc and free for small and non-intense allocation
 * just moves the allocation ptr forward each time and ignores free
 */

void *malloc(size_t size)
{
	void *p = malloc_pointer;

	malloc_pointer += (size & ~3) + 4;

	if (((u8 *)malloc_pointer - &malloc_pool[0]) > sizeof(malloc_pool)) {
		puts("Ran out of malloc pool\n");
		while (1)
			;
	}

	return p;
}

void free(void *ptr)
{
}

int q;

void udelay(int n)
{
	while (n--)
		q+=n * q;
}
