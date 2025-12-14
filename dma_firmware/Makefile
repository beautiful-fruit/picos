CC = /opt/microchip/xc8/v3.10/bin/xc8-cc
BUILD = build
SRC = src
CFLAGS = -mcpu=18F4520 -mdfp=`pwd`/../dfp/xc8 -Wl,-Map=$(BUILD)/main.map -Iinclude

SRCS = src/hal.c src/main.c src/libc.c src/ch375.c \
		src/tests.c src/debug.c src/dma.c src/interrupt.c
OBJS := $(patsubst src/%.c,$(BUILD)/%.p1,$(SRCS))

INTERNAL_CLOCK ?= 0

ifeq ($(INTERNAL_CLOCK),0)
    CFLAGS += -DEXTERNAL_CLOCK
endif

all: build $(BUILD)/main.elf

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.p1: src/%.c
	$(CC) -c $< $(CFLAGS)
	@rm -f $*.d
	@mv $(notdir $(basename $<)).p1 $@


$(BUILD)/main.elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILD)/main.elf
flash:
	./parse_config.py $(BUILD)/main.hex pic18f4520.fuses.conf
	minipro -p 'PIC18F4520@DIP40' -i -E
	minipro -p 'PIC18F4520@DIP40' -i -w pic18f4520.fuses.conf -c config
	minipro -p 'PIC18F4520@DIP40' -i -e -w build/main.hex --format ihex

clean:
	rm $(BUILD)/*
