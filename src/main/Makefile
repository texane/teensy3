TARGET = main
OPTIONS = -DF_CPU=48000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH
OPTIONS += -D__MK20DX256__ -DARDUIO=104

TOPDIR = $(HOME)/repo/arduino/arduino-1.0.5
HWDIR = $(TOPDIR)/hardware
COREDIR = $(HOME)/repo/teensy3/src/core

TOOLSPATH = $(HWDIR)/tools

LIBRARYPATH = $(TOPDIR)/libraries

COMPILERPATH = $(HWDIR)/tools/arm-none-eabi/bin

CPPFLAGS = -Wall -g -Os -mcpu=cortex-m4 -mthumb -nostdlib -MMD $(OPTIONS)

CXXFLAGS = -std=c++0x -felide-constructors -fno-exceptions -fno-rtti -I$(COREDIR)

CFLAGS = -I$(COREDIR) -I$(HOME)/repo/teensy3/src/common

LDFLAGS = -Os -Wl,--gc-sections -mcpu=cortex-m4 -mthumb -T$(COREDIR)/mk20dx256.ld

LIBS = -lm

# names for the compiler programs
CC = $(abspath $(COMPILERPATH))/arm-none-eabi-gcc
CXX = $(abspath $(COMPILERPATH))/arm-none-eabi-g++
OBJCOPY = $(abspath $(COMPILERPATH))/arm-none-eabi-objcopy
SIZE = $(abspath $(COMPILERPATH))/arm-none-eabi-size

C_FILES := $(wildcard *.c) $(wildcard $(COREDIR)/*.c)
CPP_FILES := $(wildcard *.cpp) $(wildcard $(COREDIR)/*.cpp)
OBJS := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)

all: $(TARGET).hex

$(TARGET).elf: $(OBJS) $(COREDIR)/mk20dx256.ld
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.hex: %.elf
	$(SIZE) $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@

program: $(TARGET).hex
	sudo $(HOME)/repo/teensy3/src/teensy_loader_cli/teensy_loader_cli -v -mmcu=mk20dx256 main.hex

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	rm -f *.o *.d $(COREDIR)/*.o $(COREDIR)/*.d $(TARGET).elf $(TARGET).hex
