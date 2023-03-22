###
#
#   Makefile to compile liblabdev.a
#
###

# General compiler settings/flags
CC=g++
LDFLAGS=
CFLAGS=-Wall --std=c++11 -fPIC
# Debugging
CFLAGS+=-g #-D LD_DEBUG

# Library name and objects
LIBNAME=liblabdev
SRC=src
INC=inc
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
OBJ+=$(SRC)/devices/device.o
#OBJ+=$(SRC)/devices/oscilloscope.o

# Vendor specific devices
OBJ+=$(SRC)/devices/scpi_device.o
#OBJ+=$(SRC)/devices/feeltech/fy6900.o
#OBJ+=$(SRC)/devices/uni-t/ut61b.o
OBJ+=$(SRC)/devices/rigol/ds1000z.o
#OBJ+=$(SRC)/devices/rigol/dg4000.o
#OBJ+=$(SRC)/devices/hantek/dso5000p.o
#OBJ+=$(SRC)/devices/rohde-schwarz/hmp4000.o
#OBJ+=$(SRC)/devices/rohde-schwarz/rta4000.o
#OBJ+=$(SRC)/devices/tektronix/dpo5000b.o
OBJ+=$(SRC)/devices/baumer/om70_l.o
OBJ+=$(SRC)/devices/jenny-science/xenax_xvi.o
OBJ+=$(SRC)/devices/musashi/ml-808gx.o

###   INSTALL SETUP   ###

# Use PREFIX for local installation
PREFIX=
ifeq ($(PREFIX),)
  PREFIX:=/usr/local
endif

UNAME=$(shell uname)

# VISA support
VISA=
ifeq ($(VISA),1)
  OBJ+=$(SRC)/visa_interface.o
  CFLAGS+=-D LDVISA

  ifeq ($(UNAME),Darwin)  # macOS
    CFLAGS+=-F/Library/Frameworks
  else ifeq ($(UNAME),Linux)  # linux
    # nothing to do here (so far)
  endif
endif

# Generate pkg-config file
PC_CFLAGS=--std=c++11 -I$${includedir}
PC_LDFLAGS=-L$${libdir} -llabdev

ifeq ($(VISA),1)  # Add VISA dependencies
  ifeq ($(UNAME), Darwin)
    PC_CFLAGS+=-F/Library/Frameworks -D LDVISA
    PC_LDFLAGS+=-F/Library/Frameworks -framework RsVisa
  else ifeq ($(UNAME), Linux)
    PC_CFLAGS+=-D LDVISA
    PC_LDFLAGS+=-lrsvisa
  endif
endif

define PKG_CONF_FILE
prefix=$(PREFIX)
includedir=$${prefix}/include
libdir=$${prefix}/lib

Name: labdev
Description: Library for remote control and operation of lab devices
Version: 0.0.1
Cflags: $(PC_CFLAGS)
Libs: $(PC_LDFLAGS)
Requires: libusb-1.0 >= 0.29.2
endef
export PKG_CONF_FILE

# pkg-config .pc file path
PC_PATH=
ifeq ($(PC_PATH),)
  PC_PATH:=$(PREFIX)/lib/pkgconfig
endif

.PHONY: all clean install uninstall $(LIBNAME).pc

all: $(LIBNAME).a

%.o: %.cpp Makefile
	$(CC) -c -o $@ $< $(CFLAGS) -I$(SRC) -I$(INC)

$(LIBNAME).a: $(OBJ)
	ar -rc $@ $^
	ranlib $@

$(LIBNAME).pc:
	@echo "$$PKG_CONF_FILE" > $@

install: $(LIBNAME).a $(LIBNAME).pc
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PC_PATH)
	mkdir -p $(DESTDIR)$(PREFIX)/include
	cp $(LIBNAME).a $(DESTDIR)$(PREFIX)/lib/$(LIBNAME).a
	cp $(LIBNAME).pc $(DESTDIR)$(PC_PATH)/$(LIBNAME).pc
	cp -R $(INC)/labdev $(DESTDIR)$(PREFIX)/include/

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/$(LIBNAME).a
	rm -f $(DESTDIR)$(PC_PATH)/$(LIBNAME).pc
	rm -rf $(DESTDIR)$(PREFIX)/include/labdev
  
clean:
	rm -f $(OBJ)
	rm -f $(LIBNAME).a
	rm -f $(LIBNAME).pc
