#
# Copyright (c) 2015 Sergi Granell (xerpi)
# based on Cirne's vita-toolchain test Makefile
#

TARGET		:= VITA-8
SOURCES		:= source
DATA		:= data
INCLUDES	:= include

LIBS = -lc_stub -lSceKernel_stub -lSceDisplay_stub -lSceGxm_stub	\
	-lSceCtrl_stub -lSceTouch_stub


CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o)

PREFIX  = $(DEVKITARM)/bin/arm-none-eabi
CC      = $(PREFIX)-gcc
AS      = $(PREFIX)-as
READELF = $(PREFIX)-readelf
OBJDUMP = $(PREFIX)-objdump
CFLAGS  = -Wall -specs=$(PSP2SDK)/psp2.specs -I$(INCLUDES) -I$(DATA)
ASFLAGS = $(CFLAGS)


all: $(TARGET)_fixup.elf

%_fixup.elf: %.elf
	psp2-fixup -q -S $< $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

.SUFFIXES: .bin

%.bin.o: %.bin
	$(bin2o)

define bin2o
	bin2s $< | $(AS) -o $(@)
	echo "extern const unsigned char" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"_end[];" > $(DATA)/`(echo $(<F) | tr . _)`.h
	echo "extern const unsigned char" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`"[];" >> $(DATA)/`(echo $(<F) | tr . _)`.h
	echo "extern const unsigned int" `(echo $(<F) | sed -e 's/^\([0-9]\)/_\1/' | tr . _)`_size";" >> $(DATA)/`(echo $(<F) | tr . _)`.h
endef

clean:
	@rm -rf $(TARGET)_fixup.elf $(TARGET).elf $(OBJS) $(DATA)/*.h

copy: $(TARGET)_fixup.elf
	@cp $(TARGET)_fixup.elf ~/shared/vitasample.elf
	@echo "Copied!"


