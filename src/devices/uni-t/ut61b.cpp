#include <labdev/devices/uni-t/ut61b.hh>
#include "ld_debug.hh"

#include <unistd.h>
#include <math.h>
#include <algorithm>

namespace labdev {

    ut61b::ut61b():
        m_comm(nullptr),
        m_unit("?") {
        return;
    }

    ut61b::ut61b(serial_interface* ser):
        ut61b() {

        if (!ser) {
            fprintf(stderr, "invalid interface\n");
            abort();
        }
        if (!ser->connected()) {
            fprintf(stderr, "interface not connected\n");
            abort();
        }

        // Serial setup: 2400 8N1, 1 stop bit
        ser->set_baud(2400);
        ser->set_nbits(8);
        ser->set_parity(false);
        ser->set_stop_bits(1);
        ser->apply_settings();

        // General initialization
        m_comm = ser;

        return;
    }

    ut61b::~ut61b() {
        return;
    };

    double ut61b::get_value() {
        // with dtr+ and rts- the multimeter starts sending values
        m_comm->set_dtr();
        m_comm->clear_rts();
        std::string msg(""), buf("");
        // read until EOM found
        while (msg.find(ut61b::EOM) == std::string::npos) {
            buf = m_comm->read();
            if (buf.size() > 0)
                msg.append(buf);
            // DMM is slow, so dont stress it out!
            usleep(100e3);
        }
        // stop sending values
        m_comm->set_dtr();
        m_comm->clear_rts();

        // Packet should have 14 bytes!
        if (msg.size() != 14)
            throw ut61b_exception("Packet has wrong size", msg.size());

        // replace all '?' (whitespace on multimeter) with '0'
        std::replace(msg.begin(), msg.end(), '?', '0');
        if (msg.find(':') != std::string::npos)     // value out of range "0.L"
            return -1.;

        // value is stored as ascii string in first 6 bytes
        double value = std::stof( msg.substr(0, 5) );

        // byte 6 determines position of m_comma
        switch ( msg.at(6) ) {
        case '1':   value /= 1000;  break;
        case '2':   value /= 100;   break;
        case '4':   value /= 10;    break;
        case '0':   value /= 1;     break;
        default:
            throw ut61b_exception("Illegal comma position", msg.at(6));
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
            m_unit = "volt";
        else if (msg.at(10) & unit_flag::amp)
            m_unit = "ampere";
        else if (msg.at(10) & unit_flag::ohm)
            m_unit = "ohm";
        else if (msg.at(10) & unit_flag::hertz)
            m_unit = "hertz";
        else if (msg.at(10) & unit_flag::degf)
            m_unit = "degF";
        else if (msg.at(10) & unit_flag::degc)
            m_unit = "degC";
        else
            m_unit = "?";

        return value;
    }

}