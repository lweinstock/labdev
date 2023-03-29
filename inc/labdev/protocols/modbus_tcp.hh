#ifndef MODBUS_TCP_HH
#define MODBUS_TCP_HH

#include <labdev/tcpip_interface.hh>
#include <vector>

namespace labdev {

    class modbus_tcp {
    public:
        modbus_tcp(tcpip_interface &ip);
        ~modbus_tcp();

        // Function Code 01; read coils -> returns true = on, false = off
        std::vector<bool> read_coils(uint16_t addr, uint16_t len);

        // Function Code 02; read discrete inputs
        std::vector<uint8_t> read_discrete_inputs(uint16_t addr, uint16_t len);

        // Function Code 03; read multiple holding registers
        std::vector<uint16_t> read_multiple_holding_regs(uint16_t addr, uint16_t len);

        // Function Code 04; read input registers
        std::vector<uint16_t> read_input_regs(uint16_t addr, uint16_t len);

        // Function Code 05; write single coil -> on = true, off = false
        void write_single_coil(uint16_t addr, bool ena);    

        // Function Code 06; write single holding register
        void write_single_holding_reg(uint16_t addr, uint16_t data);

        // Function Code 15; write multiple coils -> on = true, off = false
        void write_multiple_coils(uint16_t addr, std::vector<bool> ena);

        // Function Code 16; write multiple holding registers
        void write_multiple_holding_regs(uint16_t addr, std::vector<uint16_t> data);

    private:
        tcpip_interface* m_comm;

        // Container to hold and format modbus tcp messages
        struct tcp_frame {
            tcp_frame(std::vector<uint8_t> message);
            tcp_frame(uint16_t transaction_id, uint8_t unit_id, uint16_t fc, 
                std::vector<uint8_t> data);
            ~tcp_frame() {};

            std::vector<uint8_t> get_frame() 
            { 
                std::vector<uint8_t> ret;
                return ret;
            }

            uint16_t transaction_id, function_code;
            uint8_t unit_id;
            std::vector<uint8_t> data;
        };
    };

}

#endif
