###
#
#   Example makefile for HX711 Arduino reader
#
###

BIN := lcreadout
CC  := g++

# Sources: example app + the HX711 device shim
SRC := lcreadout.cpp #../../src/devices/lc_hx711/hx711_arduino.cpp
OBJ := $(SRC:.cpp=.o)

CFLAGS  := -Wall -g --std=c++17
LDFLAGS :=

# labdev setup via pkg-config
LABDEV_CFLAGS := $(shell pkg-config liblabdev --cflags)
LABDEV_LDFLAGS := $(shell pkg-config liblabdev --libs)
CFLAGS += $(LABDEV_CFLAGS)
LDFLAGS += $(LABDEV_LDFLAGS)

.PHONY: all clean

all: $(BIN)

%.o: %.cpp Makefile
	$(CC) -c $(CFLAGS) $< -o $@

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm -rf $(OBJ)
	rm -rf $(BIN)
