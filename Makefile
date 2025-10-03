CC = /opt/microchip/xc8/v3.10/bin/xc8-cc
BUILD = build
SRC = src
CFLAGS = -mcpu=18F4520 -mdfp=`pwd`/dfp/xc8 -Wl,-Map=$(BUILD)/main.map -Iinclude

SRCS = src/hal.c src/main.c src/libc.c
OBJS := $(patsubst src/%.c,$(BUILD)/%.p1,$(SRCS))


all: build $(BUILD)/main.elf

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.p1: src/%.c
	$(CC) -c $< $(CFLAGS)
	@rm -f $*.d
	@mv $(notdir $(basename $<)).p1 $@


# $(BUILD)/hal.p1: $(SRC)/hal.c
# 	$(CC) -c $(SRC)/hal.c $(CFLAGS)
# 	rm hal.d
# 	mv hal.p1 $(BUILD)/hal.p1

$(BUILD)/main.elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(BUILD)/main.elf

clean:
	rm $(BUILD)/*
