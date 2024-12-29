#ifndef DG4000_H
#define DG4000_H

#include <labdev/tcpip_iface.hh>
#include <labdev/visa_iface.hh>
#include <labdev/usbtmc_iface.hh>
#include <labdev/devices/fgen.hh>
#include <labdev/protocols/scpi.hh>
#include <memory>
#include <map>

namespace labdev {

/*
 *      Rigol DG4000 series function generator
 */
class dg4000 : public fgen {
public:
    dg4000() : fgen(2, "Rigol,DG4000") {};
    dg4000(std::unique_ptr<tcpip_iface> tcpip);
    dg4000(std::unique_ptr<usbtmc_iface> usbtmc);
    dg4000(std::unique_ptr<visa_iface> visa);
    ~dg4000();

    void connect(std::unique_ptr<ld_iface> comm) override;
    void disconnect() override;

    static constexpr uint16_t DG4162_VID = 0x1AB1;
    static constexpr uint16_t DG4162_PID = 0x0641;
    static constexpr unsigned PORT = 5555;

    // Turn channel on/off
    void enable_channel(unsigned channel, bool ena = true) override;
    // Get output state of channel (true = on, false = off)
    bool get_state(unsigned channel) override;

    // Waveform
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
    void init();

    // Aborts if channel is invalid
    void check_channel(unsigned channel);

    // Returns when at least time_ms have passed after sending msg
    void write_at_least(std::string msg, unsigned time_ms);

    std::map<waveform, std::string> m_wvfm_string {
        {SINE, "SIN"}, {SQUARE, "SQU"}, {RAMP, "RAMP"}, {PULSE, "PULS"}, 
        {NOISE, "NOIS"}, {DC, "DC"}
    };
    std::string wvfm_to_str(waveform item) { return m_wvfm_string[item]; }

};

}

#endif