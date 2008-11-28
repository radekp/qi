/*
 * (C) Copyright 2008 Openmoko, Inc.
 * Author: Andy Green <andy@openmoko.org>
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

#ifndef __QI_H__
#define __QI_H__

#include <stdarg.h>
#include <qi-serial.h>
#include <qi-ctype.h>

#define u32 unsigned int
#define u16 unsigned short
#define u8 unsigned char
typedef unsigned int uint32_t;

int printk(const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list args);
int puts(const char *string);
void printhex(unsigned char v);
void print32(unsigned int u);
void hexdump(unsigned char *start, int len);
unsigned int _ntohl(unsigned int n);

#endif

