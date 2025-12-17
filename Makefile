CC = /opt/microchip/xc8/v3.10/bin/xc8-cc
BUILD = build
SRC = src
CFLAGS = -mcpu=18F4520 -mdfp=`pwd`/dfp/xc8 -Wl,-Map=$(BUILD)/main.map -Iinclude -O2

SRCS = src/hal.c src/main.c src/libc.c src/interrupt.c src/schedule.c src/ch375.c \
		src/debug.c src/dma.c src/usr_libc.c src/fat32.c
OBJS := $(patsubst src/%.c,$(BUILD)/%.p1,$(SRCS))

INTERNAL_CLOCK ?= 0

ifeq ($(INTERNAL_CLOCK),0)
    CFLAGS += -DEXTERNAL_CLOCK
endif

all: build $(BUILD)/main.elf dma

dma: dma_firmware
	make -C dma_firmware

$(BUILD):
	mkdir -p $(BUILD)

flash:
	./parse_config.py $(BUILD)/main.hex pic18f4520.fuses.conf
	./minipro -p 'PIC18F4520@DIP40' -i -E
	./minipro -p 'PIC18F4520@DIP40' -i -w pic18f4520.fuses.conf -c config
	./minipro -p 'PIC18F4520@DIP40' -i -e -w build/main.hex --format ihex


$(BUILD)/%.p1: src/%.c
	$(CC) -c $< $(CFLAGS)
	@rm -f $*.d
	@mv $(notdir $(basename $<)).p1 $@

$(BUILD)/isr.o: src/isr.s
	$(CC) -c src/isr.s $(CFLAGS)
	@rm -f $*.d
	@mv $(notdir $(basename $<)).o $@

$(BUILD)/keep_funcs.o: src/keep_funcs.s
	$(CC) -c src/keep_funcs.s $(CFLAGS)
	@rm -f $*.d
	@mv $(notdir $(basename $<)).o $@

$(BUILD)/main.elf: $(OBJS) $(BUILD)/isr.o $(BUILD)/keep_funcs.o
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILD)/main.elf $(BUILD)/isr.o $(BUILD)/keep_funcs.o

flash:
	./parse_config.py $(BUILD)/main.hex pic18f4520.fuses.conf
	minipro -p 'PIC18F4520@DIP40' -i -E
	minipro -p 'PIC18F4520@DIP40' -i -w pic18f4520.fuses.conf -c config
	minipro -p 'PIC18F4520@DIP40' -i -e -w build/main.hex --format ihex

flash-dma:
	./parse_config.py dma_firmware/$(BUILD)/main.hex pic18f4520.fuses.conf
	minipro -p 'PIC18F4520@DIP40' -i -E
	minipro -p 'PIC18F4520@DIP40' -i -w pic18f4520.fuses.conf -c config
	minipro -p 'PIC18F4520@DIP40' -i -e -w dma_firmware/$(BUILD)/main.hex --format ihex

clean:
	rm $(BUILD)/*
	make -C dma_firmware clean
