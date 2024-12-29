#include <labdev/devices/riden/rd6006.hh>
#include <labdev/exceptions.hh>

using namespace std;

namespace labdev {

rd6006::rd6006(std::unique_ptr<modbus_rtu_iface> modbus) : rd6006()
{
    this->connect(std::move(modbus));
    return;
}

rd6006::~rd6006()
{
    if (this->connected())
        this->disconnect();
    return;
}

void rd6006::connect(std::unique_ptr<ld_iface> comm)
{
    if ( this->connected() ) {
        string err = this->get_info() + " : device is already connected";
        throw device_error(err);
        return;
    }

    Interface_type type = comm->type();
    if (type == MODBUS_RTU) {
        // Convert to modbus rtu interface
        unique_ptr<modbus_rtu_iface> modbus(
            dynamic_cast<modbus_rtu_iface*>(comm.release()));
        
        if (modbus->get_nbits() != 8) {
            fprintf(stderr, "Invalid number of bits %u; RD6006 only supports 8N1\n", 
                modbus->get_nbits());
            abort();
        }
        if (modbus->get_parity() != false) {
            fprintf(stderr, "Invalid parity; RD6006 only supports 8N1\n");
            abort();
        }
        if (modbus->get_stop_bits() != 1) {
            fprintf(stderr, "Invalid number of stop bits %u; RD6006 only supports 8N1\n", 
                modbus->get_stop_bits());
            abort();
        }

        // Everything seems to be in order
        m_modbus = std::move(modbus);
    } else {
        string err = this->get_info() + " : interface is not supported";
        throw device_error(err); 
    } 

    this->init();
    return;
}

void rd6006::disconnect()
{
    m_modbus.reset();
    return;
}

void rd6006::enable_output(bool ena)
{
    m_modbus->write_single_holding_reg(UID, OUTP, static_cast<uint16_t>(ena));
    return;
}

bool rd6006::output_enabled()
{
    auto resp = m_modbus->read_multiple_holding_regs(UID, OUTP, 1);
    return static_cast<bool>(resp.at(0));
}

void rd6006::set_voltage(double volts)
{
    uint16_t volt = static_cast<uint16_t>(100. * volts);
    m_modbus->write_single_holding_reg(UID, VSET0, volt);
    return;
}

double rd6006::get_voltage()
{
    auto resp = m_modbus->read_multiple_holding_regs(UID, VSET0, 0x0001);
    double volt = 1e-2 * static_cast<double>(resp.at(0));
    return volt;
}

void rd6006::set_current(double amps)
{
    uint16_t amp = static_cast<uint16_t>(1000. * amps);
    m_modbus->write_single_holding_reg(UID, VSET0, amp);
    return;
}

double rd6006::get_current()
{
    auto resp = m_modbus->read_multiple_holding_regs(UID, ISET0, 0x0001);
    double curr = 1e-3 * static_cast<double>(resp.at(0));
    return curr;
}

double rd6006::measure_voltage()
{
    auto resp = m_modbus->read_multiple_holding_regs(UID, VOUT, 0x0001);
    double volt = 1e-2 * static_cast<double>(resp.at(0));
    return volt;
}

double rd6006::measure_current()
{
    auto resp = m_modbus->read_multiple_holding_regs(UID, IOUT, 0x0001);
    double amps = 1e-3 * static_cast<double>(resp.at(0));
    return amps;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void rd6006::init()
{
    // Recall memory settings 0
    m_modbus->write_single_holding_reg(UID, MEMID, 0x0000);
    return;
}

}