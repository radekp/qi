#
# Include the make variables (CC, etc...)
#

CROSS_PATH=/usr/local/openmoko/arm
CROSS_COMPILE=${CROSS_PATH}/bin/arm-angstrom-linux-gnueabi-

####
COMPILER_LIB_PATH_PRE=${CROSS_PATH}/lib/gcc/arm-angstrom-linux-gnueabi
COMPILER_LIB_PATH=${COMPILER_LIB_PATH_PRE}/`ls ${COMPILER_LIB_PATH_PRE}`

AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
HOSTCC	= gcc

# we need the mkudfu tool from U-Boot build
#MKUDFU = ../uboot/u-boot/tools/mkudfu

export CROSS_COMPILE AD LD CC OBJCOPY OBJDUMP MKUDFU
