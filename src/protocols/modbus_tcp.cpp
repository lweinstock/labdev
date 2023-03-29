#include <labdev/protocols/modbus_tcp.hh>

using namespace std;

namespace labdev {

    modbus_tcp::modbus_tcp(tcpip_interface &ip)
    {
        return;
    }

    modbus_tcp::~modbus_tcp()
    {
        return;
    }

    vector<bool> modbus_tcp::read_coils(uint16_t addr, uint16_t len)
    {
        vector<bool> ret;
        return ret;
    }

    vector<uint16_t> modbus_tcp::read_discrete_inputs(uint16_t addr, 
        uint16_t len)
    {
        vector<uint16_t> ret;
        return ret;
    }

    vector<uint16_t> modbus_tcp::read_multiple_holding_regs(uint16_t addr, 
        uint16_t len)
    {
        vector<uint16_t> ret;
        return ret;
    }

    vector<uint16_t> modbus_tcp::read_input_regs(uint16_t addr, uint16_t len)
    {
        vector<uint16_t> ret;
        return ret;
    }

    void modbus_tcp::write_single_coil(uint16_t addr, bool ena)
    {
        return;
    }

    void modbus_tcp::write_single_holding_reg(uint16_t addr, uint16_t data)
    {
        return;
    }

    void modbus_tcp::write_multiple_coils(uint16_t addr, vector<bool> ena)
    {
        return;
    }

    void modbus_tcp::write_multiple_holding_regs(uint16_t addr, 
        vector<uint16_t> data)
    {
        return;
    }

}