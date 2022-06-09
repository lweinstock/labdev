###
#
#   Makefile to compile liblabdev.a
#
###

UNAME=$(shell uname)
# VISA support: compile with/without VISA (1/0)
VISA=0

# General compiler settings/flags
CC=g++
LDFLAGS=
CFLAGS=-Wall -g --std=c++11
# Debugging
CFLAGS+=-g -D LD_DEBUG

# Library name and objects
SRC=src
INC=inc
LIBNAME=liblabdev.a
OBJ=

# libusb compiler flags
LIBUSB_CFLAGS=$(shell pkg-config libusb-1.0 --cflags)
CFLAGS+=$(LIBUSB_CFLAGS)

# Interfaces
OBJ+=$(SRC)/interface.o
OBJ+=$(SRC)/serial_interface.o
OBJ+=$(SRC)/tcpip_interface.o
OBJ+=$(SRC)/usb_interface.o
OBJ+=$(SRC)/usbtmc_interface.o

# Utilies
OBJ+=$(SRC)/utils/utils.o
OBJ+=$(SRC)/utils/config.o

# Basic devices
OBJ+=$(SRC)/devices/oscilloscope.o

# Vendor devices
OBJ+=$(SRC)/devices/scpi_device.o
OBJ+=$(SRC)/devices/feeltech/fy6900.o
OBJ+=$(SRC)/devices/uni-t/ut61b.o
OBJ+=$(SRC)/devices/rigol/ds1000z.o
OBJ+=$(SRC)/devices/rigol/dg4000.o
OBJ+=$(SRC)/devices/hantek/dso5000p.o
OBJ+=$(SRC)/devices/rohde-schwarz/hmp4000.o
OBJ+=$(SRC)/devices/rohde-schwarz/rta4000.o
OBJ+=$(SRC)/devices/tektronix/dpo5000b.o
OBJ+=$(SRC)/devices/baumer/om70_l.o
OBJ+=$(SRC)/devices/jenny-science/xenax_xvi_75v8.o
OBJ+=$(SRC)/devices/musashi/ml-808gx.o

### --- VISA SETUP --- ###
ifeq ($(VISA), 1)  # Compile with VISA

  # MACOS path (hard coded)
  ifeq ($(UNAME),Darwin)
    VISA_INC=/Library/Frameworks/RsVisa.framework/Versions/A/Headers
  endif

  # Linux path (hard coded)
  ifeq ($(UNAME),Linux)
    VISA_INC=/usr/include/rsvisa
  endif

  OBJ+=$(SRC)/visa_interface.o
  CFLAGS+=-I$(VISA_INC)

  CFLAGS+=-D LDVISA

else  # Compile without VISA

endif
### --- END VISA SETUP --- ###

# Install setup
PREFIX=
ifeq ($(PREFIX),)
	PREFIX:=/usr/local
endif

# pkg-config setup
PC_PATH=
ifeq ($(PC_PATH),)
	PC_PATH:=$(PREFIX)/lib/pkgconfig
endif

.PHONY: all clean install uninstall

all: $(LIBNAME)

%.o: %.cpp Makefile
	$(CC) -c -o $@ $< $(CFLAGS) -I$(SRC) -I$(INC)

$(LIBNAME): $(OBJ)
	ar -rc $@ $^
	ranlib $@

install: $(LIBNAME)
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include
	mkdir -p $(DESTDIR)$(PC_PATH)
	cp $(LIBNAME) $(DESTDIR)$(PREFIX)/lib/$(LIBNAME)
	cp -a $(INC)/ $(DESTDIR)$(PREFIX)/include/
	cp labdev.pc $(DESTDIR)$(PC_PATH)/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/$(LIBNAME)
	rm -rf $(DESTDIR)$(PREFIX)/include
	rm -f $(DESTDIR)$(PC_PATH)/labdev.pc
  
clean:
	rm -f $(OBJ)
	rm -f $(LIBNAME)
