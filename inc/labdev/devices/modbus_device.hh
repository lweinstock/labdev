#ifndef LD_MODBUS_DEVICE_HH
#define LD_MODBUS_DEVICE_HH

#include <string>
#include <cstdint>
#include <vector>

#include <labdev/devices/ld_device.hh>

namespace labdev 
{

class modbus_device : public ld_device
{
public:
    virtual ~modbus_device() {};

    // No copy constructor or assignment, default move constructor
    modbus_device(const modbus_device&) = delete;
    modbus_device& operator=(const modbus_device&) = delete;

    /// Function Code 01; read coils -> returns true = on, false = off
    std::vector<bool> read_coils(uint8_t unit_id, uint16_t addr, 
        uint16_t len);

    /// Function Code 02; read discrete inputs
    std::vector<bool> read_discrete_inputs(uint8_t unit_id, uint16_t addr, 
        uint16_t len);

    /// Function Code 03; read multiple holding registers
    std::vector<uint16_t> read_multiple_holding_regs(uint8_t unit_id, 
        uint16_t addr, uint16_t len);

    /// Function Code 04; read input registers
    std::vector<uint16_t> read_input_regs(uint8_t unit_id, uint16_t addr, 
        uint16_t len);

    /// Function Code 05; write single coil -> on = true, off = false
    void write_single_coil(uint8_t unit_id, uint16_t addr, bool ena);    

    /// Function Code 06; write single holding register
    void write_single_holding_reg(uint8_t unit_id, uint16_t addr, 
        uint16_t reg);

    /// Function Code 15; write multiple coils -> on = true, off = false
    void write_multiple_coils(uint8_t unit_id, uint16_t addr, 
        std::vector<bool> ena);

    /// Function Code 16; write multiple holding registers
    void write_multiple_holding_regs(uint8_t unit_id, uint16_t addr, 
        std::vector<uint16_t> regs);

protected:
    // Initializer with name for derived classes
    modbus_device() : ld_device() {};
    modbus_device(std::string name) : ld_device(name) {};
    
private:
    // Modbus function codes
    static constexpr uint8_t FC01 = 0x01;   ///< Read coils
    static constexpr uint8_t FC02 = 0x02;   ///< Read discrete inputs
    static constexpr uint8_t FC03 = 0x03;   ///< Read multiple holding registers
    static constexpr uint8_t FC04 = 0x04;   ///< Read input registers
    static constexpr uint8_t FC05 = 0x05;   ///< Write single coil
    static constexpr uint8_t FC06 = 0x06;   ///< Write single holding register
    static constexpr uint8_t FC15 = 0x0F;   ///< Write multiple coils
    static constexpr uint8_t FC16 = 0x10;   ///< Write multiple holding registers
    // Error codes
    static constexpr uint8_t ERRC = 0x80;   ///< Function code for errors
    static constexpr uint8_t ERR1 = 0x01;   ///< Illegal Function
    static constexpr uint8_t ERR2 = 0x02;   ///< Illegal Data Address
    static constexpr uint8_t ERR3 = 0x03;   ///< Illegal Data Value
    static constexpr uint8_t ERR4 = 0x04;   ///< Slave Device Failure

    /// Transaction ID used by MODBUS TCP
    uint16_t m_tid {0x0000};

    /// Returns CRC sum used by MODBUS RTU
    uint16_t calc_crc16(std::vector<uint8_t> data);

    /// Returns MODBUS packet
    std::vector<uint8_t> create_packet(uint8_t unit_id, uint8_t function_code, 
        std::vector<uint8_t> &data);

    /// Read 16 bit registers; used by FC03 & FC04
    std::vector<uint16_t> read_16bit_regs(uint8_t unit_id, uint8_t function_code,
        uint16_t start_addr, uint16_t len);

    /// Check error codes and throw corresponding exception
    void check_and_throw(uint8_t error);

};

}

#endif