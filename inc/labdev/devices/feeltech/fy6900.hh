#ifndef FY6900_HH
#define FY6900_HH

#include <labdev/serial_interface.hh>
#include <labdev/devices/fgen.hh>

namespace labdev {

/*
 * FeelTech FY6900 series function generator
 */

class fy6900 : public fgen {
public:
    fy6900(const serial_config ser);
    ~fy6900() {};

    static constexpr unsigned BAUD = 115200;

    /* Definition of generic function generator methods */

    // Turn channel on/off
    void enable_channel(unsigned channel, bool ena = true) override;
    // Get output state of channel (true = on, false = off)
    bool get_state(unsigned channel) override;

    // Set/get waveforms
    void set_sine(unsigned channel,float freq_hz, float ampl_v = 1., 
        float offset_v = 0., float phase_deg = 0.) override;
    void set_square(unsigned channel,float freq_hz, float ampl_v = 1., 
        float offset_v = 0., float phase_deg = 0., float duty_cycle = 0.5) override;
    void set_ramp(unsigned channel,float freq_hz, float ampl_v = 1., 
        float offset_v = 0., float phase_deg = 0., float symm = 0.5) override;
    void set_pulse(unsigned channel,float period_s, float width_s, 
        float delay_s = 0., float high_v = .1, float low_v = 0., 
        float rise_s = 1e-6, float fall_s = 1e-6) override;
    void set_noise(unsigned channel,float mean_v = 0.,
        float stdev_v = 0.1) override;

    bool is_sine(unsigned channel) override;
    bool is_square(unsigned channel) override;
    bool is_ramp(unsigned channel) override;
    bool is_pulse(unsigned channel) override;
    bool is_noise(unsigned channel) override;

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
    // Private default ctor
    fy6900() : fgen(2, "FeelTech,FY6900") {};

    // Set/get waveform according to number (manual p.6ff)
    void set_waveform(unsigned channel, unsigned wvfm);
    unsigned get_waveform(unsigned channel);

    // Set pulse width (only valid with pulse-waveforms)
    void set_pulse_width(unsigned channel, float width_s);    

};
 
}

#endif