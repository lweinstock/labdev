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

PYTHON_SITE=$(shell python3 -c 'import sysconfig; print(sysconfig.get_paths()["purelib"])')

# Change compiler flags for macOS/Linux
UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)  # macOS
    CFLAGS+=-undefined dynamic_lookup
  else ifeq ($(UNAME),Linux)  # linux
    CFLAGS+=-fPIC
endif

.PHONY: all clean install uninstall

all: $(LIB)$(SUFF)

$(LIB)$(SUFF): $(LIB).cpp
	$(CC) $(CFLAGS) $(INC_PYBIND) $^ -o $@ $(LDFLAGS)

install: $(LIB)$(SUFF)
	cp $< $(PYTHON_SITE)

uninstall:
	rm $(PYTHON_SITE)/$(LIB)$(SUFF)

clean:
	rm -f *.so
