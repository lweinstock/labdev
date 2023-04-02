#include <labdev/devices/siglent/sdg1000x.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <iomanip>

using namespace std;

namespace labdev {

    sdg1000x::sdg1000x(ip_address &ip):
    fgen(2, "Siglent,SDG1000X")
    {
        this->connect(ip);
        return;
    }

    sdg1000x::~sdg1000x() 
    {
        if (m_scpi) {
            delete m_scpi;
            m_scpi = nullptr;
        }
        return;
    }

    void sdg1000x::connect(ip_address &ip) 
    {
        if ( this->connected() ) {
            fprintf(stderr, "Device is already connected!\n");
            abort();
        }
        if ( ip.port != sdg1000x::PORT )
        {
            fprintf(stderr, "SDG1000X only supports port %i\n", sdg1000x::PORT);
            abort();
        }
        m_comm = new tcpip_interface(ip);
        init();
        return;
    }

    void sdg1000x::enable_channel(unsigned channel, bool ena)
    {
        this->check_channel(channel);
        stringstream msg;
        msg << "C" << channel << ":OUTP " << (ena ? "ON" : "OFF") << "\n";
        m_comm->write(msg.str()); 
        return;
    }

    bool sdg1000x::get_state(unsigned channel)
    {
        this->check_channel(channel);
        string ret = m_comm->query("C" + to_string(channel) + ":OUTP?\n");
        if (ret.find("ON") != string::npos)
            return true;
        return false;
    }

    void sdg1000x::set_waveform(unsigned channel, fgen::waveform wvfm)
    {
        this->check_channel(channel);
        stringstream msg;
        msg << "C" << channel << ":BSWV WVTP," << this->wvfm_to_str(wvfm) << "\n";
        m_comm->write(msg.str());
        return;
    }

    fgen::waveform sdg1000x::get_waveform(unsigned channel)
    {
        this->check_channel(channel);
        string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
        string wvfm = this->get_bswv_val(bswv, "WVTP");
        debug_print("Received waveform '%s'\n", wvfm.c_str());
        return this->str_to_wvfm(wvfm);
    }

    void sdg1000x::set_freq(unsigned channel, float freq_hz)
    {
        this->check_channel(channel);
        stringstream msg;
        msg << "C" << channel << ":BSWV FRQ,";
        msg << setprecision(3) << fixed << freq_hz << "\n";
        m_comm->write(msg.str());
        return;
    }

    float sdg1000x::get_freq(unsigned channel)
    {
        this->check_channel(channel);
        string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
        string freq = this->get_bswv_val(bswv, "FRQ");
        if (freq.empty())  // Parameter not found
            return 0.;
        debug_print("Received frequency %s\n", freq.c_str());
        return stof(freq);
    }

    void sdg1000x::set_duty_cycle(unsigned channel, float dcl)
    {
        this->check_channel(channel);
        stringstream msg;
        msg << "C" << channel << ":BSWV DUTY,";
        msg << setprecision(3) << fixed << 100.*dcl << "\n";
        m_comm->write(msg.str());
        return;
    }

    float sdg1000x::get_duty_cycle(unsigned channel)
    {
        this->check_channel(channel);
        string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
        string dcl = this->get_bswv_val(bswv, "DUTY");
        if (dcl.empty())  // Parameter not found
            return 0.;
        debug_print("Received duty cycle %s\n", dcl.c_str());
        return 0.01*stof(dcl);
    }

    void sdg1000x::set_phase(unsigned channel, float phase_deg)
    {
        this->check_channel(channel);
        stringstream msg;
        msg << "C" << channel << ":BSWV PHSE,";
        msg << setprecision(3) << fixed << phase_deg << "\n";
        m_comm->write(msg.str());
        return;
    }

    float sdg1000x::get_phase(unsigned channel)
    {
        this->check_channel(channel);
        string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
        string phase = this->get_bswv_val(bswv, "PHSE");
        if (phase.empty())  // Parameter not found
            return 0.;
        debug_print("Received phase %s\n", phase.c_str());
        return stof(phase);
    }

    void sdg1000x::set_ampl(unsigned channel, float ampl_v)
    {
        this->check_channel(channel);
        stringstream msg;
        msg << "C" << channel << ":BSWV AMP,";
        msg << setprecision(3) << fixed << ampl_v << "\n";
        m_comm->write(msg.str());
        return;
    }
    
    float sdg1000x::get_ampl(unsigned channel)
    {
        this->check_channel(channel);
        string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
        string ampl = this->get_bswv_val(bswv, "AMP");
        if (ampl.empty())  // Parameter not found
            return 0.;
        debug_print("Received amplitude %s\n", ampl.c_str());
        return stof(ampl);
    }

    void sdg1000x::set_offset(unsigned channel, float offset_v)
    {
        return;
    }

    float sdg1000x::get_offset(unsigned channel)
    {
        return 0.;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    void sdg1000x::init() 
    {
        // Setup SCPI
        m_scpi = new scpi(m_comm);
        m_scpi->clear_status();
        m_dev_name = m_scpi->get_identifier();
        return;
    }

    void sdg1000x::check_channel(unsigned channel) 
    {
        if ( (channel == 0) || (channel > this->get_n_channels()) ) {
            fprintf(stderr, "Invalid channel %i\n", channel);
            abort();
        }
        return;
    }

    std::string sdg1000x::get_bswv_val(std::string bswv, std::string par) 
    {
        size_t pos1 = bswv.find(par);
        if (pos1 == string::npos)   // Parameter not found, return empty string
            return {};
        pos1 += par.size() + 1;
        // Values and parameters are separated by commas
        size_t pos2 = bswv.find(',', pos1 + 1);
        return bswv.substr(pos1, pos2 - pos1);
    }

    std::string sdg1000x::wvfm_to_str(fgen::waveform wvfm)
    {
        string ret("");
        switch (wvfm) {
            case fgen::waveform::SINE:
                ret = "SINE";
                break;
            case fgen::waveform::SQUARE:
                ret = "SQUARE";
                break;
            case fgen::waveform::PULSE:
                ret = "PULSE";
                break;
            case fgen::waveform::DC:
                ret = "DC";
                break;
            case fgen::waveform::NOISE:
                ret = "NOISE";
                break;
            case fgen::waveform::POS_RAMP:
            case fgen::waveform::NEG_RAMP:
                ret = "RAMP";
                break;
            default:
                fprintf(stderr, "Waveform not supported!\n");
                abort();
        }
        return ret;
    }
    
    fgen::waveform sdg1000x::str_to_wvfm(std::string wvfm)
    {
        if (wvfm == "SINE")
            return fgen::waveform::SINE;
        if (wvfm == "SQUARE")
            return fgen::waveform::SQUARE;
        if (wvfm == "PULSE")
            return fgen::waveform::PULSE;
        if (wvfm == "DC")
            return fgen::waveform::DC;
        if (wvfm == "NOISE")
            return fgen::waveform::NOISE;
        if (wvfm == "RAMP")
            return fgen::waveform::POS_RAMP;

        fprintf(stderr, "Waveform not supported!\n");
        abort();
        return fgen::waveform::SINE;
    }

}