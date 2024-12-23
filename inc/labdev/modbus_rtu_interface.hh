#ifndef MODBUS_RTU_HH
#define MODBUS_RTU_HH

#include <labdev/serial_port.hh>
#include <labdev/modbus_interface.hh>
#include <vector>

namespace labdev {

class modbus_rtu_interface : public serial_port, public modbus_interface 
{
public:
    modbus_rtu_interface() : serial_port(), modbus_interface() {};
    modbus_rtu_interface(std::string path, unsigned baud = 9600, unsigned nbits = 8,
        bool par_ena = false, bool par_even = false, unsigned stop_bits = 1)
      : serial_port(path, baud, nbits, par_ena, par_even, stop_bits), 
        modbus_interface() {};
    ~modbus_rtu_interface() {};

    Interface_type type() const noexcept override { return MODBUS_RTU; }

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

};

}

#endif