#include <labdev/modbus_rtu_iface.hh>
#include <labdev/ld_debug.hh>

using namespace std;

namespace labdev 
{

vector<bool> modbus_rtu_iface::read_coils(uint8_t uid, uint16_t addr, 
    uint16_t len)
{
    vector<bool> ret;
    // TODO -> need device that actually uses this..
    return ret;
}

vector<bool> modbus_rtu_iface::read_discrete_inputs(uint8_t uid, uint16_t addr, 
    uint16_t len)
{
    vector<bool> ret;
    // TODO -> need device that actually uses this..
    return ret;
}

vector<uint16_t> modbus_rtu_iface::read_multiple_holding_regs(uint8_t uid, 
    uint16_t addr, uint16_t len)
{
    return this->read_16bit_regs(uid, FC03, addr, len);
}

vector<uint16_t> modbus_rtu_iface::read_input_regs(uint8_t uid, uint16_t addr, 
    uint16_t len)
{
    return this->read_16bit_regs(uid, FC04, addr, len);
}

void modbus_rtu_iface::write_single_coil(uint8_t uid, uint16_t addr, bool ena)
{
    // TODO -> need device that actually uses this..
    return;
}    

void modbus_rtu_iface::write_single_holding_reg(uint8_t uid, uint16_t addr, 
    uint16_t data)
{
    vector<uint8_t> payload {};
    payload.push_back(static_cast<uint8_t>(0xFF & (addr >> 8)));
    payload.push_back(static_cast<uint8_t>(0xFF & addr));
    payload.push_back(static_cast<uint8_t>(0xFF & (data >> 8)));
    payload.push_back(static_cast<uint8_t>(0xFF & data));
    frame sframe(uid, FC06, payload);
    this->write_byte(sframe.get());

    vector<uint8_t> resp = this->read_byte();
    frame rframe(resp);

    // Check error code
    if (rframe.function_code & ERRC)
        this->check_error_code(rframe.get_error());

    // TODO: check returned values (function code, address, etc. ...)

    return;
}

void modbus_rtu_iface::write_multiple_coils(uint8_t uid, uint16_t addr, 
    vector<bool> ena)
{
    // TODO -> need device that actually uses this..
    return;
}

void modbus_rtu_iface::write_multiple_holding_regs(uint8_t uid, uint16_t addr, 
    vector<uint16_t> data)
{
    uint16_t len = data.size();
    vector<uint8_t> payload;
    payload.push_back(static_cast<uint8_t>(0xFF & (addr >> 8)));
    payload.push_back(static_cast<uint8_t>(0xFF & addr));
    payload.push_back(static_cast<uint8_t>(0xFF & (len >> 8)));
    payload.push_back(static_cast<uint8_t>(0xFF & len));
    payload.push_back(static_cast<uint8_t>(0xFF & 2*len));
    for (unsigned i = 0; i < len; i++) {
        payload.push_back(static_cast<uint8_t>(0xFF & (data.at(i) >> 8)));
        payload.push_back(static_cast<uint8_t>(0xFF & data.at(i)));
    }
    frame sframe(uid, FC16, payload);

    debug_print("Writing %u registers with starting address 0x%04X (uid=%u)\n", 
        len, addr, uid);

    this->write_byte(sframe.get());
    vector<uint8_t> resp = this->read_byte();

    struct frame rframe(resp);
    if (rframe.function_code & ERRC)
        this->check_error_code(rframe.get_error());
    
    // TODO: check returned values
    return;
}

/*
 *      P R I V A T E   M E T H O D S
 */

vector<uint16_t> modbus_rtu_iface::read_16bit_regs(uint8_t uid, uint8_t func,
    uint16_t addr, uint16_t len)
{
    vector<uint8_t> payload{};
    payload.push_back(static_cast<uint8_t>(0xFF & (addr >> 8)));
    payload.push_back(static_cast<uint8_t>(0xFF & addr));
    payload.push_back(static_cast<uint8_t>(0xFF & (len >> 8)));
    payload.push_back(static_cast<uint8_t>(0xFF & len));
    frame frame(uid, func, payload);

    debug_print("Reading %u registers with starting address 0x%04X (uid=%u)\n", 
        len, addr, uid);

    this->write_byte(frame.get());
    vector<uint8_t> resp = this->read_byte();

    struct frame rframe(resp);
    if (rframe.function_code & ERRC)
        this->check_error_code(rframe.get_error());
    rframe.data.erase(rframe.data.begin()); // skip first byte (data byte count)

    // TODO: check returned values (function code, address, etc. ...)

    vector<uint16_t> ret;
    if (rframe.data.size() % 2) // 0-padding if number of bytes is not even
        rframe.data.push_back(0x00);
    for (unsigned i = 0; i < len; i++)
        ret.push_back((rframe.data.at(2*i) << 8) | rframe.data.at(2*i+1));
    return ret;
}

modbus_rtu_iface::frame::frame(vector<uint8_t> msg) 
    : address(0x00), function_code(0x00), data()
{
    address = msg.at(0);
    function_code = msg.at(1);
    data.insert(data.begin(), msg.begin() + 2, msg.end() - 2);
    size_t last = msg.size() - 1;
    crc = static_cast<uint16_t>( (msg.at(last - 1) << 8) | msg.at(last) );
    return;
}

modbus_rtu_iface::frame::frame(uint8_t addr, uint8_t func, vector<uint8_t> payload) 
    : address(addr), function_code(func), data(payload)
{
    return;
}

vector<uint8_t> modbus_rtu_iface::frame::get() 
{ 
    vector<uint8_t> frame;
    frame.push_back(address);
    frame.push_back(function_code);
    // Append data
    frame.insert(frame.end(), data.begin(), data.end());
    uint16_t crc = calc_crc16(frame);
    frame.push_back( static_cast<uint8_t>( 0xFF & crc));
    frame.push_back( static_cast<uint8_t>( 0xFF & (crc >> 8) ));
    return frame;
}

uint16_t modbus_rtu_iface::calc_crc16(std::vector<uint8_t> data)
{
    uint16_t crc = 0xFFFF;  // Start value
    for (auto pos : data) {
        crc ^= static_cast<uint16_t>(pos);  // XOR byte into least sig. byte of crc
        for (int i = 8; i != 0; i--) {      // Loop over each bit
            if ( (crc & 0x0001) != 0) {     // If the LSB is set
                crc >>= 1;                  // Shift right and XOR 0xA001
                crc ^= 0xA001;
            } else {                        // Else LSB is not set
                crc >>= 1;                  // Just shift right
            }
        }
    }
    // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
    return crc;
}

}