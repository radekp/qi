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
#include "blink_led.h"

int delay(int time)
{
  int i=0;
  for(i=0;i<time;i++);
  return 0;
}
int set_GPB()
{
  GPBCON = 0x5;
  GPBDW = 0xffff;
  return 0;
}

int orange_on(int times)
{
  int count=0;
  set_GPB();

  for(count=0;count<times;count++)
    {
      ORANGE_ON();
      delay(0xfffff);
      ORANGE_OFF() ;
      delay(0xfffff);
    }

  return 0;
}

int blue_on(int times)
{
  int count=0;
  set_GPB();

  for(count=0;count<times;count++)
    {
      BLUE_ON();
      delay(0xfffff);
      BLUE_OFF();
      delay(0xfffff);
    }

  return 0;
}

int blink_led(void)
{
  set_GPB();

  while(1)
    {
      ORANGE_ON();
      delay(0xfffff);
      ORANGE_OFF() ;
      delay(0xfffff);

      BLUE_ON();
      delay(0xfffff);
      BLUE_OFF();
      delay(0xfffff);
    }

  return 0;
}


