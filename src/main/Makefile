TARGET = main
OPTIONS = -DF_CPU=48000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH

TOPDIR = $(HOME)/repo/arduino/arduino-1.0.5-teensy
HWDIR = $(TOPDIR)/hardware

TOOLSPATH = $(HWDIR)/tools

LIBRARYPATH = $(TOPDIR)/libraries

COMPILERPATH = $(HWDIR)/tools/arm-none-eabi/bin

CPPFLAGS = -Wall -g -Os -mcpu=cortex-m4 -mthumb -nostdlib -MMD $(OPTIONS)

CXXFLAGS = -std=c++0x -felide-constructors -fno-exceptions -fno-rtti -Icore

CFLAGS = -Icore -I$(HOME)/repo/teensy3/src/common

LDFLAGS = -Os -Wl,--gc-sections -mcpu=cortex-m4 -mthumb -Tcore/mk20dx128.ld

LIBS = -lm

# names for the compiler programs
CC = $(abspath $(COMPILERPATH))/arm-none-eabi-gcc
CXX = $(abspath $(COMPILERPATH))/arm-none-eabi-g++
OBJCOPY = $(abspath $(COMPILERPATH))/arm-none-eabi-objcopy
SIZE = $(abspath $(COMPILERPATH))/arm-none-eabi-size

C_FILES := $(wildcard *.c) $(wildcard core/*.c)
CPP_FILES := $(wildcard *.cpp) $(wildcard core/*.cpp)
OBJS := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)

all: $(TARGET).hex

$(TARGET).elf: $(OBJS) core/mk20dx128.ld
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.hex: %.elf
	$(SIZE) $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@

program: $(TARGET).hex
	sudo /home/texane/install/bin/teensy3_loader -v -mmcu=mk20dx128 main.hex

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	rm -f *.o *.d core/*.o core/*.d $(TARGET).elf $(TARGET).hex
