#ifndef MODBUS_INTERFACE_HH
#define MODBUS_INTERFACE_HH

#include <vector>

class modbus_interface 
{
public:
    modbus_interface() {};
    virtual ~modbus_interface() {};

    // Function Code 01; read coils -> returns true = on, false = off
    virtual std::vector<bool> read_coils(uint8_t uid, uint16_t addr, 
        uint16_t len) = 0;

    // Function Code 02; read discrete inputs
    virtual std::vector<bool> read_discrete_inputs(uint8_t uid, uint16_t addr, 
        uint16_t len) = 0;

    // Function Code 03; read multiple holding registers
    virtual std::vector<uint16_t> read_multiple_holding_regs(uint8_t uid, 
        uint16_t addr, uint16_t len) = 0;

    // Function Code 04; read input registers
    virtual std::vector<uint16_t> read_input_regs(uint8_t uid, uint16_t addr, 
        uint16_t len) = 0;

    // Function Code 05; write single coil -> on = true, off = false
    virtual void write_single_coil(uint8_t uid, uint16_t addr, bool ena) = 0;    

    // Function Code 06; write single holding register
    virtual void write_single_holding_reg(uint8_t uid, uint16_t addr, 
        uint16_t data) = 0;

    // Function Code 15; write multiple coils -> on = true, off = false
    virtual void write_multiple_coils(uint8_t uid, uint16_t addr, 
        std::vector<bool> ena) = 0;

    // Function Code 16; write multiple holding registers
    virtual void write_multiple_holding_regs(uint8_t uid, uint16_t addr, 
        std::vector<uint16_t> data) = 0;
    
    // Modbus function codes
    static constexpr uint8_t FC01 = 0x01;
    static constexpr uint8_t FC02 = 0x02;
    static constexpr uint8_t FC03 = 0x03;
    static constexpr uint8_t FC04 = 0x04;
    static constexpr uint8_t FC05 = 0x05;
    static constexpr uint8_t FC06 = 0x06;
    static constexpr uint8_t FC15 = 0x0F;
    static constexpr uint8_t FC16 = 0x10;
    // Error codes
    static constexpr uint8_t ERRC = 0x80;
    static constexpr uint8_t ERR1 = 0x01;
    static constexpr uint8_t ERR2 = 0x02;
    static constexpr uint8_t ERR3 = 0x03;
    static constexpr uint8_t ERR4 = 0x04;

protected:
    void check_error_code(uint8_t error);
};

#endif