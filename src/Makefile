#
# Makefile
#

CIRCLEBASE = ../../circle-stdlib
include $(CIRCLEBASE)/Config.mk

CIRCLEHOME = $(CIRCLEBASE)/libs/circle
NEWLIBDIR = $(CIRCLEBASE)/install/$(NEWLIB_ARCH)

OBJS	= main.o kernel.o mandel-driver.o fake-posix.o 


ifeq ($(kernel), rad)
RADOBJ = rad/lowlevel_arm64.o rad/lowlevel_dma.o rad/helpers.o rad/gpio_defs.o \
		 rad/config.o rad/dirscan.o rad/rad_main.o rad/rad_hijack.o 
OBJS += $(RADOBJ)
CPPFLAGS += -DRAD
endif

ifeq ($(kernel), skvic20)
CPPFLAGS += -DCOMPILE_MENU=1 -DSIDEKICK20
OLED += OLED/ssd1306xled.o OLED/num2str.o
SKVIC20 = sk20/kernel_menu20.o sk20/vic20config.o sk20/vic20screen.o sk20/lowlevel_arm64.o sk20/gpio_defs.o sk20/latch.o sk20/helpers.o \
		  sk20/oled.o sk20/disk_emulation.o sk20/dirscan.o sk20/crt.o sk20/mempool.o \
		  #sk20/tft_st7789.o 

#mygpiopinfiq.o sound.o 
OBJS += $(SKVIC20) $(OLED)

CPPFLAGS += -DSKVIC20

#CPPFLAGS += -DCOMPILE_MENU_WITH_SOUND=1
#CPPFLAGS += -DUSE_VCHIQ_SOUND=$(USE_VCHIQ_SOUND) 
endif

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
