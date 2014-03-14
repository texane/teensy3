
# The name of your project (used to name the compiled .hex file)
TARGET = main

# configurable options
OPTIONS = -DF_CPU=48000000 -DUSB_SERIAL -DLAYOUT_US_ENGLISH


#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

TOPDIR = $(HOME)/repo/arduino/arduino-1.0.5-teensy
HWDIR = $(TOPDIR)/hardware

# path location for Teensy Loader, teensy_post_compile and teensy_reboot
TOOLSPATH = $(HWDIR)/tools   # on Linux
#TOOLSPATH = ../../../tools/avr/bin   # on Mac or Windows

# path location for Arduino libraries (currently not used)
LIBRARYPATH = $(TOPDIR)/libraries

# path location for the arm-none-eabi compiler
COMPILERPATH = $(HWDIR)/tools/arm-none-eabi/bin

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -g -Os -mcpu=cortex-m4 -mthumb -nostdlib -MMD $(OPTIONS)

# compiler options for C++ only
CXXFLAGS = -std=c++0x -felide-constructors -fno-exceptions -fno-rtti -Icore

# compiler options for C only
CFLAGS = -Icore

# linker options
LDFLAGS = -Os -Wl,--gc-sections -mcpu=cortex-m4 -mthumb -Tcore/mk20dx128.ld

# additional libraries to link
LIBS = -lm


# names for the compiler programs
CC = $(abspath $(COMPILERPATH))/arm-none-eabi-gcc
CXX = $(abspath $(COMPILERPATH))/arm-none-eabi-g++
OBJCOPY = $(abspath $(COMPILERPATH))/arm-none-eabi-objcopy
SIZE = $(abspath $(COMPILERPATH))/arm-none-eabi-size

# automatically create lists of the sources and objects
# TODO: this does not handle Arduino libraries yet...
C_FILES := $(wildcard *.c) $(wildcard core/*.c)
CPP_FILES := $(wildcard *.cpp) $(wildcard core/*.cpp)
OBJS := $(C_FILES:.c=.o) $(CPP_FILES:.cpp=.o)


# the actual makefile rules (all .o files built by GNU make's default implicit rules)

all: $(TARGET).hex

$(TARGET).elf: $(OBJS) core/mk20dx128.ld
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

%.hex: %.elf
	$(SIZE) $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@

program: $(TARGET).hex
	(cd ../loader && ./do_run.sh ../main/main.hex )
#	$(abspath $(TOOLSPATH))/teensy_post_compile -file=$(basename $@) -path=$(shell pwd) -tools=$(abspath $(TOOLSPATH))/
#	-$(abspath $(TOOLSPATH))/teensy_reboot


# compiler generated dependency info
-include $(OBJS:.o=.d)

clean:
	rm -f *.o *.d core/*.o core/*.d $(TARGET).elf $(TARGET).hex


