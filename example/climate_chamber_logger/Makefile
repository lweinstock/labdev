###
#
#	Example makefile
#
###

BIN=ccl
CC=g++
CFLAGS=-Wall -g --std=c++17
LDFLAGS=

# labdev setup
CFLAGS+=$(shell pkg-config liblabdev --cflags)
LDFLAGS+=$(shell pkg-config liblabdev --libs)

# opencv setup
CFLAGS+=$(shell pkg-config liblabdev --libs)
LDFLAGS+=$(shell pkg-config opencv --libs)

.PHONY: all clean

all: $(BIN)

%.o: %.cpp Makefile
	$(CC) -c $(CFLAGS) $<

$(BIN): $(OBJ) $(BIN).o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf *.o
	rm -rf $(BIN)
