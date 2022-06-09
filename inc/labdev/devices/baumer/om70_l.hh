#ifndef LD_OM70_L_HH
#define LD_OM70_L_HH

#include <labdev/tcpip_interface.hh>

namespace labdev {
    class om70_l {
    public:
        om70_l(tcpip_interface* tcpip);
        ~om70_l();

        // Returns distance in mm
        float get_distance(int& quality, float& sample_rate, float& exposure,
            float& delay);

    private:
        static constexpr unsigned MAX_MSG_LEN = 264;

        tcpip_interface* comm;
        uint16_t m_msg_id, m_signal_quality;
        float m_distance, m_sample_rate, m_exposure, m_response_delay_ms;

        // Modbus TCP FC 04: read input register
        int read_input_registers(uint16_t addr, uint16_t size, uint16_t* data);

        // Update measurements
        void get_measurement();

        void init();
    };
}

#endif