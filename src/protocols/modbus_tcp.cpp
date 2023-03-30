#include <labdev/protocols/modbus_tcp.hh>
#include <labdev/exceptions.hh>

using namespace std;

namespace labdev {

    modbus_tcp::modbus_tcp(tcpip_interface* tcpip)
        : m_comm(nullptr), m_tid(0x0000)
    {
        if ( !tcpip->good() ) {
            fprintf(stderr, "Interface '%s' is not ready for communication\n",
                tcpip->get_info().c_str());
            abort();
        }
        m_comm = tcpip;
        return;
    }

    modbus_tcp::~modbus_tcp()
    {
        return;
    }

    vector<bool> modbus_tcp::read_coils(uint8_t uid, uint16_t addr, uint16_t len)
    {
        vector<bool> ret;
        // TODO
        return ret;
    }

    vector<bool> modbus_tcp::read_discrete_inputs(uint8_t uid, uint16_t addr, 
        uint16_t len)
    {
        vector<bool> ret;
        // TODO
        return ret;
    }

    vector<uint16_t> modbus_tcp::read_multiple_holding_regs(uint8_t uid, 
        uint16_t addr, uint16_t len)
    {
        return this->read_16bit_regs(uid, FC03, addr, len);
    }

    vector<uint16_t> modbus_tcp::read_input_regs(uint8_t uid, uint16_t addr, 
        uint16_t len)
    {
        return this->read_16bit_regs(uid, FC04, addr, len);
    }

    void modbus_tcp::write_single_coil(uint8_t uid, uint16_t addr, bool ena)
    {
        // TODO
        return;
    }

    void modbus_tcp::write_single_holding_reg(uint8_t uid, uint16_t addr, 
        uint16_t data)
    {
        vector<uint8_t> payload{};
        payload.push_back(static_cast<uint8_t>(0xFF & (addr >> 8)));
        payload.push_back(static_cast<uint8_t>(0xFF & addr));
        payload.push_back(static_cast<uint8_t>(0xFF & (data >> 8)));
        payload.push_back(static_cast<uint8_t>(0xFF & data));
        tcp_frame sframe(m_tid, uid, FC06, payload);
        m_comm->write_byte(sframe.get_frame());
        vector<uint8_t> resp = m_comm->read_byte();
        // TODO: check echo + error codes
        return;
    }

    void modbus_tcp::write_multiple_coils(uint8_t uid, uint16_t addr, 
        vector<bool> ena)
    {
        // TODO
        return;
    }

    void modbus_tcp::write_multiple_holding_regs(uint8_t uid, uint16_t addr, 
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
        tcp_frame sframe(m_tid, uid, FC16, payload);
        m_comm->write_byte(sframe.get_frame());
        vector<uint8_t> resp = m_comm->read_byte();
        // TODO: check echo + error codes
        return;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    void write_16bit_regs(uint8_t uid, uint8_t func, uint16_t addr,
            std::vector<uint8_t> data)
    {
        uint16_t len = data.size();
        data.insert(data.begin(), static_cast<uint8_t>(0xFF & len));
        data.insert(data.begin(), static_cast<uint8_t>(0xFF & (len >> 8)));
        return;
    }

    vector<uint16_t> modbus_tcp::read_16bit_regs(uint8_t uid, uint8_t func,
        uint16_t addr, uint16_t len)
    {
        vector<uint8_t> payload{};
        payload.push_back(static_cast<uint8_t>(0xFF & (addr >> 8)));
        payload.push_back(static_cast<uint8_t>(0xFF & addr));
        payload.push_back(static_cast<uint8_t>(0xFF & (len >> 8)));
        payload.push_back(static_cast<uint8_t>(0xFF & len));
        tcp_frame frame(m_tid, uid, func, payload);

        m_comm->write_byte(frame.get_frame());
        vector<uint8_t> resp = m_comm->read_byte();
        tcp_frame rframe(resp);

        if (rframe.function_code & ERRC) {
            // The byte_count field carries the exception code
            switch (rframe.byte_count) {
            case ERR1:
                throw bad_protocol("Function code not supported");
                break;
            case ERR2:
                throw bad_protocol("Starting address or last address not"
                    "supported");
                break;
            case ERR3:
                throw bad_protocol("Quantity of registers not supported" 
                    " (range 1 - 125)");
                break;
            case ERR4:
                throw bad_protocol("No read access to registers");
                break;
            default:
                throw bad_protocol("Unknown exception code " + 
                    to_string(rframe.byte_count));
            }
        }

        if (rframe.byte_count != rframe.data.size())
            throw bad_protocol("Wrong number of bytes received");

        vector<uint16_t> ret;
        // Increase transaction ID after each transaction
        if (m_tid < 0xFFFF) m_tid++;
        else m_tid = 0;

        if (rframe.data.size() % 2) // 0-padding if number of bytes is not even
            rframe.data.push_back(0x00);
        for (unsigned i = 0; i < len; i++)
            ret.push_back((rframe.data.at(2*i) << 8) | rframe.data.at(2*i+1));

        return ret;
    }

    modbus_tcp::tcp_frame::tcp_frame(vector<uint8_t> msg)
        : transaction_id(0x0000), protocol_id(0x0000), length(0x0000),
          function_code(0x0000), unit_id(0x00), byte_count(0x00), data()
    {
        transaction_id = static_cast<uint16_t>( (msg.at(0) << 8) | msg.at(1) );
        protocol_id    = static_cast<uint16_t>( (msg.at(2) << 8) | msg.at(3) );
        length         = static_cast<uint16_t>( (msg.at(4) << 8) | msg.at(5) );
        unit_id        = msg.at(6);
        function_code  = msg.at(7);
        byte_count     = msg.at(8);
        data.insert(data.begin(), msg.begin() + 9, msg.end());
        return;
    }

    modbus_tcp::tcp_frame::tcp_frame(uint16_t trans_id, uint8_t uid, 
        uint8_t func, std::vector<uint8_t> payload)
        : transaction_id(trans_id), protocol_id(0x0000), length(0x0000),
          function_code(func), unit_id(uid), byte_count(0x00), data(payload)
    {
        return;
    }

    vector<uint8_t> modbus_tcp::tcp_frame::get_frame() 
    { 
        vector<uint8_t> frame;
        length = 2 + data.size();
        frame.push_back(static_cast<uint8_t>(0xFF & (transaction_id >> 8)));
        frame.push_back(static_cast<uint8_t>(0xFF & transaction_id));
        frame.push_back(static_cast<uint8_t>(0xFF & (protocol_id >> 8)));
        frame.push_back(static_cast<uint8_t>(0xFF & protocol_id));
        frame.push_back(static_cast<uint8_t>(0xFF & (length >> 8)));
        frame.push_back(static_cast<uint8_t>(0xFF & length));
        frame.push_back(unit_id);
        frame.push_back(function_code);
        // Append data
        frame.insert(frame.end(), data.begin(), data.end());
        return frame;
    }

}