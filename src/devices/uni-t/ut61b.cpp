#include <labdev/devices/uni-t/ut61b.hh>
#include <labdev/ld_debug.hh>
#include <labdev/exceptions.hh>

#include <unistd.h>
#include <math.h>
#include <algorithm>

using namespace std;

namespace labdev {

ut61b::ut61b(serial_config &ser): ut61b() 
{
    if ( ser.baud != ut61b::BAUD || ser.nbits != 8 || ser.par_ena
      || ser.stop_bits != 1) {
        fprintf(stderr, "UT61B only supports %i baud 8N1\n", ut61b::BAUD);
        abort();
    }
    m_serial = std::make_unique<serial_interface>(ser);
    //m_serial->read();
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