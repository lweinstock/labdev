#include <labdev/devices/siglent/sdg1000x.hh>

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
        return;
    }

    bool sdg1000x::get_state(unsigned channel)
    {
        return false;
    }

    void sdg1000x::set_waveform(unsigned channel, fgen::waveform wvfm)
    {
        return;
    }

    fgen::waveform sdg1000x::get_waveform(unsigned channel)
    {
        return fgen::waveform::SINE;
    }

    void sdg1000x::set_freq(unsigned channel, float freq_hz)
    {
        return;
    }

    float sdg1000x::get_freq(unsigned channel)
    {
        return 0.;
    }

    void sdg1000x::set_duty_cycle(unsigned channel, float dcl)
    {
        return;
    }

    float sdg1000x::get_duty_cycle(unsigned channel)
    {
        return 0.;
    }

    void sdg1000x::set_phase(unsigned channel, float phase_deg)
    {
        return;
    }

    float sdg1000x::get_phase(unsigned channel)
    {
        return 0.;
    }

    void sdg1000x::set_ampl(unsigned channel, float ampl_v)
    {
        return;
    }
    
    float sdg1000x::get_ampl(unsigned channel)
    {
        return 0.;
    }

    void sdg1000x::set_offset(unsigned channel, float offset_v)
    {
        return;
    }

    float sdg1000x::get_offset(unsigned channel)
    {
        return 0.;
    }

    void sdg1000x::init() 
    {
        // Setup SCPI
        m_scpi = new scpi(m_comm);
        m_scpi->clear_status();
        m_dev_name = m_scpi->get_identifier();
        return;
    }

}