CC=g++
CFLAGS=-Wall -std=c++11 -I. -shared
LDFLAGS=
LIB=pylabdev
SUFF=$(shell python3-config --extension-suffix)

INC_PYBIND=$(shell python3 -m pybind11 --includes)
LIB_PYTHON=$(shell python3-config --ldflags)
LDFLAGS+=$(LIB_PYTHON)
LDFLAGS+=$(shell pkg-config liblabdev --libs)

.PHONY: all clean

all: $(LIB)$(SUFF)

$(LIB)$(SUFF): $(LIB).cpp
	$(CC) $(CFLAGS) -undefined dynamic_lookup $(INC_PYBIND) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.so
