#include <labdev/devices/baumer/om70_l.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/exceptions.hh>

namespace labdev {

    om70_l::om70_l(tcpip_interface* tcpip) {
        this->open(tcpip);
        return;
    }

    void om70_l::open(tcpip_interface* tcpip) {
        if ( this->good() ) {
            fprintf(stderr, "Device is already connected!\n");
            abort();
        }
        // Default port 502
        if (tcpip->get_port() != om70_l::PORT) {
            fprintf(stderr, "OM70-L only supports port %u.\n", om70_l::PORT);
            abort();
        }
        m_comm = tcpip;
        return;
    }

    float om70_l::get_distance() {
        this->get_measurement();
        return m_distance;
    }   

    float om70_l::get_distance(int& quality, float& sample_rate, float& exposure,
    float& delay) {
        this->get_measurement();

        quality = m_signal_quality;
        sample_rate = m_sample_rate;
        exposure = m_exposure;
        delay = m_response_delay_ms;

        return m_distance;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    int om70_l::read_input_registers(uint16_t addr, uint16_t size, uint16_t* data) {
        uint8_t msg[12];
        // Modbus protocol
        msg[0] = (uint8_t)(m_msg_id >> 8);
        msg[1] = (uint8_t)(m_msg_id & 0xFF);
        msg[2] = 0x00;  // upper byte protocol ID
        msg[3] = 0x00;  // lower byte protocol ID
        msg[4] = 0x00;  // upper byte len
        msg[5] = 0x06;  // lower byte len
        msg[6] = 0x01;  // unit ID
        msg[7] = 0x04;  // function code FC 04
        msg[8] = (uint8_t)(addr >> 8);
        msg[9] = (uint8_t)(addr & 0xFF);
        msg[10] = (uint8_t)(size >> 8);
        msg[11] = (uint8_t)(size & 0xFF);

        // Send request to read and read
        m_comm->write_raw(msg, sizeof(msg));
        uint8_t resp[MAX_MSG_LEN] = {0};
        m_comm->read_raw(resp, MAX_MSG_LEN);

        // Check response
        if ( (resp[0] != msg[0]) &&  (resp[1] != msg[1]))
            throw bad_protocol("Wrong message ID received");
        if ( (resp[2] != msg[2]) &&  (resp[3] != msg[3]))
            throw bad_protocol("Wrong protocol ID received");
        if (resp[6] != msg[6])
            throw bad_protocol("Wrong unit ID received", resp[6]);
        if (resp[7] != msg[7])
            throw bad_protocol("Wrong function code received", resp[7]);
        if (resp[8] != 2*size)
            throw bad_protocol("Wrong number of bytes received");

        for (int i = 0; i < size; i++)
            data[i] = (resp[9+2*i] << 8) | (resp[9+2*i+1]);

        // Increase message ID
        if (m_msg_id < 0xFFFF)
            m_msg_id++;
        else m_msg_id = 0;

        return size;
    }

    void om70_l::get_measurement() {
        uint16_t msg[MAX_MSG_LEN];
        // Input register "All Measurements" (manual p. 60)
        this->read_input_registers(0x00C8, 0x0011, msg);

        // Reg 1 = signal quality
        m_signal_quality = msg[1];

        // Reg 3 & 4 = distance measurement [mm]
        uint32_t val = (uint32_t)(msg[3] | (msg[4] << 16));
        memcpy(&m_distance, &val, sizeof(float));
        // Reg 5 & 6 = sample rate [Hz]
        val = (uint32_t)(msg[5] | (msg[6] << 16));
        memcpy(&m_sample_rate, &val, sizeof(float));
        // Reg 7 & 8 = exposure reserve [a.u.]
        val = (uint32_t)(msg[7] | (msg[8] << 16));
        memcpy(&m_exposure, &val, sizeof(float));
        // Reg 9 & 10 = responde delay seconds [s]
        uint32_t delay_s = (uint32_t)(msg[9] | (msg[10] << 16));
        // Reg 11 & 12 = responde delay microseconds [us]
        uint32_t delay_us = (uint32_t)(msg[11] | (msg[12] << 16));
        m_response_delay_ms = delay_s*1000. + delay_us/1000.;

        return;
    }

}