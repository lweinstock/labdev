#include <labdev/devices/rohde-schwarz/hmp4000.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <unistd.h>

namespace labdev {

hmp4000::hmp4000() 
    : ld_device("Rohde&Schwarz,HMP4000"), m_cur_ch(0)
{
    return;
}


hmp4000::hmp4000(tcpip_interface* tcpip) : hmp4000() 
{
    this->connect(tcpip);
    this->init();
    return;
}

hmp4000::hmp4000(serial_interface* ser) : hmp4000() 
{
    this->connect(ser);
    this->init();
    return;
}

hmp4000::~hmp4000()
{
    if (this->connected())
        this->disconnect();
    return;
}

void hmp4000::connect(tcpip_interface* tcpip) 
{
    // Check and assign interface
    this->set_comm(tcpip);

    if (tcpip->get_port() != hmp4000::PORT)
    {
        fprintf(stderr, "HMP4000 only supports port %i\n", hmp4000::PORT);
        abort();
    }

    this->init();
    return;
}

void hmp4000::connect(serial_interface* ser) 
{
    // Check and assign interface
    this->set_comm(ser);

    // TODO: check serial port settings!

    this->init();
    return;
}

void hmp4000::disconnect()
{
    if (m_scpi) {
        delete m_scpi;
        m_scpi = nullptr;
    }
    this->reset_comm();
    return;
}

void hmp4000::enable_channel(int channel, bool ena) 
{
    this->select_channel(channel);
    this->activate(ena);
    return;
}

bool hmp4000::channel_enabled(int channel) 
{
    this->select_channel(channel);
    std::string resp = get_comm()->query("OUTP?\n");
    if (resp.find("1") != std::string::npos)
        return true;
    return false;
}

void hmp4000::enable_outputs(bool ena) 
{
    std::stringstream msg("");
    msg << "OUTP:GEN " << (ena? "1" : "0") << "\n";
    get_comm()->write(msg.str());
    return;
}

void hmp4000::set_voltage(int channel, double volts) 
{
    this->select_channel(channel);
    // Check voltage range
    if ( volts < 0 || volts > 32.05 ) {
        fprintf(stderr, "Voltage %f V out of range\n", volts);
        abort();
    }
    std::stringstream msg("");
    msg << "VOLT " << volts << "\n";
    get_comm()->write(msg.str());
    return;
}

double hmp4000::get_voltage(int channel) 
{
    // Switch channel
    this->select_channel(channel);
    std::string resp = get_comm()->query("VOLT?\n");
    return std::stod(resp); // TODO: check conversions with ifnan()
}

void hmp4000::set_current(int channel, double amps) 
{
    this->select_channel(channel);

    // Check current range
    if ( amps < 0 || amps > 10.01 ) {
        fprintf(stderr, "Current %f A out of range\n", amps);
        abort();
    }
    std::stringstream msg("");
    msg << "CURR " << amps << "\n";
    get_comm()->write(msg.str());
    return;
}

double hmp4000::get_current(int channel) 
{
    // Switch channel
    this->select_channel(channel);
    std::string resp = get_comm()->query("CURR?\n");
    return std::stod(resp);
}

double hmp4000::measure_voltage(int channel) 
{
    this->select_channel(channel);
    std::string resp = get_comm()->query("MEAS:VOLT?\n");
    return std::stod(resp);
}

double hmp4000::measure_current(int channel) 
{
    this->select_channel(channel);
    std::string resp = get_comm()->query("MEAS:CURR?\n");
    return std::stod(resp);
}

void hmp4000::set_ovp(int channel, double volts) 
{
    this->select_channel(channel);

    // Check voltage range
    if ( volts < 0 || volts > 32.25 ) {
        fprintf(stderr, "Voltage %f V out of range\n", volts);
        abort();
    }
    std::stringstream msg("");
    msg << "VOLT:PROT " << volts << "\n";
    get_comm()->write(msg.str());
    return;
}

void hmp4000::ovp_reset(int channel) 
{
    this->select_channel(channel);
    get_comm()->write("VOLT:PROT:CLE\n");
    return;
}

bool hmp4000::ovp_tripped(int channel) 
{
    this->select_channel(channel);
    std::string resp = get_comm()->query("VOLT:PROT:TRIP?\n");
    if (resp.find("ON") != std::string::npos)
        return true;
    return false;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void hmp4000::init() 
{
    // Setup SCPI
    if (m_scpi)
        delete m_scpi;
    m_scpi = new scpi( get_comm() );
    m_scpi->clear_status();
    
    this->select_channel(1);
    m_dev_name = m_scpi->get_identifier();
    return;
}

void hmp4000::select_channel(int channel) 
{
    // Check channel
    if (channel < 1 || channel > 4) {
        fprintf(stderr, "Invalid channel %i\n", channel);
        abort();
    }

    // Check current channel and switch if necessary
    if (m_cur_ch != channel) {
        m_cur_ch = channel;
        debug_print("Switching to channel %i...\n", channel);
        std::stringstream msg("");
        msg << "INST OUTP" << channel << "\n";
        get_comm()->write(msg.str());
    }
    return;
}

void hmp4000::activate(bool ena) 
{
    std::stringstream msg("");
    msg << "OUTP:SEL " << (ena? "1" : "0") << "\n";
    get_comm()->write(msg.str());
    return;
}

}