#ifndef LD_FGEN_HH
#define LD_FGEN_HH

#include <cstring>
#include <string>
#include <vector>

#include <labdev/devices/ld_device.hh>

namespace labdev{

/*
 *  Abstract base class for all function generators
 */
class fgen : public ld_device {
public:
    fgen(unsigned n_ch, std::string name = "?") : 
        ld_device(name), m_n_ch(n_ch) {};
    virtual ~fgen() {};

    // Returns number of channels of device
    const unsigned get_n_channels() const { return m_n_ch; }

    // Turn channel on/off
    virtual void enable_channel(unsigned channel, bool ena = true) = 0;
    void disable_channel(unsigned channel) 
        { this->enable_channel(channel, false); }
    // Get output state of channel (true = on, false = off)
    virtual bool get_state(unsigned channel) = 0;

    // Waveform
    enum waveform : unsigned {SINE = 0, SQUARE, RAMP, PULSE, NOISE, DC};
    virtual void set_wvfm(unsigned channel, waveform wvfm) = 0;
    virtual waveform get_wvfm(unsigned channel) = 0;

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

    // Rising and falling edges
    virtual void set_rising(unsigned channel, float rise_s) = 0;
    virtual float get_rising(unsigned channel) = 0;
    virtual void set_falling(unsigned channel, float fall_s) = 0;
    virtual float get_falling(unsigned channel) = 0;

    // Pulse width
    virtual void set_pulse_width(unsigned channel, float width_s) = 0;
    virtual float get_pulse_width(unsigned channel) = 0;


private:
    const unsigned m_n_ch;
};

}

#endif