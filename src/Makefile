#
# Makefile
#

CIRCLEBASE = ../../circle-stdlib
include $(CIRCLEBASE)/Config.mk

CIRCLEHOME = $(CIRCLEBASE)/libs/circle
NEWLIBDIR = $(CIRCLEBASE)/install/$(NEWLIB_ARCH)

OBJS	= main.o kernel.o mandel-driver.o fake-posix.o

include $(CIRCLEHOME)/Rules.mk

ADDONINC = -I $(CIRCLEHOME)/addon/linux

CFLAGS += -D__CIRCLE__ -I "$(NEWLIBDIR)/include" -I $(STDDEF_INCPATH) -I $(CIRCLEBASE)/include $(ADDONINC) 
LIBS := "$(NEWLIBDIR)/lib/libm.a" "$(NEWLIBDIR)/lib/libc.a" "$(NEWLIBDIR)/lib/libcirclenewlib.a" \
 	$(CIRCLEHOME)/addon/linux/liblinuxemu.a \
 	$(CIRCLEHOME)/addon/SDCard/libsdcard.a \
  	$(CIRCLEHOME)/lib/usb/libusb.a \
 	$(CIRCLEHOME)/lib/input/libinput.a \
 	$(CIRCLEHOME)/addon/fatfs/libfatfs.a \
 	$(CIRCLEHOME)/lib/fs/libfs.a \
  	$(CIRCLEHOME)/lib/net/libnet.a \
  	$(CIRCLEHOME)/lib/sched/libsched.a \
  	$(CIRCLEHOME)/lib/libcircle.a

-include $(DEPS)
