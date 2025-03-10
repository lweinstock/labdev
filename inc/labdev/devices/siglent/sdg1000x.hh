#ifndef SDG1000X_HH
#define SDG1000X_HH

#include <labdev/devices/fgen.hh>
#include <labdev/devices/scpi_device.hh>
#include <labdev/tcpip_iface.hh>
#include <labdev/usbtmc_iface.hh>
#include <labdev/visa_iface.hh>
#include <map>

namespace labdev {

class sdg1000x: public fgen, public scpi_device {
public:
    sdg1000x() : fgen(2), scpi_device("Siglent,SDG1000X") {};
    sdg1000x(std::unique_ptr<tcpip_iface> tcpip);
    sdg1000x(std::unique_ptr<usbtmc_iface> usbtmc);
    sdg1000x(std::unique_ptr<visa_iface> visa);
    ~sdg1000x();

    void connect(std::unique_ptr<ld_iface> comm) override;
    void disconnect() override;

    static constexpr unsigned PORT = 5025;
    static constexpr uint16_t SDG1032X_VID = 0xF4EC;
    static constexpr uint16_t SDG1032X_PID = 0x1103;

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
    // Allowed channels = 1 or 2!
    void check_channel(unsigned channel);

    // Get value from basic wave command (manual p. 27)
    std::string get_bswv_val(std::string bswv, std::string par);

    std::map<waveform, std::string> m_wvfm_string {
        {SINE, "SINE"}, {SQUARE, "SQU"}, {RAMP, "RAMP"}, {PULSE, "PULS"}, 
        {NOISE, "NOIS"}, {DC, "DC"}
    };
    std::string wvfm_to_str(waveform item) { return m_wvfm_string[item]; }
};

}

#endif