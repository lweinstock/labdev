###
#
#	Example makefile
#
###

BIN=tfmeas
CC=g++
CFLAGS=-Wall -g --std=c++11

LABDEV_INC=../../../labdev/inc
LABDEV_LIB=../../../labdev
LDFLAGS=-L$(LABDEV_LIB) -llabdev -lusb-1.0

### --- VISA SETUP --- ###

# VISA support: compile with/without VISA (0/1)
NO_VISA=0
ifeq ($(NO_VISA), 0)  # Compile with VISA

	UNAME=$(shell uname)
	# MACOS setup

	ifeq ($(UNAME),Darwin)
		VISA_LIB=/Library/Frameworks/RsVisa.framework/Versions/A/RsVisa
		VISA_INC=/Library/Frameworks/RsVisa.framework/Versions/A/Headers
	endif

	# Linux setup
	ifeq ($(UNAME),Linux)
		VISA_LIB=/usr/lib/x86_64-linux-gnu
		VISA_INC=/usr/include/rsvisa
	endif

	# Add lib and inc paths to linker and compiler flags
	LDFLAGS+=-L$(VISA_LIB) -lrsvisa
	CFLAGS+=-I$(VISA_INC)

else
	CFLAGS+=-D NO_VISA
endif

### --- END VISA SETUP --- ###

.PHONY: all clean

all: $(BIN)

%.o: %.cpp Makefile
	$(CC) -c $(CFLAGS) -I$(LABDEV_INC) $<

$(BIN): $(OBJ) $(BIN).o
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf *.o
	rm -rf $(BIN)
