/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: xiangfu liu <xiangfu@openmoko.org>
 *
 * Configuation settings for the FIC Neo GTA02 Linux GSM phone
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
#include "blink_led.h"

void serial_init (const int uart, const int ubrdiv_val)
{
	switch(uart)
	{
	case UART0:
		rULCON0 = 0x3;
		rUCON0 = 0x245;
		rUFCON0 = 0x0;
		rUMCON0 = 0x0;
		rUBRDIV0 = ubrdiv_val;
		break;
	case UART1:
		rULCON1 = 0x3;
		rUCON1 = 0x245;
		rUFCON1 = 0x0;
		rUMCON1 = 0x0;
		rUBRDIV1 = ubrdiv_val;
		break;
	case UART2:
		rULCON2 = 0x3;
		rUCON2 = 0x245;
		rUFCON2 = 0x1;
		rUBRDIV2 = ubrdiv_val;
		break;
	default:
		break;
	}
}
/*
 * Output a single byte to the serial port.
 */
void serial_putc (const int uart, const char c)
{
	switch(uart)
	{
	case UART0:
		while ( !( rUTRSTAT0 & 0x2 ) );
		WrUTXH0(c);
		break;
	case UART1:
		while ( !( rUTRSTAT1 & 0x2 ) );
		WrUTXH1(c);
		break;
	case UART2:
		while ( !( rUTRSTAT2 & 0x2 ) );
		WrUTXH2(c);
		break;
	default:
		break;
	}
}
