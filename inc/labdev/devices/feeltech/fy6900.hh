#ifndef FY6900_HH
#define FY6900_HH

#include <labdev/serial_interface.hh>
#include <labdev/devices/fgen.hh>
#include <map>

namespace labdev {

/*
 * FeelTech FY6900 series function generator
 */

class fy6900 : public fgen {
public:
    fy6900() : fgen(2, "FeelTech,FY6900") {};
    fy6900(std::unique_ptr<serial_interface> ser);
    ~fy6900();

    void connect(std::unique_ptr<ld_interface> comm) override;
    void disconnect() override { m_comm.reset(); }

    static constexpr unsigned BAUD = 115200;

    /* Definition of generic function generator methods */

    // Turn channel on/off
    void enable_channel(unsigned channel, bool ena = true) override;
    // Get output state of channel (true = on, false = off)
    bool get_state(unsigned channel) override;

    // Waveforms
    void set_wvfm(unsigned channel, waveform wvfm) override;
    waveform get_wvfm(unsigned channel) override;

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

    // Rising and falling edges
    void set_rising(unsigned channel, float rise_s) override;
    float get_rising(unsigned channel) override;
    void set_falling(unsigned channel, float fall_s) override;
    float get_falling(unsigned channel) override;

    // Pulse width
    void set_pulse_width(unsigned channel, float width_s) override;
    float get_pulse_width(unsigned channel) override;

private:
    std::map<waveform, unsigned> m_wvfm_no {
        {SINE, 0}, {SQUARE, 1}, {RAMP, 8}, {PULSE, 5}, 
        {NOISE, 27}, {DC, 6}
    };
    unsigned wvfm_to_no(waveform item) { return m_wvfm_no[item]; }
};
 
}

#endif