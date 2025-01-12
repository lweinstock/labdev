###
#
#	Example makefile
#
###

BIN=tfmeas
CC=g++
CFLAGS=-Wall -g
LDFLAGS=

# labdev setup
CFLAGS+=$(shell pkg-config --cflags liblabdev)
LDFLAGS+=$(shell pkg-config --libs liblabdev)

.PHONY: all clean

all: $(BIN)

%.o: %.cpp Makefile
	$(CC) -c $(CFLAGS) $<

$(BIN): $(OBJ) $(BIN).o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf *.o
	rm -rf $(BIN)
