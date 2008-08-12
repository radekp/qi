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

BUILD_DATE := $(shell date)
BUILD_HOST := $(shell hostname)
BUILD_BRANCH := $(shell git branch | grep ^\* | cut -d' ' -f2)
BUILD_HEAD := $(shell git show --pretty=oneline | head -n1 | cut -d' ' -f1 | cut -b1-16)
BUILD_VERSION := ${BUILD_BRANCH}_${BUILD_HEAD}

LDS	= src/kboot-stage1.lds
INCLUDE	= include
IMAGE_DIR	= image
CFLAGS	= -Wall -Werror -I $(INCLUDE) -g -c -O2 -fno-strict-aliasing \
	  -fno-common -ffixed-r8 -msoft-float -fno-builtin -ffreestanding \
	  -march=armv4t -mno-thumb-interwork -Wstrict-prototypes \
	  -DBUILD_HOST="${BUILD_HOST}" -DBUILD_VERSION="${BUILD_VERSION}" \
	  -DBUILD_DATE="${BUILD_DATE}"
LDFLAGS = 
#START	= start.o lowlevel_init.o
S_SRCS	= src/start.S src/lowlevel_init.S
S_OBJS	= $(patsubst %.S,%.o, $(S_SRCS))
C_SRCS	= $(wildcard src/*.c)
C_OBJS	= $(patsubst %.c,%.o, $(C_SRCS))

#SRCS	:= $(START: .o=.S) $(COBJS: .o=.c)
SRCS	= ${S_SRCS} ${C_SRCS}
OBJS	= ${S_OBJS} ${C_OBJS}
LIBS	= -L${COMPILER_LIB_PATH} -lgcc

# GTA02 A5 and A6 U-Boot will eat these for DFU action
UDFU_VID = 0x1d50
UDFU_PID = 0x5119
UDFU_REV = 0x350

TARGET	= src/start_kboot_all
IMAGE = $(IMAGE_DIR)/kboot
UDFU_IMAGE = $(IMAGE_DIR)/kboot.udfu

%.o: %.S
	@$(CC) $(CFLAGS) -o $@ $<

%.o: %.c
	@$(CC) $(CFLAGS) -o $@ $<

all:${UDFU_IMAGE}

${OBJS}:${SRCS}

${UDFU_IMAGE}:${OBJS}
	$(LD) ${LDFLAGS} -T$(LDS) -g $(OBJS) -o ${TARGET} ${LIBS}
	$(OBJCOPY) -O binary -S ${TARGET} ${IMAGE}
	$(MKUDFU) -v ${UDFU_VID} -p ${UDFU_PID} -r ${UDFU_REV} \
						-d ${IMAGE} ${UDFU_IMAGE}
	$(OBJDUMP) -D ${TARGET} >${IMAGE}.dis

blink_led:src/led_on.S
	$(CC) $(CFLAGS) led_on.o led_on.S
	$(LD) -g led_on.o -o led_on_temp.o
	$(OBJCOPY) -O binary -S led_on_temp.o $(IMAGE)/led_on 

clean:
	rm -f src/*.o  src/*~ include/*~ ${IMAGE}* ${TARGET} ${UDFU_IMAGE}
