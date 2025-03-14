#ifndef DS1000Z_H
#define DS1000Z_H

#include <labdev/devices/scpi_device.hh>
#include <labdev/devices/osci.hh>
#include <labdev/tcpip_iface.hh>
#include <labdev/usbtmc_iface.hh>
#include <labdev/visa_iface.hh>
#include <memory>
#include <map>

namespace labdev {

/*
 *  Rigol DS1000Z series oscilloscope
 */
class ds1000z : public osci, public scpi_device {
public:
    ds1000z() : osci(4), scpi_device("Rigol,DS1000Z") {};
    ds1000z(std::unique_ptr<tcpip_iface> tcpip);
    ds1000z(std::unique_ptr<usbtmc_iface> usbtmc);
    ds1000z(std::unique_ptr<visa_iface> visa);
    ~ds1000z();

    void connect(std::unique_ptr<ld_iface> comm) override;

    void disconnect() override;

    static constexpr uint16_t DS1104_VID = 0x1AB1;
    static constexpr uint16_t DS1104_PID = 0x04CE;
    static constexpr uint16_t PORT = 5555;

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
    void set_mem_range(unsigned sta, unsigned sto);
    std::vector<uint8_t> read_mem_data();

    std::map<meas_item, std::string> m_meas_string {
        {VMAX, "VMAX"}, {VMIN, "VMIN"}, {VPP, "VPP"}, {VTOP, "VTOP"},
        {VBASE, "VBAS"}, {VAMP, "VAMP"}, {VAVG, "VAVG"}, {VRMS, "VRMS"}, 
        {OVERSHOOT, "OVER"}, {PRESHOOT, "PRES"}, {FREQ, "FREQ"}, 
        {RISETIME, "RTIM"}, {FALLTIME, "FTIM"}, {POS_WIDTH, "PWID"}, 
        {NEG_WIDTH, "NWID"}, {POS_DUTY, "PDUT"}, {NEG_DUTY, "NDUT"}, 
        {POS_DELAY, "RDEL"}, {NEG_DELAY, "FDEL"}, {POS_PHASE, "RPH"}, 
        {NEG_PHASE, "FPH"}
    };
    std::string meas_to_str(meas_item item) { return m_meas_string[item]; }
};

}

#endif