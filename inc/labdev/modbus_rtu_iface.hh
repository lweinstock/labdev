#ifndef MODBUS_RTU_HH
#define MODBUS_RTU_HH

#include <labdev/serial_port.hh>
#include <labdev/modbus_iface.hh>
#include <vector>

namespace labdev {

/** \brief MODBUS Remote Terminal Unit (RTU) implementation
 *
 *  Implementation of a mininal version of MODBUS RTU to control lab devices.
 *  Great source for MODBUS protocol: https://www.modbustools.com/modbus.html
 */
class modbus_rtu_iface : public serial_port, public modbus_iface 
{
public:
    modbus_rtu_iface() : serial_port(), modbus_iface() {};
    modbus_rtu_iface(std::string path, unsigned baud = 9600, unsigned nbits = 8,
        bool par_ena = false, bool par_even = false, unsigned stop_bits = 1)
      : serial_port(path, baud, nbits, par_ena, par_even, stop_bits), 
        modbus_iface() {};
    ~modbus_rtu_iface() {};

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

    /// Used for reading holding and input registers
    std::vector<uint16_t> read_16bit_regs(uint8_t uid, uint8_t func,
        uint16_t addr, uint16_t len);

    /// Calculate 16 bit CRC for modbus (copied from stackoverflow...)
    static uint16_t calc_crc16(std::vector<uint8_t> data);

    /// Container to hold and format modbus rtu messages
    struct frame {
        /// Create frame from raw message
        frame(std::vector<uint8_t> msg);
        /// Create frame with given address, function code, and payload
        frame(uint8_t addr, uint8_t func, std::vector<uint8_t> payload);
        ~frame() {};

        /// Returns vector of bytes formatted according to modbus rtu protocol
        std::vector<uint8_t> get();

        /// If the highest bit of the function code is set, the payload contains 
        /// the error code
        uint8_t get_error() { return data.at(0); }

        uint8_t address, function_code;
        uint16_t crc;
        std::vector<uint8_t> data;
    };

};

}

#endif