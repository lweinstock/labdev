#ifndef SDG1000X_HH
#define SDG1000X_HH

#include <labdev/devices/fgen.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/devices/scpi.hh>

namespace labdev {
    class sdg1000x: public fgen {
    public:
        sdg1000x() : fgen(2, "Siglent,SDG1000X") {};
        sdg1000x(ip_address &ip);
        ~sdg1000x();

        static constexpr unsigned PORT = 5025;

        void connect(ip_address &ip);

        // Turn channel on/off
        void enable_channel(unsigned channel, bool ena = true) override;
        // Get output state of channel (true = on, false = off)
        bool get_state(unsigned channel) override;

        // Signal waveform 
        void set_waveform(unsigned channel, waveform wvfm) override;
        waveform get_waveform(unsigned channel) override;

        // Signal frequency 
        void set_freq(unsigned channel, float freq_hz) override;
        float get_freq(unsigned channel) override;

        // Signal duty cycle
        void set_duty_cycle(unsigned channel, float dcl) override;
        float get_duty_cycle(unsigned channel) override;

        // Signal phase in degree
        void set_phase(unsigned channel, float phase_deg) override;
        float get_phase(unsigned channel) override;

        // Signal amplitude in V
        void set_ampl(unsigned channel, float ampl_v) override;
        float get_ampl(unsigned channel) override;

        // Signal offset in V
        void set_offset(unsigned channel, float offset_v) override;
        float get_offset(unsigned channel) override;

    private:
        void init();
        
        scpi* m_scpi;

    };
}

#endif