###
#
#	Example makefile
#
###

BIN=tfmeas
CC=g++
CFLAGS=-Wall -g --std=c++11
LDFLAGS=

# labdev setup
LABDEV_CFLAGS=$(shell pkg-config liblabdev --cflags)
LABDEV_LDFLAGS=$(shell pkg-config liblabdev --libs)
CFLAGS+=$(LABDEV_CFLAGS)
LDFLAGS+=$(LABDEV_LDFLAGS)

.PHONY: all clean

all: $(BIN)

%.o: %.cpp Makefile
	$(CC) -c $(CFLAGS) $<

$(BIN): $(OBJ) $(BIN).o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf *.o
	rm -rf $(BIN)
