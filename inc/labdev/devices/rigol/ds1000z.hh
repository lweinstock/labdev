#ifndef DS1000Z_H
#define DS1000Z_H

#include <labdev/devices/osci.hh>
#include <labdev/protocols/scpi.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/usbtmc_interface.hh>
#include <labdev/visa_interface.hh>
#include <memory>

namespace labdev {

/*
 *  Rigol DS1000Z series oscilloscope
 */

class ds1000z : public osci {
public:
    ds1000z();
    ds1000z(tcpip_interface* tcpip);
    ds1000z(usbtmc_interface* usbtmc);
    ds1000z(visa_interface* visa);
    ~ds1000z() {};

    void connect(tcpip_interface* tcpip);
    void connect(usbtmc_interface* usbtmc);
    void connect(visa_interface* visa);

    static constexpr uint16_t DS1104_VID = 0x1AB1;
    static constexpr uint16_t DS1104_PID = 0x04CE;
    static constexpr uint16_t PORT = 5555;

    enum measurement_item : unsigned {
        MEAS_VMAX,
        MEAS_VMIN,
        MEAS_VPP,
        MEAS_VTOP,
        MEAS_VBAS,
        MEAS_VAMP,
        MEAS_VAVG,
        MEAS_VRMS,
        MEAS_OVER,
        MEAS_PRES,
        MEAS_MAR,
        MEAS_MPAR,
        MEAS_PER,
        MEAS_FREQ,
        MEAS_RTIM,
        MEAS_FTIM,
        MEAS_PWID,
        MEAS_NWID,
        MEAS_PDUT,
        MEAS_NDUT,
        MEAS_RDEL,
        MEAS_FDEL,
        MEAS_RPH,
        MEAS_FPH
    };

    enum measurement_type : unsigned {
        MEAS_MAX,
        MEAS_MIN,
        MEAS_CUR,
        MEAS_AVG,
        MEAS_STD
    };

    /* Definition of generic oscilloscope functions */

    // Turn channel on/off
    void enable_channel(unsigned channel, bool enable = true) override;

    // Attenuation settings
    void set_atten(unsigned channel, double att) override;
    double get_atten(unsigned channel) override;

    // Vertical settings
    void set_vert_base(unsigned channel, double volts_per_div) override;
    double get_vert_base(unsigned channel) override;

    // Horizontal settings
    void set_horz_base(double sec_per_div) override;
    double get_horz_base() override;

    // Acquisition settings
    void start_acquisition() override;
    void stop_acquisition() override;
    void single_shot() override;

    // Edge trigger settings
    void set_trigger_source(unsigned channel) override;
    void set_trigger_type(trigger_type trig) override;
    void set_trigger_level(double level) override;

    // Returns true if trigger conditions have been met
    bool triggered() override;
    // Returns true if data acquisition has stopped
    bool stopped() override;

    // Read sample data
    void read_sample_data(unsigned channel,  
        std::vector<double> &horz_data, std::vector<double> &vert_data) override;

    /* Definition of DS1000Z series specific functions */

    // Statistics measurement setup and readout
    void set_measurement(unsigned channel, unsigned item);
    void set_measurement(unsigned channel1, unsigned channel2,
        unsigned item);
    double get_measurement(unsigned channel, unsigned item,
        unsigned type);
    double get_measurement(unsigned channel1, unsigned channel2,
        unsigned item, unsigned type);
    void clear_measurements();
    void reset_measurements();

private:
    void init();
    void check_channel(unsigned channel);
    std::vector<uint8_t> read_mem_data( unsigned sta, unsigned sto);

    std::unique_ptr<scpi> m_scpi;

    unsigned m_npts;
    double m_xincr, m_xorg, m_xref, m_yinc;
    int m_yorg, m_yref;

    static const std::string s_meas_item_string[];
    static const std::string s_meas_type_string[];
};

}

#endif