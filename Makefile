
include $(DEVKITARM)/3ds_rules

TARGET := MM3DRandoManager
SOURCES := source
DATA := data
INCLUDES := include

CFLAGS := -O2 -Wall -march=armv6k
LIBS := -lctru -lm

all: $(TARGET).cia

$(TARGET).cia: $(TARGET).elf
	makerom -f cia -o $@ -elf $< -rsf app.rsf

clean:
	rm -rf build *.cia *.elf
