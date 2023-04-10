#
# Makefile
#

CIRCLEBASE = ../../circle-stdlib
include $(CIRCLEBASE)/Config.mk

CIRCLEHOME = $(CIRCLEBASE)/libs/circle
NEWLIBDIR = $(CIRCLEBASE)/install/$(NEWLIB_ARCH)

OBJS	= main.o kernel.o mandel-driver.o fake-posix.o \
		  rad/lowlevel_arm64.o rad/lowlevel_dma.o rad/helpers.o rad/gpio_defs.o \
		  rad/config.o rad/dirscan.o rad/rad_main.o rad/rad_hijack.o 
#include $(CIRCLEHOME)/Rules.mk
include Circle/Rules.mk

ADDONINC = -I $(CIRCLEHOME)/addon/linux

CFLAGS += -D__CIRCLE__ -I. -I./Circle -I "$(NEWLIBDIR)/include" -I $(STDDEF_INCPATH) -I $(CIRCLEBASE)/include $(ADDONINC) 
LIBS := $(NEWLIBDIR)/lib/libm.a $(NEWLIBDIR)/lib/libc.a $(NEWLIBDIR)/lib/libcirclenewlib.a \
 	$(CIRCLEHOME)/addon/linux/liblinuxemu.a \
 	$(CIRCLEHOME)/addon/SDCard/libsdcard.a \
  	$(CIRCLEHOME)/lib/usb/libusb.a \
 	$(CIRCLEHOME)/lib/input/libinput.a \
 	$(CIRCLEHOME)/addon/fatfs/libfatfs.a \
 	$(CIRCLEHOME)/lib/fs/libfs.a \
  	$(CIRCLEHOME)/lib/net/libnet.a \
  	$(CIRCLEHOME)/lib/sched/libsched.a \
  	$(CIRCLEHOME)/lib/libcircle.a

TOCLEAN = ${OBJS:.o=.d} *.img *.lst *.map *.elf
clean::
	@rm -f $(TOCLEAN) $(OBJS)

-include $(DEPS)
