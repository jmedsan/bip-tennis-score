# TennisScore Makefile for Linux/Fedora WSL build
# Mirrors build.bat workflow for ARM Cortex-M4 compilation

# Project name (derived from directory)
PROGRAM_NAME = TennisScore

# ARM Embedded Toolchain Configuration
TOOLCHAIN_PREFIX = arm-none-eabi
CPU_FLAGS = -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard
COMPILE_FLAGS = -c -Os -Wall -fpie -pie -fpic -mthumb -mlittle-endian \
                -ffunction-sections -fdata-sections -Wa,-R
LINK_FLAGS = -pie -N -Os --cref --gc-sections -EL
LIBS = -lm -lc

# Compiler and tools
CC = $(TOOLCHAIN_PREFIX)-gcc
LD = $(TOOLCHAIN_PREFIX)-ld
AR = $(TOOLCHAIN_PREFIX)-ar
OBJCOPY = $(TOOLCHAIN_PREFIX)-objcopy
NM = $(TOOLCHAIN_PREFIX)-nm

# Library paths
LIB_BIP = ../libbip/libbip.a
LIB_PATH = -L../libbip

# Source files
C_SOURCES = $(wildcard *.c)
OBJECTS = $(C_SOURCES:.c=.o)

# Output file
OUTPUT = $(PROGRAM_NAME).elf
MAP_FILE = $(PROGRAM_NAME).map

# Default target
all: $(OUTPUT)

# Compile C files to object files
%.o: %.c
	$(CC) $(CPU_FLAGS) $(COMPILE_FLAGS) -I../libbip -I. -o $@ $<

# Link object files and libraries
$(OUTPUT): $(OBJECTS)
	$(LD) -Map $(MAP_FILE) -o $(OUTPUT) $(OBJECTS) $(LINK_FLAGS) $(LIB_PATH) $(LIB_BIP) $(LIBS)
	@if [ -f label.txt ]; then \
		$(OBJCOPY) $(OUTPUT) --add-section .elf.label=label.txt; \
	fi
	@echo "$(PROGRAM_NAME)" > name.txt
	$(OBJCOPY) $(OUTPUT) --add-section .elf.name=name.txt
	@rm -f name.txt

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(OUTPUT) $(MAP_FILE)

# Phony targets
.PHONY: all clean
