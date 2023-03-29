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
        vector<uint16_t> ret;
        // TODO
        return ret;
    }

    vector<uint16_t> modbus_tcp::read_input_regs(uint8_t uid, uint16_t addr, 
        uint16_t len)
    {
        vector<uint8_t> payload{};
        payload.push_back(static_cast<uint8_t>(0xFF & (len >> 8)));
        payload.push_back(static_cast<uint8_t>(0xFF & len));
        tcp_frame frame(m_tid, uid, FC04, payload);

        m_comm->write_byte(frame.get_frame());
        vector<uint8_t> resp = m_comm->read_byte();
        tcp_frame resp_frame(resp);
        
        vector<uint16_t> ret;
        return ret;
    }

    void modbus_tcp::write_single_coil(uint8_t uid, uint16_t addr, bool ena)
    {
        // TODO
        return;
    }

    void modbus_tcp::write_single_holding_reg(uint8_t uid, uint16_t addr, 
        uint16_t data)
    {
        // TODO
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
        // TODO
        return;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    modbus_tcp::tcp_frame::tcp_frame(vector<uint8_t> msg)
        : transaction_id(0x0000),
          protocol_id(0x0000),
          length(0x0000),
          function_code(0x0000),
          unit_id(0x00),
          byte_count(0x00),
          data()
    {
        transaction_id  = static_cast<uint16_t>( (msg.at(0) << 8) | msg.at(1) );
        protocol_id = static_cast<uint16_t>( (msg.at(2) << 8) | msg.at(3) );
        length = static_cast<uint16_t>( (msg.at(4) << 8) | msg.at(5) );
        unit_id = msg.at(6);
        function_code = msg.at(7);
        byte_count = msg.at(8);
        data.insert(data.begin(), msg.begin() + 9, msg.end());
        return;
    }

    modbus_tcp::tcp_frame::tcp_frame(uint16_t trans_id, uint8_t uid, 
        uint8_t func, std::vector<uint8_t> payload)
        : transaction_id(trans_id), 
          protocol_id(0x0000),
          length(0x0000),
          function_code(func),
          unit_id(uid),
          byte_count(0x00),
          data(payload)
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