#include <labdev/devices/rohde-schwarz/hmp4000.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <unistd.h>

using namespace std;

namespace labdev {

hmp4000::hmp4000(std::unique_ptr<tcpip_iface> tcpip) : hmp4000() 
{
    this->connect(std::move(tcpip));
    this->init();
    return;
}

hmp4000::hmp4000(std::unique_ptr<serial_iface> ser) : hmp4000() 
{
    this->connect(std::move(ser));
    this->init();
    return;
}

hmp4000::~hmp4000()
{
    if (this->connected())
        this->disconnect();
    return;
}

void hmp4000::connect(std::unique_ptr<ld_iface> comm)
{
    if ( this->connected() ) {
        string err = this->get_info() + " : device is already connected";
        throw device_error(err);
        return;
    }

    Interface_type type = comm->type();
    if (type == TCPIP) {
        // Convert to tcpip interface
        unique_ptr<tcpip_iface> tcpip(
            dynamic_cast<tcpip_iface*>(comm.release()));
        
        // Default port 5025
        if (tcpip->get_port() != hmp4000::PORT) {
            fprintf(stderr, "HMP4000 only supports port %i\n", hmp4000::PORT);
            abort();
        }

        // Everything seems to be in order
        m_comm = std::move(tcpip);
    } else if (type == SERIAL) {
        // Convert to usbtmc interface
        unique_ptr<serial_iface> ser(
            dynamic_cast<serial_iface*>(comm.release()));

        // Everything seems to be in order
        m_comm = std::move(ser);
    } else if (type == VISA) {
        // Convert to usbtmc interface
        unique_ptr<visa_iface> visa(
            dynamic_cast<visa_iface*>(comm.release()));

        m_comm = std::move(visa);
    } else {
        string err = this->get_info() + " : interface is not supported";
        throw device_error(err); 
    } 

    this->init();
    return;
}

void hmp4000::disconnect()
{
    m_comm.reset();
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
    std::string resp = m_comm->query("OUTP?\n");
    if (resp.find("1") != std::string::npos)
        return true;
    return false;
}

void hmp4000::enable_outputs(bool ena) 
{
    std::stringstream msg("");
    msg << "OUTP:GEN " << (ena? "1" : "0") << "\n";
    m_comm->write(msg.str());
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
    m_comm->write(msg.str());
    return;
}

double hmp4000::get_voltage(int channel) 
{
    // Switch channel
    this->select_channel(channel);
    std::string resp = m_comm->query("VOLT?\n");
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
    m_comm->write(msg.str());
    return;
}

double hmp4000::get_current(int channel) 
{
    // Switch channel
    this->select_channel(channel);
    std::string resp = m_comm->query("CURR?\n");
    return std::stod(resp);
}

double hmp4000::measure_voltage(int channel) 
{
    this->select_channel(channel);
    std::string resp = m_comm->query("MEAS:VOLT?\n");
    return std::stod(resp);
}

double hmp4000::measure_current(int channel) 
{
    this->select_channel(channel);
    std::string resp = m_comm->query("MEAS:CURR?\n");
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
    m_comm->write(msg.str());
    return;
}

void hmp4000::ovp_reset(int channel) 
{
    this->select_channel(channel);
    m_comm->write("VOLT:PROT:CLE\n");
    return;
}

bool hmp4000::ovp_tripped(int channel) 
{
    this->select_channel(channel);
    std::string resp = m_comm->query("VOLT:PROT:TRIP?\n");
    if (resp.find("ON") != std::string::npos)
        return true;
    return false;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void hmp4000::init() 
{
    this->CLS();
    usleep(100e3);
    m_dev_name = this->get_IDN();

    this->select_channel(1);
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
        m_comm->write(msg.str());
    }
    return;
}

void hmp4000::activate(bool ena) 
{
    std::stringstream msg("");
    msg << "OUTP:SEL " << (ena? "1" : "0") << "\n";
    m_comm->write(msg.str());
    return;
}

}