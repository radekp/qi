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
#define GPBCON        (*(volatile unsigned *)0x56000010)
#define GPBDAT        (*(volatile unsigned *)0x56000014)
#define GPBDW         (*(volatile unsigned *)0x56000018)   
#define LED3_ON()    (GPBDAT &= ~(0x1))  
#define LED4_ON()    (GPBDAT &= ~(0x2))   
#define LED3_OFF()     (GPBDAT |= (0x1))
#define LED4_OFF()     (GPBDAT |= (0x2))

int delay(int time)
{
	int i=0;
	for(i=0;i<time;i++);
	return 0;
}

int blink_led()
{
    GPBCON = 0x5;
    GPBDW = 0xffff;
    while(1)
    {
	LED3_ON();
	delay(0xfffff);
	LED3_OFF() ;
	delay(0xfffff);

	LED4_ON();
	delay(0xfffff);
	LED4_OFF();
	delay(0xfffff);
    }
    return 0;
}
