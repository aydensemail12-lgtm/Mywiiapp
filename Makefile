# Project settings
TARGET := boot
BUILD := build
SOURCE := source
INCLUDE := include

# devkitPro paths
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=/opt/devkitpro/devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

# Compiler flags
CFLAGS := -g -O2 -Wall $(MACHDEP) -I$(INCLUDE)
CXXFLAGS := $(CFLAGS)

# Libraries
LIBS := -lwiiuse -lbte -logc -lm -lfat

# Objects
SOURCES := $(wildcard $(SOURCE)/*.c)
OFILES := $(SOURCES:.c=.o)

.PHONY: all clean

all: $(TARGET).dol

$(TARGET).dol: $(TARGET).elf
$(TARGET).elf: $(OFILES)

clean:
	rm -rf $(TARGET).elf $(TARGET).dol $(OFILES)
