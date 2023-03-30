#ifndef OM70_L_HH
#define OM70_L_HH

#include <labdev/devices/device.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/protocols/modbus_tcp.hh>
#include <vector>

namespace labdev {

    class om70_l : public device {
    public:
        om70_l() : device("Baumer,OM70-L") {};
        om70_l(ip_address &ip);
        om70_l(const om70_l&) = delete;
        ~om70_l();

        // OM70 default port 502
        static constexpr unsigned PORT = 502;

        void connect(ip_address &ip);

        // Get single measurement; get_measurement() updates quality, sample
        // rate, exposure, and delay
        float get_measurement();
        int get_quality();
        float get_sample_rate();
        float get_exposure();
        float get_delay();

        // Get 100 measurements from memory; get_measurement_mem() updates 
        // quality, sample rate, exposure, and delay
        std::vector<float> get_measurement_mem();
        std::vector<int> get_quality_mem();
        std::vector<float> get_sample_rate_mem();
        std::vector<float> get_exposure_mem();
        std::vector<float> get_delay_mem();

    private:
        static constexpr uint8_t UNIT_ID = 0x01;

        modbus_tcp* m_modbus;
        int m_quality {0};
        float m_dist {0}, m_sr {0}, m_exp {0}, m_delay_ms {0};
        std::vector<int> m_quality_vec {};
        std::vector<float> m_dist_vec {}, m_sr_vec {}, m_exp_vec {}, m_delay_ms_vec {};
    };
}

#endif