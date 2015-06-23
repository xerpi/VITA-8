#
# Copyright (c) 2015 Sergi Granell (xerpi)
# based on Cirne's vita-toolchain test Makefile
#

TARGET		:= VITA-8
SOURCES		:= source
DATA		:= data
INCLUDES	:= include

STUBS = libSceLibKernel.a libSceDisplay.a libSceGxm.a libSceSysmem.a \
	libSceCtrl.a libSceTouch.a

NIDS_DB = sample-db.json

CFILES   := $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.c))
BINFILES := $(foreach dir,$(DATA), $(wildcard $(dir)/*.bin))
OBJS     := $(addsuffix .o,$(BINFILES)) $(CFILES:.c=.o)

PREFIX  = $(DEVKITARM)/bin/arm-none-eabi
AS	= $(PREFIX)-as
CC      = $(PREFIX)-gcc
READELF = $(PREFIX)-readelf
OBJDUMP = $(PREFIX)-objdump
CFLAGS  = -Wall -nostartfiles -nostdlib -I$(PSP2SDK)/include -I$(INCLUDES) -I$(DATA)

STUBS_FULL = $(addprefix stubs/, $(STUBS))

all: $(TARGET).velf

$(TARGET).velf: $(TARGET).elf
	vita-elf-create $(TARGET).elf $(TARGET).velf $(NIDS_DB)
#	$(READELF) -a $(TARGET).velf
#	$(OBJDUMP) -D -j .text.fstubs $(TARGET).velf
#	$(OBJDUMP) -s -j .data.vstubs -j .sceModuleInfo.rodata -j .sceLib.ent -j .sceExport.rodata -j .sceLib.stubs -j .sceImport.rodata -j .sceFNID.rodata -j .sceFStub.rodata -j .sceVNID.rodata -j .sceVStub.rodata -j .sce.rel $(TARGET).velf

$(TARGET).elf: $(OBJS) $(STUBS_FULL)
	$(CC) -Wl,-q -o $@ $^ $(CFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(STUBS_FULL):
	@mkdir -p stubs
	vita-libs-gen $(NIDS_DB) stubs/
	$(MAKE) -C stubs $(notdir $@)

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
	@rm -rf $(TARGET).elf $(TARGET).velf $(OBJS) stubs $(DATA)/*.h

copy: $(TARGET).velf
	@cp $(TARGET).velf ~/shared/vitasample.elf

