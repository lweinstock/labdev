#include <labdev/devices/riden/rd6006.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

using namespace std;

namespace labdev {

rd6006::rd6006(std::unique_ptr<serial_iface> modbus) : rd6006()
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
    if (type == SERIAL) {
        // Convert to modbus rtu interface
        unique_ptr<serial_iface> serial(
            dynamic_cast<serial_iface*>(comm.release()));
        
        if (serial->get_nbits() != 8) {
            fprintf(stderr, "Invalid number of bits %u; RD6006 only supports 8N1\n", 
                serial->get_nbits());
            abort();
        }
        if (serial->get_parity() != false) {
            fprintf(stderr, "Invalid parity; RD6006 only supports 8N1\n");
            abort();
        }
        if (serial->get_stop_bits() != 1) {
            fprintf(stderr, "Invalid number of stop bits %u; RD6006 only supports 8N1\n", 
                serial->get_stop_bits());
            abort();
        }

        // Everything seems to be in order
        m_comm = std::move(serial);
    } else {
        string err = this->get_info() + " : interface is not supported";
        throw device_error(err); 
    } 

    this->init();
    return;
}

void rd6006::disconnect()
{
    m_comm.reset();
    return;
}

void rd6006::enable_output(bool ena)
{
    this->write_single_holding_reg(UID, OUTP, static_cast<uint16_t>(ena));
    return;
}

bool rd6006::output_enabled()
{
    auto resp = this->read_multiple_holding_regs(UID, OUTP, 1);
    if (resp.size() == 0)
        throw bad_protocol("Received empty response");
    return static_cast<bool>(resp.at(0));
}

void rd6006::set_voltage(double volts)
{
    uint16_t volt = static_cast<uint16_t>(100. * volts);
    debug_print("Setting voltage to %.3f (0x%04X)\n", volts, volt);
    this->write_single_holding_reg(UID, VSET0, volt);
    return;
}

double rd6006::get_voltage()
{
    auto resp = this->read_multiple_holding_regs(UID, VSET0, 0x0001);
    if (resp.size() == 0)
        throw bad_protocol("Received empty response");
    double volt = 1e-2 * static_cast<double>(resp.at(0));
    return volt;
}

void rd6006::set_current_limit(double amps)
{
    uint16_t amp = static_cast<uint16_t>(1000. * amps);
    debug_print("Setting current to %.3f (0x%04X)\n", amps, amp);
    this->write_single_holding_reg(UID, ISET0, amp);
    return;
}

double rd6006::get_current_limit()
{
    auto resp = this->read_multiple_holding_regs(UID, ISET0, 0x0001);
    if (resp.size() == 0)
        throw bad_protocol("Received empty response");
    double curr = 1e-3 * static_cast<double>(resp.at(0));
    return curr;
}

double rd6006::measure_voltage()
{
    auto resp = this->read_multiple_holding_regs(UID, VOUT, 0x0001);
    if (resp.size() == 0)
        throw bad_protocol("Received empty response");
    double volt = 1e-2 * static_cast<double>(resp.at(0));
    return volt;
}

double rd6006::measure_current()
{
    auto resp = this->read_multiple_holding_regs(UID, IOUT, 0x0001);
    if (resp.size() == 0)
        throw bad_protocol("Received empty response");
    double amps = 1e-3 * static_cast<double>(resp.at(0));
    return amps;
}

void rd6006::set_ovp(double volts)
{
    uint16_t volt = static_cast<uint16_t>(100. * volts);
    this->write_single_holding_reg(UID, OVP0, volt);
    return;
}

bool rd6006::ovp_tripped()
{
    auto resp = this->read_multiple_holding_regs(UID, STAT, 1);
    if (resp.size() == 0)
        throw bad_protocol("Received empty response");
    if (resp.at(0) == 1)    // OVP
        return true;
    return false;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void rd6006::init()
{
    // Recall memory settings 0
    this->write_single_holding_reg(UID, MEMID, 0x0000);
    return;
}

}