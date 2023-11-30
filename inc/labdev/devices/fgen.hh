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

    // Set and get waveforms
    virtual void set_sine(unsigned channel, float freq_hz, float ampl_v = 1., 
        float offset_v = 0., float phase_deg = 0.) = 0;
    virtual void set_square(unsigned channel, float freq_hz, float ampl_v = 1., 
        float offset_v = 0., float phase_deg = 0., float duty_cycle = 0.5) = 0;
    virtual void set_ramp(unsigned channel, float freq_hertz, float ampl_v = 1., 
        float offset_v = 0., float phase_deg = 0., float symm = 0.5) = 0;
    virtual void set_pulse(unsigned channel, float period_s, float width_s, 
        float delay_s = 0., float high_v = .1, float low_v = 0., 
        float rise_s = 1e-6, float fall_s = 1e-6) = 0;
    virtual void set_noise(unsigned channel, float mean_v = 0., 
        float stdev_v = 1.) = 0;

    virtual bool is_sine(unsigned channel) = 0;
    virtual bool is_square(unsigned channel) = 0;
    virtual bool is_ramp(unsigned channel) = 0;
    virtual bool is_pulse(unsigned channel) = 0;
    virtual bool is_noise(unsigned channel) = 0;

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