#include <labdev/devices/modbus_device.hh>
#include <labdev/ld_debug.hh>
#include <labdev/exceptions.hh>

using namespace std;

namespace labdev
{

std::vector<bool> modbus_device::read_coils(uint8_t unit_id, uint16_t addr, 
    uint16_t len)
{
    vector<bool> ret;
    // TODO -> need device that actually uses this..
    return ret;
}

std::vector<bool> modbus_device::read_discrete_inputs(uint8_t unit_id, 
    uint16_t addr, uint16_t len)
{
    vector<bool> ret;
    // TODO -> need device that actually uses this..
    return ret;
}

std::vector<uint16_t> modbus_device::read_multiple_holding_regs(uint8_t unit_id, 
    uint16_t addr, uint16_t len)
{
    return this->read_16bit_regs(unit_id, FC03, addr, len);
}

std::vector<uint16_t> modbus_device::read_input_regs(uint8_t unit_id, 
    uint16_t addr, uint16_t len)
{
    return this->read_16bit_regs(unit_id, FC04, addr, len);
}

void modbus_device::write_single_coil(uint8_t unit_id, uint16_t addr, bool ena)   
{
    // TODO -> need device that actually uses this..
    return;
}

void modbus_device::write_single_holding_reg(uint8_t unit_id, uint16_t addr, 
    uint16_t reg)
{
    vector<uint8_t> data {};
    data.push_back(static_cast<uint8_t>(0xFF & (addr >> 8)));
    data.push_back(static_cast<uint8_t>(0xFF & addr));
    data.push_back(static_cast<uint8_t>(0xFF & (reg >> 8)));
    data.push_back(static_cast<uint8_t>(0xFF & reg));
    auto packet = this->create_packet(unit_id, FC06, data);

    debug_print("Writing 0x%04X to address 0x%04X (tid=%u, unit_id=%u)\n",
        reg, addr, m_tid, unit_id);

    vector<uint8_t> resp = m_comm->query_byte(packet);

    // Extract header and data from response
    uint8_t received_fcode {0x00}, received_err {0x00};
    if (m_comm->type() == TCPIP) {
        received_fcode = resp.at(7);
        received_err = resp.at(8);
    } else if (m_comm->type() == SERIAL) {
        received_fcode = resp.at(1);
        received_err = resp.at(2);
    }

    // Check for errors (TODO: also check received address, register and unit_id)
    if (received_fcode & ERRC)
        this->check_and_throw(received_err);
    
    // Increase transaction ID after each transaction
    if (m_comm->type() == TCPIP)
        m_tid++;

    return;
}

void modbus_device::write_multiple_coils(uint8_t unit_id, uint16_t addr, 
    std::vector<bool> ena)
{
    // TODO -> need device that actually uses this..
    return;
}

void modbus_device::write_multiple_holding_regs(uint8_t unit_id, uint16_t addr, 
    std::vector<uint16_t> regs)
{
    uint16_t len = regs.size();
    vector<uint8_t> data;
    data.push_back(static_cast<uint8_t>(0xFF & (addr >> 8)));   // Starting register
    data.push_back(static_cast<uint8_t>(0xFF & addr));          // address
    data.push_back(static_cast<uint8_t>(0xFF & (len >> 8)));    // Number of registers
    data.push_back(static_cast<uint8_t>(0xFF & len));
    data.push_back(static_cast<uint8_t>(0xFF & 2*len));         // Number of bytes
    for (unsigned i = 0; i < len; i++) {
        data.push_back(static_cast<uint8_t>(0xFF & (data.at(i) >> 8)));
        data.push_back(static_cast<uint8_t>(0xFF & data.at(i)));
    }
    auto packet = this->create_packet(unit_id, FC16, data);

    debug_print("Writing %u registers with starting address 0x%04X "
        "(tid=%u, unit_id=%u)\n", len, addr, m_tid, unit_id);

    vector<uint8_t> resp = m_comm->query_byte(packet);

    // Extract header and data from response
    uint8_t received_fcode {0x00}, received_err {0x00};
    if (m_comm->type() == TCPIP) {
        received_fcode = resp.at(7);
        received_err = resp.at(8);
    } else if (m_comm->type() == SERIAL) {
        received_fcode = resp.at(1);
        received_err = resp.at(2);
    }
    
    // Check for errors (TODO: check received address, register and unit_id)
    if (received_fcode & ERRC)
        this->check_and_throw(received_err);
    
    // Increase transaction ID after each transaction
    if (m_comm->type() == TCPIP)
        m_tid++;
    return;
}

/*
 *  P R I V A T E   M E T H O D S
 */

uint16_t modbus_device::calc_crc16(vector<uint8_t> data)
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
    // Note: this number has low and high bytes swapped, so use it accordingly 
    // (or swap bytes)
    return crc;
}

vector<uint8_t> modbus_device::create_packet(uint8_t unit_id, 
    uint8_t function_code, vector<uint8_t> &data)
{
    vector<uint8_t> packet {};

    // MODBUS TCP: Add MBAP header
    if (m_comm->type() == TCPIP) {
        uint16_t length = static_cast<uint16_t>(2 + data.size());
        packet.push_back(static_cast<uint8_t>(0xFF & (m_tid >> 8)));
        packet.push_back(static_cast<uint8_t>(0xFF & m_tid));
        packet.push_back(static_cast<uint8_t>(0x00));   // Protocol ID, always
        packet.push_back(static_cast<uint8_t>(0x00));   // 0x0000
        packet.push_back(static_cast<uint8_t>(0xFF & (length >> 8)));
        packet.push_back(static_cast<uint8_t>(0xFF & length));
    }

    // Add Protocol Data Unit (PDU)
    packet.push_back(unit_id);
    packet.push_back(function_code);
    packet.insert(packet.end(), data.begin(), data.end());

    // MODBUS RTU: Append CRC checksum
    if (m_comm->type() == SERIAL) {
        uint16_t crc = this->calc_crc16(packet);
        packet.push_back(static_cast<uint8_t>(0xFF & crc));
        packet.push_back(static_cast<uint8_t>(0xFF & (crc >> 8)));
    }

    return packet;
}

vector<uint16_t> modbus_device::read_16bit_regs(uint8_t unit_id, uint8_t function_code, 
    uint16_t start_addr, uint16_t len)
{
    // Create packet with payload
    vector<uint8_t> data {};
    data.push_back(static_cast<uint8_t>(0xFF & (start_addr >> 8)));
    data.push_back(static_cast<uint8_t>(0xFF & start_addr));
    data.push_back(static_cast<uint8_t>(0xFF & (len >> 8)));
    data.push_back(static_cast<uint8_t>(0xFF & len));
    vector<uint8_t> packet = this->create_packet(unit_id, function_code, data);

    debug_print("Reading %u registers with starting address 0x%04X "
        "(tid=%u, unit_id=%u)\n", len, start_addr, m_tid, unit_id);

    auto resp = m_comm->query_byte(packet);

    // Extract header and data from response
    vector<uint8_t> received_data {};
    uint8_t received_fcode {0x00}, received_bytes {0x00};
    if (m_comm->type() == TCPIP) {
        received_fcode = resp.at(7);
        received_bytes = resp.at(8);
        received_data.insert(received_data.begin(), resp.begin() + 9, resp.end());
    } else if (m_comm->type() == SERIAL) {
        received_fcode = resp.at(1);
        received_bytes = resp.at(2);
        received_data.insert(received_data.begin(), resp.begin() + 3, resp.end() - 2);
    }

    // Check for errors (TODO: check received bytes and unit_id)
    if (received_fcode & ERRC)
        this->check_and_throw(received_bytes);

    // Create 16-bit return vector
    vector<uint16_t> ret;
    if (received_data.size() % 2) // 0-padding if number of bytes is not even
        received_data.push_back(0x00);
    for (unsigned i = 0; i < len; i++)
        ret.push_back((received_data.at(2*i) << 8) | received_data.at(2*i+1));

    // MODBUS TCP: increase transaction id counter
    if (m_comm->type() == TCPIP)
        m_tid++;

    return ret;
}

void modbus_device::check_and_throw(uint8_t error)
{
    switch (error) {
    case ERR1:
        throw bad_protocol("Function code not supported");
        break;
    case ERR2:
        throw bad_protocol("Starting address or last address not supported");
        break;
    case ERR3:
        throw bad_protocol("Quantity of registers not supported (range 1 - 125)");
        break;
    case ERR4:
        throw bad_protocol("No read access to registers");
        break;
    default:
        throw bad_protocol("Unknown error code " + to_string(error));
    }
    return; 
}

}