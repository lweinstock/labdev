# labdev

## General Info

The Library for Lab Devices (labdev) provides a C++ API for readout and remote control of laboratory instruments such as oscilloscopes, function generators, power supplies and so on. The library defines two main components: interfaces and devices. Interfaces provide a communication channel to transfer data from and to the instrument. Devices provide user friendly functions and use interfaces to send strings formatted according to a protocol to control and monitor the instrument.

The library is supported on macOS, Linux and Windows using Windows Subsystem for Linux (WSL).

## Setup

To labdev is compiled using `make >= 3.81` and a C++ compiler with `std >= C++11`. Building labdev requires a version of `libusb-1.0`:

- `sudo apt-get install libusb-1.0-0`: Linux and WSL
- `sudo port install libusb`: macOS using MacPorts
- `brew install libusb`: macOS using homebrew (no root required)

The library supports implementations of the Virtual Instrument Software Architecture (VISA) which can be used to interface with measurement instruments. If VISA support is not required compile the library with `make VISA=0` or simply `make`. If VISA is required or preferred by your application the makefiles include path `VISA_INC` has to be updated and labdev has to be compiled using `make VISA=1`. For more details see the Wiki (TODO).

To compile labdev clone the repository using your user name/mail address and password:
```
git clone git@gitlab.rlp.net:lweinsto/labdev.git
cd labdev
make
```

The makefile creates the static library `liblabdev.a` in `labdev/` ready for use. 

## Integration

To compile a project using labdev refer to `labdev/examples`. If you encounter problems during the setup with the lab equipment (e.g. frequent timeouts, cannot connect to device, etc.) refer to the instruments Wiki page and check for known bugs!

## License

TODO!
