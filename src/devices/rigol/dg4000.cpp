#include <labdev/devices/rigol/dg4000.hh>
#include <labdev/exceptions.hh>
#include "ld_debug.hh"

#include <sstream>
#include <unistd.h>

namespace labdev {

    const std::string dg4000::waveform_string[] =  {"SIN", "SQU", "RAMP",
        "PULS", "NOIS", "ARB", "HARM", "USER"};

    dg4000::dg4000() {
        return;
    }

    dg4000::dg4000(tcpip_interface* tcpip) : scpi_device(tcpip) {
        // General initialization
        init();
        return;
    }

    dg4000::dg4000(visa_interface* visa) : scpi_device(visa) {
        // General initialization
        init();
        return;
    }

    dg4000::dg4000(usbtmc_interface* usbtmc) : scpi_device(usbtmc) {
        // USB initialization
        usbtmc->claim_interface(0);
        usbtmc->set_endpoint_in(0x06);
        usbtmc->set_endpoint_out(0x02);

        // General initialization
        init();
        return;
    }

    dg4000::~dg4000() {
        return;
    }

    void dg4000::enable_channel(unsigned channel, bool enable) {
        this->check_channel(channel);
        std::stringstream msg("");
        msg << ":OUTP" << channel << ":STAT" << (enable? " ON" : " OFF") << "\n";
        comm->write(msg.str());
        return;
    }

    void dg4000::set_waveform(unsigned channel, unsigned wvfm) {
        this->check_channel(channel);
        if (wvfm > waveform::size - 1) {
            fprintf(stderr, "Invalid waveform %i\n", wvfm);
            abort();
        }

        std::stringstream msg("");
        msg << ":SOUR" << channel << ":APPL:" << waveform_string[wvfm] << "\n";
        this->write_at_least(msg.str(), 5);

        return;
    }

    void dg4000::set_freq(unsigned channel, double freq_hz) {
        this->check_channel(channel);
        if (freq_hz < 0) {
            fprintf(stderr, "Invalid frequency %f\n", freq_hz);
            abort();
        }
        std::stringstream msg("");
        msg << ":SOUR" << channel << ":FREQ " << freq_hz << "\n";
        this->write_at_least(msg.str(), 5);

        return;
    }

    void dg4000::set_duty_cycle(unsigned channel, double dcl) {
        this->check_channel(channel);
        if (dcl < 0. || dcl > 1.) {
            fprintf(stderr, "Invalid duty cycle %f\n", dcl);
            abort();
        }
        std::stringstream msg("");
        msg << ":SOUR" << channel << ":PULS:DCYC " << 100*dcl << "\n";
        this->write_at_least(msg.str(), 5);
        return;
    };

    void dg4000::set_phase(unsigned channel, double phase_deg) {
        this->check_channel(channel);
        if (phase_deg < 0. || phase_deg > 360.) {
            fprintf(stderr, "Invalid phase %f\n", phase_deg);
            abort();
        }
        std::stringstream msg("");
        msg << ":SOUR" << channel << ":PHAS " << phase_deg << "\n";
        this->write_at_least(msg.str(), 5);
        return;
    };

    void dg4000::set_ampl(unsigned channel, double ampl_v) {
        this->check_channel(channel);
        if (ampl_v < 0. || ampl_v > 20.) {
            fprintf(stderr, "Amplitude %f out of range\n", ampl_v);
            abort();
        }
        std::stringstream msg("");
        msg << ":SOUR" << channel << ":VOLT " << ampl_v << "\n";
        this->write_at_least(msg.str(), 5);

        return;
    };

    void dg4000::set_offset(unsigned channel, double offset_v) {
        this->check_channel(channel);
        if (offset_v < -7.5 || offset_v > 7.5) {
            fprintf(stderr, "Offset %f out of range\n", offset_v);
            abort();
        }
        std::stringstream msg("");
        msg << ":SOUR" << channel << ":VOLT:OFFS " << offset_v << "\n";
        this->write_at_least(msg.str(), 5);
        return;
    };

    std::string dg4000::get_waveform(int channel) {
        this->check_channel(channel);

        std::stringstream msg("");
        msg << ":SOUR" << channel << ":APPL?\n";
        std::string resp = comm->query(msg.str());
        return resp.substr(1, resp.find(",") - 1);
    }

    double dg4000::get_freq(unsigned channel) {
        this->check_channel(channel);

        std::stringstream msg("");
        msg << ":SOUR" << channel << ":FREQ?\n";
        std::string resp = comm->query(msg.str());
        return std::stof(resp);
    }

    double dg4000::get_duty_cycle(unsigned channel) {
        this->check_channel(channel);

        std::stringstream msg("");
        msg << ":SOUR" << channel << ":PULS:DCYC?\n";
        std::string resp = comm->query(msg.str());
        return std::stof(resp)/100.;
    }

    double dg4000::get_phase(unsigned channel) {
        this->check_channel(channel);

        std::stringstream msg("");
        msg << ":SOUR" << channel << ":PHAS?\n";
        std::string resp = comm->query(msg.str());
        return std::stof(resp);
    }

    double dg4000::get_ampl(unsigned channel) {
        this->check_channel(channel);

        std::stringstream msg("");
        msg << ":SOUR" << channel << ":VOLT?\n";
        std::string resp = comm->query(msg.str());
        return std::stof(resp);
    }

    double dg4000::get_offset(unsigned channel) {
        this->check_channel(channel);

        std::stringstream msg("");
        msg << ":SOUR" << channel << ":VOLT:OFFS?\n";
        std::string resp = comm->query(msg.str());
        return std::stof(resp);
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    void dg4000::init() {
        this->clear_status();
        this->wait_to_complete();
        return;
    }

    void dg4000::check_channel(unsigned channel) {
        if ( (channel != 1) && (channel != 2) ) {
            fprintf(stderr, "Invalid channel %i\n", channel);
            abort();
        }
        return;
    }

    void dg4000::write_at_least(std::string msg, unsigned time_ms) {

        /*
         * Known issue: while processing queries the DG4000 apparently ignores
         *  all following queries instead of storing them in an internal command
         *  queue. This makes it neccesary for some write commands to take at
         *  least a few milliseconds before returning.
         */

        struct timeval sta, sto;
        gettimeofday(&sta, NULL);
        comm->write(msg);
        gettimeofday(&sto, NULL);

        int diff_ms = (sto.tv_sec - sta.tv_sec)*1000
            + (sto.tv_usec - sta.tv_usec)/1000;
        if (time_ms > diff_ms)
            usleep( (time_ms - diff_ms)*1000 );
        return;
    }

}