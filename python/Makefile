CC=g++
CFLAGS=-O3 -Wall -std=c++11 -shared
LDFLAGS=
LIB=pylabdev
SUFF=$(shell python3-config --extension-suffix)

INC_PYBIND=$(shell python3 -m pybind11 --includes)
LIB_PYTHON=$(shell python3-config --ldflags)
LDFLAGS+=$(LIB_PYTHON)
CFLAGS+=$(shell pkg-config liblabdev --cflags)
LDFLAGS+=$(shell pkg-config liblabdev --libs)

# Change compiler flags for macOS/Linux
UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)  # macOS
    CFLAGS+=-undefined dynamic_lookup
  else ifeq ($(UNAME),Linux)  # linux
    CFLAGS+=-fPIC
endif

.PHONY: all clean

all: $(LIB)$(SUFF)

$(LIB)$(SUFF): $(LIB).cpp
	$(CC) $(CFLAGS) $(INC_PYBIND) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.so