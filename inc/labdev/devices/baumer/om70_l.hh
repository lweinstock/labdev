#ifndef OM70_L_HH
#define OM70_L_HH

#include <labdev/devices/device.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/protocols/modbus_tcp.hh>
#include <vector>

namespace labdev {

class om70_l : public device {
public:
    om70_l();
    om70_l(ip_address &ip);
    ~om70_l();

    // OM70 default port 502
    static constexpr unsigned PORT = 502;

    void connect(ip_address &ip);

    void enable_laser(bool ena = true);
    void disable_laser() { this->enable_laser(false); }

    // Get single measurement; get_measurement() updates quality, sample
    // rate, exposure, and delay
    float get_measurement();                    // returns distance in [mm]
    int get_quality() { return m_quality; }     // returns 0, 1, 2 for good, medium, bad quality
    float get_sample_rate() { return m_sr; }    // returns sample rate in [Hz]
    float get_exposure() { return m_exp; }      // arb unit, high means good, low means bad

    // Get 100 measurements from memory; get_measurement_mem() updates 
    // quality, sample rate, exposure, and delay
    std::vector<float> get_measurement_mem();
    std::vector<int> get_quality_mem() { return m_quality_vec; }
    std::vector<float> get_sample_rate_mem() { return  m_sr_vec; }
    std::vector<float> get_exposure_mem() { return m_exp_vec; }

private:
    static constexpr uint8_t UNIT_ID = 0x01;

    // Address space holding registers (FC03/06/16)
    static constexpr uint16_t ADDR_CONF_ON = 0x0000;
    static constexpr uint16_t ADDR_CONF_OFF = 0x0001;
    static constexpr uint16_t ADDR_ENA_LASER = 0x019A;
    // Address space input registers (FC04)
    static constexpr uint16_t ADDR_VEND_INFO = 0x0000;
    static constexpr uint16_t ADDR_DEV_INFO = 0x0028;
    static constexpr uint16_t ADDR_ALL_MEAS = 0x00C8;
    // Block memory input registers
    static constexpr uint16_t ADDR_BLK_MEM0 = 0x0258;
    static constexpr uint16_t ADDR_BLK_MEM1 = 0x02C8;
    static constexpr uint16_t ADDR_BLK_MEM2 = 0x0338;
    static constexpr uint16_t ADDR_BLK_MEM3 = 0x03A8;
    static constexpr uint16_t ADDR_BLK_MEM4 = 0x0418;
    static constexpr uint16_t ADDR_BLK_MEM5 = 0x0488;
    static constexpr uint16_t ADDR_BLK_MEM6 = 0x04F8;
    static constexpr uint16_t ADDR_BLK_MEM7 = 0x0568;
    static constexpr uint16_t ADDR_BLK_MEM8 = 0x05D8;
    static constexpr uint16_t ADDR_BLK_MEM9 = 0x0648;
    static constexpr uint16_t ADDR_BLK_MEM10 = 0x06B8;
    static constexpr uint16_t ADDR_BLK_MEM11 = 0x0728;
    static constexpr uint16_t ADDR_BLK_MEM12 = 0x0798;
    static constexpr uint16_t ADDR_BLK_MEM13 = 0x0808;
    static constexpr uint16_t ADDR_BLK_MEM14 = 0x0878;

    // Extracts the measurements from the 16x 16-bit block memory
    // Warning: The format is (for some stupid reason) different to the
    //  "All Measurements" format (compare manual p.60 & p.66 WTF?!?!)
    void extract_mem_meas(std::vector<uint16_t> data, float &dist, 
        int &quality, float &sample_rate, float &exposure);

    std::shared_ptr<tcpip_interface> m_tcpip;
    modbus_tcp* m_modbus;
    int m_quality;
    float m_dist, m_sr, m_exp;
    std::vector<int> m_quality_vec;
    std::vector<float> m_dist_vec, m_sr_vec, m_exp_vec;
    bool m_config_mode;
};

}

#endif