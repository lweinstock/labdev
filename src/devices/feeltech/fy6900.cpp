#include <labdev/devices/feeltech/fy6900.hh>
#include "ld_debug.hh"

#include <unistd.h>
#include <math.h>

namespace labdev {

    fy6900::fy6900 () {
        return;
    }

    fy6900::fy6900 (serial_interface* ser) {
        if (!ser) {
            fprintf(stderr, "invalid interface\n");
            abort();
        }
        if (!ser->connected()) {
            fprintf(stderr, "interface not connected\n");
            abort();
        }

        // Serial setup: 115200 8N1
        ser->set_baud(115200);
        ser->set_nbits(8);
        ser->set_parity(false);
        ser->set_stop_bits(1);
        ser->apply_settings();

        // General initialization
        comm = ser;

        return;
    }

    fy6900::~fy6900 () {
        return;
    };

    void fy6900::enable_ch(int ch, bool on_off) {
        std::string msg;
        if (ch == 1) {
            msg = "WMN" + (on_off ? std::string("1") : std::string("0") );
        } else if (ch == 2) {
            msg = "WFN" + (on_off ? std::string("1") : std::string("0") );
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        std::string ret = this->query_cmd(msg);
        // Response should be empty
        if (ret.size() > 0)
            throw fy6900_exception("Received unexpected non-empty response", -1);

        return;
    }

    void fy6900::set_waveform(int ch, int wvfm) {
        std::string msg;
        if (ch == 1) {
            msg = "WMW";
        } else if (ch == 2) {
            msg = "WFW";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        if ( (wvfm < 0) || (wvfm > 99) ) {
            fprintf(stderr, "Invalid waveform %i\n", wvfm);
            abort();
        }
        msg += std::to_string(wvfm);
        std::string ret = this->query_cmd(msg);
        // Response should be empty
        if (ret.size() > 0)
            throw fy6900_exception("Received unexpected non-empty response", -1);

        return;
    }

    void fy6900::set_freq(int ch, float freq_hz) {
        std::string msg;
        if (ch == 1) {
            msg = "WMF";
        } else if (ch == 2) {
            msg = "WFF";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        if ( (freq_hz < 0) || (freq_hz > 60e6) ) {
            fprintf(stderr, "Invalid frequency %f\n", freq_hz);
            abort();
        }
        // Frequency is set in steps of uHz!
        long unsigned freq_uhz = static_cast<long unsigned>(freq_hz*1e6);
        msg += std::to_string(freq_uhz);
        std::string ret = this->query_cmd(msg);
        // Response should be empty
        if (ret.size() > 0)
            throw fy6900_exception("Received unexpected non-empty response", -1);

        return;
    }

    void fy6900::set_duty_cycle(int ch, float dcl) {
        std::string msg;
        if (ch == 1) {
            msg = "WMD";
        } else if (ch == 2) {
            msg = "WFD";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        if ( (dcl < 0) || (dcl > 1.) ) {
            fprintf(stderr, "Invalid duty cycle %.3f\n", dcl);
            abort();
        }
        // duty cycle is given in %
        char buf[6];
        sprintf(buf, "%05.3f", 100*dcl);
        msg.append(buf, sizeof(buf));
        std::string ret = this->query_cmd(msg);
        // Response should be empty
        if (ret.size() > 0)
            throw fy6900_exception("Received unexpected non-empty response", -1);

        return;
    }

    void fy6900::set_phase(int ch, float phase_deg) {
        std::string msg;
        if (ch == 1) {
            msg = "WMP";
        } else if (ch == 2) {
            msg = "WFP";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        if ( (phase_deg < 0) || (phase_deg >= 360.) ) {
            fprintf(stderr, "Invalid phase value %.3f\n", phase_deg);
            abort();
        }
        char buf[7];
        sprintf(buf, "%06.3f", phase_deg);
        msg.append(buf, sizeof(buf));
        std::string ret = this->query_cmd(msg);
        // Response should be empty
        if (ret.size() > 0)
            throw fy6900_exception("Received unexpected non-empty response", -1);

        return;
    }

    void fy6900::set_ampl(int ch, float ampl_v) {
        std::string msg;
        if (ch == 1) {
            msg = "WMA";
        } else if (ch == 2) {
            msg = "WFA";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        if ( (ampl_v < 0) || (ampl_v > 24.) ) {
            fprintf(stderr, "Invalid amplitude %.4f\n", ampl_v);
            abort();
        }
        char buf[8] = {'\0'};
        sprintf(buf, "%07.4f", ampl_v);
        msg.append(buf, sizeof(buf));
        std::string ret = this->query_cmd(msg);
        // Response should be empty
        if (ret.size() > 0)
            throw fy6900_exception("Received unexpected non-empty response", -1);

        return;
    }

    void fy6900::set_offset(int ch, float offset_v) {
        std::string msg;
        if (ch == 1) {
            msg = "WMO";
        } else if (ch == 2) {
            msg = "WFO";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        if ( (offset_v < -12.) || (offset_v > 12.0) ) {
            fprintf(stderr, "Invalid offset %.4f\n", offset_v);
            abort();
        }
        char buf[8];
        sprintf(buf, "%07.4f", offset_v);
        msg.append(buf, sizeof(buf));
        std::string ret = this->query_cmd(msg);
        // Response should be empty
        if (ret.size() > 0)
            throw fy6900_exception("Received non-empty response", -1);

        return;
    }

    bool fy6900::get_status(int ch) {
        std::string msg;
        if (ch == 1) {
            msg = "RMN";
        } else if (ch == 2) {
            msg = "RFN";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        std::string resp = this->query_cmd(msg);
        if (resp.size() == 0)
            throw fy6900_exception("Received empty response", -1);
        int stat = std::stoi(resp);
        return (stat == 0) ? false : true;
    }

    std::string fy6900::get_waveform(int ch) {
        std::string msg;
        if (ch == 1) {
            msg = "RMW";
        } else if (ch == 2) {
            msg = "RFW";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        std::string resp = this->query_cmd(msg);
        if (resp.size() == 0)
            throw fy6900_exception("Received empty response", -1);
        return this->wvfm_string( std::stoi(resp) );
    }

    float fy6900::get_freq(int ch) {
        std::string msg;
        if (ch == 1) {
            msg = "RMF";
        } else if (ch == 2) {
            msg = "RFF";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        std::string resp = this->query_cmd(msg);
        if (resp.size() == 0)
            throw fy6900_exception("Received empty response", -1);
        return std::stof(resp);
    }

    float fy6900::get_duty_cycle(int ch) {
        std::string msg;
        if (ch == 1) {
            msg = "RMD";
        } else if (ch == 2) {
            msg = "RFD";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        std::string resp = this->query_cmd(msg);
        if (resp.size() == 0)
            throw fy6900_exception("Received empty response", -1);
        float ret = std::stoi(resp)/10000.;
        return ret;
    }

    float fy6900::get_phase(int ch) {
        std::string msg;
        if (ch == 1) {
            msg = "RMP";
        } else if (ch == 2) {
            msg = "RFP";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        std::string resp = this->query_cmd(msg);
        if (resp.size() == 0)
            throw fy6900_exception("Received empty response", -1);
        return std::stoi(resp)/1000.;
    }

    float fy6900::get_ampl(int ch) {
        std::string msg;
        if (ch == 1) {
            msg = "RMA";
        } else if (ch == 2) {
            msg = "RFA";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        std::string resp = this->query_cmd(msg);
        if (resp.size() == 0)
            throw fy6900_exception("Received empty response", -1);
        return std::stoi(resp)/10000.;
    }

    float fy6900::get_offset(int ch) {
        std::string msg;
        if (ch == 1) {
            msg = "RMO";
        } else if (ch == 2) {
            msg = "RFO";
        } else {
            fprintf(stderr, "Invalid channel number %i\n", ch);
            abort();
        }
        std::string resp = this->query_cmd(msg);
        if (resp.size() == 0)
            throw fy6900_exception("Received empty response", -1);
        float ret = (int32_t)std::stol(resp)/1000.;
        return ret;
    }


    /*
     *      P R I V A T E   M E T H O D S
     */


    std::string fy6900::query_cmd(std::string cmd) {
        size_t pos = std::string::npos;
        std::string ret(""), buf("");
        comm->write(cmd + '\n');

        // Read until EOM character is found
        while ( pos == std::string::npos ) {
            usleep(SLEEP_US);
            buf = comm->read();
            if (buf.size() > 0) {
                ret.append(buf);
                pos = ret.find(EOM);
            } else {
                throw fy6900_exception("Received no response for query\n");
            }
        }
        // Remove EOM character and return value
        ret = ret.substr(0, pos);
        debug_print("Queried '%s' and received %lu bytes: '%s'\n",
            cmd.c_str(), ret.size(), ret.c_str());
        return ret;
    }

    std::string fy6900::wvfm_string(unsigned wvfm) const {
        std::string ret;
        switch (wvfm) {
        // Waveforms
        case SINE:      ret = "sine";       break;
        case SQUARE:    ret = "square";     break;
        case RECTANGLE: ret = "rectangle";  break;
        case TRAPEZOID: ret = "trapezoid";  break;
        case CMOS:      ret = "cmos";       break;
        case PULSE:     ret = "adj pulse";  break;
        case DC:        ret = "dc";         break;
        case TRIANGLE:  ret = "trgl";       break;
        case POS_RAMP:  ret = "ramp";       break;
        case NEG_RAMP:  ret = "neg ramp";   break;
        // TODO: waveform 11 to 99 ...
        default:
            debug_print("%s", "Unrecognized waveform\n");
            ret = "";
        }
        return ret;
    }

}