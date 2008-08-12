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

include config.mk

LDS	= src/kboot-stage1.lds
INCLUDE	= include
IMAGE_DIR	= image
CFLAGS	= -Wall -Werror -I $(INCLUDE) -g -c 
LDFLAGS = 
#START	= start.o lowlevel_init.o
S_SRCS	= src/start.S src/lowlevel_init.S
S_OBJS	= $(patsubst %.S,%.o, $(S_SRCS))
C_SRCS	= $(wildcard src/*.c)
C_OBJS	= $(patsubst %.c,%.o, $(C_SRCS))

#SRCS	:= $(START: .o=.S) $(COBJS: .o=.c)
SRCS	= ${S_SRCS} ${C_SRCS}
OBJS	= ${S_OBJS} ${C_OBJS}

TARGET	= src/start_kboot_all
IMAGE = $(IMAGE_DIR)/start

%.o: %.S
	@$(CC) $(CFLAGS) -o $@ $<

%.o: %.c
	@$(CC) $(CFLAGS) -o $@ $<

all:${TARGET}

${OBJS}:${SRCS}

${TARGET}:${OBJS}
	$(LD) ${LDFLAGS} -T$(LDS) -g $(OBJS) -o ${TARGET}  
	$(OBJCOPY) -O binary -S ${TARGET} ${IMAGE}
	$(OBJDUMP) -D ${TARGET} >${IMAGE}.dis

blink_led:src/led_on.S
	$(CC) $(CFLAGS) led_on.o led_on.S
	$(LD) -g led_on.o -o led_on_temp.o
	$(OBJCOPY) -O binary -S led_on_temp.o $(IMAGE)/led_on 

clean:
	rm -f src/*.o  src/*~ include/*~ ${IMAGE}* ${TARGET}
