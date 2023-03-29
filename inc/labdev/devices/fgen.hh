#ifndef LD_FGEN_HH
#define LD_FGEN_HH

#include <cstring>
#include <string>
#include <vector>

#include <labdev/devices/device.hh>

namespace labdev{

    /*
     *  Abstract base class for all function generators
     */

    class fgen : public device {
    public:
        fgen(unsigned n_ch, std::string name = "?") : 
            device(name), m_n_ch(n_ch) {};
        virtual ~fgen() {};

        enum waveform : unsigned {
            SINE        = 0,
            SQUARE      = 1,
            RECTANGLE   = 2,
            TRAPEZOID   = 3,
            CMOS        = 4,
            PULSE       = 5,
            DC          = 6,
            TRIANGLE    = 7,
            POS_RAMP    = 8,
            NEG_RAMP    = 9,
            NOISE       = 10
        };

        // Returns number of channels of device
        const unsigned get_n_channels() const { return m_n_ch; }

        // Turn channel on/off
        virtual void enable_channel(unsigned channel, bool ena = true) = 0;
        void disable_channel(unsigned channel) 
            { this->enable_channel(channel, false); }
        // Get output state of channel (true = on, false = off)
        virtual bool get_state(unsigned channel) = 0;

        // Signal waveform 
        virtual void set_waveform(unsigned channel, waveform wvfm) = 0;
        virtual waveform get_waveform(unsigned channel) = 0;

        // Signal frequency 
        virtual void set_freq(unsigned channel, float freq_hz) = 0;
        virtual float get_freq(unsigned channel) = 0;

        // Signal duty cycle
        virtual void set_duty_cycle(unsigned channel, float dcl) = 0;
        virtual float get_duty_cycle(unsigned channel) = 0;

        // Signal phase in degree
        virtual void set_phase(unsigned channel, float phase_deg) = 0;
        virtual float get_phase(unsigned channel) = 0;

        // Signal amplitude in V
        virtual void set_ampl(unsigned channel, float ampl_v) = 0;
        virtual float get_ampl(unsigned channel) = 0;

        // Signal offset in V
        virtual void set_offset(unsigned channel, float offset_v) = 0;
        virtual float get_offset(unsigned channel) = 0;

    private:
        const unsigned m_n_ch;
    };
}

#endif