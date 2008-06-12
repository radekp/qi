# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#



TEXT_BASE	=0x0000000

INCLUDE		=../include
IMAGE		=../image
CFLAGS		=-I $(INCLUDE) -g -c -o

all:led.S led.c
	$(CC) $(CFLAGS) led_S.o led.S
	$(CC) $(CFLAGS) led_C.o led.c
	$(LD) -Ttext $(TEXT_BASE) -g led_S.o led_C.o  -o led.o
	$(OBJCOPY) -O binary -S led.o $(IMAGE)/led_on 

clean:
	rm -f *.o
	rm -f led_on
	rm -f $(IMAGE)/*

