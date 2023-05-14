#ifndef DG4000_H
#define DG4000_H

#include <labdev/tcpip_interface.hh>
#include <labdev/visa_interface.hh>
#include <labdev/usbtmc_interface.hh>
#include <labdev/devices/fgen.hh>
#include <labdev/protocols/scpi.hh>

namespace labdev {

/*
 *      Rigol DG4000 series function generator
 */

class dg4000 : public fgen {
public:
    dg4000(const ip_address ip);
    dg4000(const visa_identifier visa_id);
    dg4000(const usb_config usb);
    ~dg4000() {};

    static constexpr uint16_t VID = 0x1AB1;
    static constexpr uint16_t PID = 0x0641;
    static constexpr unsigned PORT = 5555;

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
    dg4000() : fgen(2, "Rigol,DG4000"), m_scpi(nullptr) {};

    std::unique_ptr<scpi> m_scpi;

    void init();

    // Aborts if channel is invalid
    void check_channel(unsigned channel);

    // Returns waveform string 
    std::string get_waveform_str(unsigned channel);

    // Returns when at least time_ms have passed after sending msg
    void write_at_least(std::string msg, unsigned time_ms);

};

}

#endif