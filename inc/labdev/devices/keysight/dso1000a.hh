#ifndef DS01000A_H
#define DS01000A_H

#include <labdev/devices/osci.hh>
#include <labdev/protocols/scpi.hh>
#include <labdev/usbtmc_interface.hh>
#include <labdev/visa_interface.hh>
#include <memory>
#include <map>

namespace labdev {

/*
 *  Keysight 1000 series oscilloscope
 */

class dso1000a : public osci {
public:
    dso1000a();
    dso1000a(usbtmc_interface* usbtmc);
    dso1000a(visa_interface* visa);
    ~dso1000a();

    void connect(usbtmc_interface* usbtmc);
    void connect(visa_interface* visa);

    void disconnect() override;

    static constexpr uint16_t DSO1024A_VID = 0x0957;
    static constexpr uint16_t DSO1024A_PID = 0x0588;

    /* Definition of generic oscilloscope functions */

    // Turn channel on/off
    void enable_channel(unsigned channel, bool enable = true) override;
    bool channel_enabled(unsigned channel) override;

    // Attenuation settings
    void set_atten(unsigned channel, double att) override;
    double get_atten(unsigned channel) override;

    // Vertical settings
    void set_vert_base(unsigned channel, double volts_per_div) override;
    double get_vert_base(unsigned channel) override;
    void set_vert_offs(unsigned channel, double offset_v) override;
    double get_vert_offs(unsigned channel) override;

    // Horizontal settings
    void set_horz_base(double sec_per_div) override;
    double get_horz_base() override;
    void set_horz_offs(double offset_s) override;
    double get_horz_offs() override;

    // Single and dual source measurements
    void set_meas(unsigned ch, meas_item meas) override;
    double get_meas(unsigned ch, meas_item meas) override;
    void set_meas(unsigned ch1, unsigned ch2, meas_item meas) override;
    double get_meas(unsigned ch1, unsigned ch2, meas_item meas) override;
    void clear_meas() override;

    // Acquisition settings
    void run() override;
    void stop() override;
    void single_shot() override;

    // Edge trigger settings
    void set_trigger_source(unsigned channel) override;
    void set_trigger_type(trig_type trig) override;
    void set_trigger_level(double level) override;

    // Returns true if trigger conditions have been met
    bool triggered() override;
    // Returns true if data acquisition has stopped
    bool stopped() override;

    // Read sample data
    void read_sample_data(unsigned channel,  
        std::vector<double> &horz_data, std::vector<double> &vert_data) override;

    /* Definition of DS1000Z series specific functions */

private:
    void init();
    void check_channel(unsigned channel);
    std::vector<uint8_t> read_mem_data();

    std::map<meas_item, std::string> m_meas_string {
        {VMAX, "VMAX"}, {VMIN, "VMIN"}, {VPP, "VPP"}, {VTOP, "VTOP"},
        {VBASE, "VBAS"}, {VAMP, "VAMP"}, {VAVG, "VAV"}, {VRMS, "VRMS"}, 
        {OVERSHOOT, "OVER"}, {PRESHOOT, "PRES"}, {FREQ, "FREQ"}, 
        {RISETIME, "RIS"}, {FALLTIME, "FALL"}, {POS_WIDTH, "PWID"}, 
        {NEG_WIDTH, "NWID"}, {POS_DUTY, "PDUT"}, {NEG_DUTY, "NDUT"}, 
        {POS_DELAY, "PDEL"}, {NEG_DELAY, "NDEL"}, {POS_PHASE, "PPHA"}, 
        {NEG_PHASE, "NPHA"}
    };
    std::string meas_to_str(meas_item item) { return m_meas_string[item]; }

    scpi* m_scpi;
};

}

#endif