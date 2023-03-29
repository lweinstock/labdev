#ifndef MODBUS_TCP_HH
#define MODBUS_TCP_HH

#include <labdev/tcpip_interface.hh>
#include <vector>

namespace labdev {

    class modbus_tcp {
    public:
        modbus_tcp(tcpip_interface* ip);
        ~modbus_tcp();

        // Function Code 01; read coils -> returns true = on, false = off
        std::vector<bool> read_coils(uint8_t uid, uint16_t addr, uint16_t len);

        // Function Code 02; read discrete inputs
        std::vector<bool> read_discrete_inputs(uint8_t uid, uint16_t addr, 
            uint16_t len);

        // Function Code 03; read multiple holding registers
        std::vector<uint16_t> read_multiple_holding_regs(uint8_t uid, 
            uint16_t addr, uint16_t len);

        // Function Code 04; read input registers
        std::vector<uint16_t> read_input_regs(uint8_t uid, uint16_t addr, 
            uint16_t len);

        // Function Code 05; write single coil -> on = true, off = false
        void write_single_coil(uint8_t uid, uint16_t addr, bool ena);    

        // Function Code 06; write single holding register
        void write_single_holding_reg(uint8_t uid, uint16_t addr, uint16_t data);

        // Function Code 15; write multiple coils -> on = true, off = false
        void write_multiple_coils(uint8_t uid, uint16_t addr, 
            std::vector<bool> ena);

        // Function Code 16; write multiple holding registers
        void write_multiple_holding_regs(uint8_t uid, uint16_t addr, 
            std::vector<uint16_t> data);

    private:
        tcpip_interface* m_comm;

        uint16_t m_tid;

        // Modbus function codes
        static constexpr uint8_t FC01 = 0x01;
        static constexpr uint8_t FC02 = 0x02;
        static constexpr uint8_t FC03 = 0x03;
        static constexpr uint8_t FC04 = 0x04;
        static constexpr uint8_t FC05 = 0x05;
        static constexpr uint8_t FC06 = 0x06;
        static constexpr uint8_t FC15 = 0x0F;
        static constexpr uint8_t FC16 = 0x10;

        // Container to hold and format modbus tcp messages
        struct tcp_frame {
            tcp_frame(std::vector<uint8_t> msg);
            tcp_frame(uint16_t trans_id, uint8_t uid, uint8_t func, 
                std::vector<uint8_t> payload);
            ~tcp_frame() {};

            std::vector<uint8_t> get_frame();

            uint16_t transaction_id, protocol_id, length;
            uint8_t function_code, unit_id, byte_count;
            std::vector<uint8_t> data;
        };

    };

}

#endif
