#ifndef OM70_L_HH
#define OM70_L_HH

#include <labdev/devices/device.hh>
#include <labdev/tcpip_interface.hh>

namespace labdev {

    class om70_l : public device {
    public:
        om70_l() : device("OM70-L") {};
        om70_l(ip_address &ip);
        om70_l(const om70_l&) = delete;
        ~om70_l() {};

        // OM70 default port 502
        static constexpr unsigned PORT = 502;

        void connect(ip_address &ip);

        // Returns distance in mm
        float get_distance();
        float get_distance(int& quality, float& sample_rate, float& exposure,
            float& delay);

    private:
        static constexpr unsigned MAX_MSG_LEN {264};

        uint16_t m_msg_id {0x0000}, m_signal_quality {0x0000};
        float m_distance {0}, m_sample_rate {0}, m_exposure {0};
        float m_response_delay_ms {0};

        // Modbus TCP FC 04: read input register
        int read_input_registers(uint16_t addr, uint16_t size, uint16_t* data);

        // Update measurements
        void get_measurement();
    };
}

#endif