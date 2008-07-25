#
# Include the make variables (CC, etc...)
#
#CROSS_COMPILE=arm-softfloat-linux-gnu-
CROSS_COMPILE=arm-angstrom-linux-gnueabi-

AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

export CROSS_COMPILE AD LD CC OBJCOPY OBJDUMP
