#ifndef MODBUS_TCP_HH
#define MODBUS_TCP_HH

#include <labdev/tcpip_iface.hh>
#include <labdev/modbus_iface.hh>
#include <vector>

namespace labdev {

class modbus_tcp_iface : public tcpip_iface, public modbus_iface 
{
public:
    modbus_tcp_iface() : tcpip_iface(), modbus_iface(), m_tid(0x0000) {};
    modbus_tcp_iface(std::string ip_addr, unsigned port)
      : tcpip_iface(ip_addr, port), modbus_iface(), m_tid(0x0000) {};
    ~modbus_tcp_iface() {};

    Interface_type type() const noexcept override { return MODBUS_TCP; }

    // Function Code 01; read coils -> returns true = on, false = off
    std::vector<bool> read_coils(uint8_t uid, uint16_t addr, uint16_t len) override;

    // Function Code 02; read discrete inputs
    std::vector<bool> read_discrete_inputs(uint8_t uid, uint16_t addr, 
        uint16_t len) override;

    // Function Code 03; read multiple holding registers
    std::vector<uint16_t> read_multiple_holding_regs(uint8_t uid, 
        uint16_t addr, uint16_t len) override;

    // Function Code 04; read input registers
    std::vector<uint16_t> read_input_regs(uint8_t uid, uint16_t addr, 
        uint16_t len) override;

    // Function Code 05; write single coil -> on = true, off = false
    void write_single_coil(uint8_t uid, uint16_t addr, bool ena) override;    

    // Function Code 06; write single holding register
    void write_single_holding_reg(uint8_t uid, uint16_t addr, uint16_t data) override;

    // Function Code 15; write multiple coils -> on = true, off = false
    void write_multiple_coils(uint8_t uid, uint16_t addr, 
        std::vector<bool> ena) override;

    // Function Code 16; write multiple holding registers
    void write_multiple_holding_regs(uint8_t uid, uint16_t addr, 
        std::vector<uint16_t> data) override;

private:
    // Transaction id
    uint16_t m_tid;

    // Used for reading holding and input registers
    std::vector<uint16_t> read_16bit_regs(uint8_t uid, uint8_t func,
        uint16_t addr, uint16_t len);

    void increase_tid_counter();
    
    void check_error_code(uint8_t error);

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