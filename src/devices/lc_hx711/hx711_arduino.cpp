#include <labdev/devices/lc_hx711/hx711_arduino.hh>
#include <labdev/ld_debug.hh>
#include <labdev/exceptions.hh>
#include <labdev/utils/utils.hh>


#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>

using namespace std;

namespace labdev {


void hx711_arduino::connect(std::unique_ptr<ld_iface> comm)
{
    if ( this->connected() ) {
        string err = this->get_info() + " : device is already connected";
        throw device_error(err);
        return;
    }

    Interface_type type = comm->type();
    if (type == SERIAL) {
        unique_ptr<serial_iface> ser(
            dynamic_cast<serial_iface*>(comm.release()));

        if (ser->get_baud() != hx711_arduino::BAUD) {
            fprintf(stderr, "Invalid baud rate %u BAUD (115200 8N1 required)\n", 
                ser->get_baud());
            abort();
        }
        if (ser->get_nbits() != 8) {
            fprintf(stderr, "Invalid number of bits %u (115200 8N1 required)\n", 
                ser->get_nbits());
            abort();
        }
        if (ser->get_parity() != false) {
            fprintf(stderr, "Invalid parity (115200 8N1 required)\n");
            abort();
        }
        if (ser->get_stop_bits() != 1) {
            fprintf(stderr, "Invalid number of stop bits %u (115200 8N1 required)\n", 
                ser->get_stop_bits());
            abort();
        }

        // Everything seems to be in order
        m_serial = std::move(ser);
    } else {
        string err = this->get_info() + " : interface is not supported";
        throw device_error(err); 
    } 
    return;
}

int hx711_arduino::read_lc()
{
    if (!m_serial) {
        throw device_error(this->get_info() + " : not connected");
    }

    m_serial->write("G");
    string resp = m_serial->read_until("\r\n");
    string strVal = resp.substr(1, string::npos);
    cout << "String value = " << strVal << endl;
    int value = stoi(strVal);
    cout << "Conversion result = "<< value << endl;
    return value;
}


} // namespace labdev