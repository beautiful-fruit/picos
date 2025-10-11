CC = /opt/microchip/xc8/v3.10/bin/xc8-cc
BUILD = build
SRC = src
CFLAGS = -mcpu=18F4520 -mdfp=`pwd`/dfp/xc8 -Wl,-Map=$(BUILD)/main.map -Iinclude

SRCS = src/hal.c src/main.c src/libc.c src/interrupt.c src/schedule.c
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

clean:
	rm $(BUILD)/*
