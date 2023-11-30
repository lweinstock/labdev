#include <labdev/devices/uni-t/ut61b.hh>
#include <labdev/ld_debug.hh>
#include <labdev/exceptions.hh>

#include <unistd.h>
#include <math.h>
#include <algorithm>

using namespace std;

namespace labdev {

ut61b::ut61b(serial_interface *ser): ut61b() 
{
    this->connect(ser);
    return;
}

void ut61b::connect(serial_interface* ser)
{
    // Check and assign communication interface
    this->set_comm(ser);

    // Check correct serial setup => 2400 8N1 (manual p. 28)
    if (ser->get_baud() != ut61b::BAUD) {
        fprintf(stderr, "Invalid baud rate %u BAUD (2400 8N1 required)\n", 
            ser->get_baud());
        abort();
    }
    if (ser->get_nbits() != 8) {
        fprintf(stderr, "Invalid number of bits %u (2400 8N1 required)\n", 
            ser->get_nbits());
        abort();
    }
    if (ser->get_parity() != false) {
        fprintf(stderr, "Invalid parity (2400 8N1 required)\n");
        abort();
    }
    if (ser->get_stop_bits() != 1) {
        fprintf(stderr, "Invalid number of stop bits %u (2400 8N1 required)\n", 
            ser->get_stop_bits());
        abort();
    }

    return;
}


double ut61b::get_value() {
    // with dtr+ and rts- the multimeter starts sending values
    m_serial->set_dtr();
    m_serial->clear_rts();

    string msg = m_serial->read_until(EOM, 1000);

    // stop sending values
    m_serial->set_dtr();
    m_serial->clear_rts();

    // Extract single packet
    size_t pos1 = msg.find_first_of("+-");
    size_t pos2 = msg.find(EOM) + strlen(EOM);
    msg = msg.substr(pos1, pos2 - pos1);
    if (msg.size() != 14)
        throw bad_protocol("Packet has wrong size " + to_string(msg.size()), msg.size());

    // replace all '0x00' (whitespace on multimeter) with '0'
    replace(msg.begin(), msg.end(), '\0', '0');
    if (msg.find(':') != string::npos)     // value out of range "0.L"
        return -1.;

    // value is stored as ascii string in first 6 bytes
    double value = stof( msg.substr(0, 5) );

    // byte 6 determines position of m_seriala
    switch ( msg.at(6) ) {
    case '1':   value /= 1000;  break;
    case '2':   value /= 100;   break;
    case '4':   value /= 10;    break;
    case '0':   value /= 1;     break;
    default:
        throw bad_protocol("Illegal comma position", msg.at(6));
    }

    // byte 9 determines prefix
    if ( msg.at(9) & prefix_flag::micro)
        value *= 1e-6;
    else if (msg.at(9) & prefix_flag::milli)
        value *= 1e-3;
    else if (msg.at(9) & prefix_flag::kilo)
        value *= 1e3;
    else if (msg.at(9) & prefix_flag::mega)
        value *= 1e6;
    else if (msg.at(9) & prefix_flag::none)
        value *= 1.;

    // byte 10 defines quantity
    m_unit.clear();
    if ( msg.at(10) & unit_flag::volt)
        m_unit = "V";
    else if (msg.at(10) & unit_flag::amp)
        m_unit = "A";
    else if (msg.at(10) & unit_flag::ohm)
        m_unit = "Ohm";
    else if (msg.at(10) & unit_flag::hertz)
        m_unit = "Hz";
    else if (msg.at(10) & unit_flag::degf)
        m_unit = "degF";
    else if (msg.at(10) & unit_flag::degc)
        m_unit = "degC";
    else
        m_unit = "?";

    return value;
}

}