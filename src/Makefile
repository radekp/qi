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

include ../config.mk

LDS	= kboot-stage1.lds
INCLUDE	= ../include
IMAGE	= ../image
CFLAGS	= -Wall -I $(INCLUDE) -g -c -o

START	= start.o lowlevel_init.o
COBJS	= start_kboot.o nand_read.o

SRCS	:= $(START: .o=.S) $(COBJS: .o=.c)

all:start.S lowlevel_init.S nand_read.c start_kboot.c
	$(CC) $(CFLAGS) start.o start.S
	$(CC) $(CFLAGS) lowlevel_init.o lowlevel_init.S
	$(CC) $(CFLAGS) nand_read.o nand_read.c
	$(CC) $(CFLAGS) start_kboot.o start_kboot.c
	$(LD) -T$(LDS) -g $(START) $(COBJS) -o start_kboot_all.o
	$(OBJCOPY) -O binary -S start_kboot_all.o $(IMAGE)/start 

blink_led:led_on.S
	$(CC) $(CFLAGS) led_on.o led_on.S
	$(LD) -g led_on.o -o led_on_temp.o
	$(OBJCOPY) -O binary -S led_on_temp.o $(IMAGE)/led_on 

clean:
	rm -f *.o
	rm -f $(IMAGE)/*

