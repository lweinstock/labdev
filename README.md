# labdev

## General Info

The Library for Lab Devices (labdev) provides a C++ API for readout and remote control of laboratory instruments such as oscilloscopes, function generators, power supplies and so on. The library defines two main components: interfaces and devices. Interfaces provide a communication channel to transfer data from and to the instrument. Devices provide user friendly functions and use interfaces to send characters and strings formatted according to a protocol to control and monitor the instrument.

The library is supported on macOS, Linux (Ubuntu) and Windows using Windows Subsystem for Linux (WSL).

## Setup

To labdev is compiled using `make` and a C++ compiler with `std >= C++11`. Building labdev requires a version of `libusb-1.0`:

- `sudo apt-get install libusb-1.0-0-dev`: Linux and WSL
- `sudo port install libusb`: macOS using MacPorts
- `brew install libusb`: macOS using homebrew (no root required)

To compile labdev clone the repository using your user name/mail address and password:
```
git clone https://github.com/lweinstock/labdev.git
cd labdev
make
sudo make install
```

The instruction `sudo make install` will copy the header files to `/usr/local/inc`, the library to `/usr/local/lib`, and a pkg-config file to `/usr/local/lib/pkgconfig`. Binaries can be compiled against labdev by adding the following line to the makefile

```
CFLAGS+=$(shell pkg-config --cflags liblabdev)
LDFLAGS+=$(shell pkg-config --libs liblabdev)
```

The installation can be undone by invoking `sudo make uninstall`.

## VISA support

The labdev also provides interfaces using the Virtual Instrument Software Architecture (VISA). The implementation by Rohde und Schwarz (RsVisa) is strongly recommended since it receives more updates and supports more platfrms that other implementations (e.g. NIVISA). The most recent version of RsVisa can be obtained at https://www.rohde-schwarz.com/applications/r-s-visa-application-note_56280-148812.html (state 14.06.2022).

Once RsVisa is installed (see below) liblabdev is compiled with VISA support by invoking `make VISA=1`.

### Install RsVisa on macOS

Simply download the `rsvisaXX.YY.ZZ.pkg` file, double click and install. Headers and library are packaged as a framework and installed to `/Library/Frameworks/RsVisa.framework`.

### Install RsVisa on Linux (Ubuntu)

Download the `rsvisa_XX.YY.ZZ_amd64.deb` and install using `sudo apt install ./rsvisa_XX.YY.ZZ_amd64.deb`. Headers should be installed to `/usr/include` and library to `/usr/lib`.

## License

This project is licensed under the MIT license - see the LICENSE.md file for details.
