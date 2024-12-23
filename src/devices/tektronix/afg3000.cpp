#include <labdev/devices/tektronix/afg3000.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <iomanip>

using namespace std;

namespace labdev {

afg3000::afg3000() : fgen(2, "Tektronix,AFG3000")
{
    return;
}

afg3000::afg3000(std::unique_ptr<tcpip_iface> tcpip) : afg3000()
{
    this->connect(std::move(tcpip));
    return;
}

afg3000::afg3000(std::unique_ptr<usbtmc_iface> usbtmc) : afg3000()
{
    this->connect(std::move(usbtmc));
    return;
}

afg3000::afg3000(std::unique_ptr<visa_iface> visa) : afg3000()
{
    this->connect(std::move(visa));
    return;
}

afg3000::~afg3000() 
{
    if (this->connected())
        this->disconnect();
    return;
}

void afg3000::connect(std::unique_ptr<ld_iface> comm)
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
        
        if (tcpip->get_port() != afg3000::PORT) {
            fprintf(stderr, "SDG1000X only supports port %i\n", afg3000::PORT);
            abort();
        }

        // Everything seems to be in order
        m_comm = std::move(tcpip);
    } else if (type == USBTMC) {
        // Convert to usbtmc interface
        unique_ptr<usbtmc_iface> usbtmc(
            dynamic_cast<usbtmc_iface*>(comm.release()));

        // USB initialization
        usbtmc->claim_iface(0);
        usbtmc->set_endpoint_out(0);
        usbtmc->set_endpoint_in(1);

        // Everything seems to be in order
        m_comm = std::move(usbtmc);
    } else if (type == VISA) {
        // Convert to visa interface
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

void afg3000::disconnect()
{
    m_comm.reset();
    return;
}

void afg3000::enable_channel(unsigned channel, bool ena)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "OUTP" << channel << " " << (ena ? "ON" : "OFF") << "\n";
    m_comm->write(msg.str()); 
    return;
}

bool afg3000::get_state(unsigned channel)
{
    this->check_channel(channel);
    string ret = m_comm->query("OUTP" + to_string(channel) + "?\n");
    if (ret.find("ON") != string::npos)
        return true;
    return false;
}

void afg3000::set_wvfm(unsigned channel, waveform wvfm)
{
    this->check_channel(channel);
    switch (wvfm) {
        case SINE:
        case SQUARE:
        case RAMP:
        case PULSE:
        case NOISE:
        case DC:
        break;

        default:
        string msg = this->get_info() + " - Invalid waveform\n";
        fprintf(stderr, "%s\n", msg.c_str());
        abort();
    }
    stringstream msg("");
    msg << ":SOUR" << channel << ":APPL:" << wvfm_to_str(wvfm) << "\n";
    return;
}

fgen::waveform afg3000::get_wvfm(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":APPL?\n";
    string resp = m_comm->query(msg.str());
    for (auto m : m_wvfm_string) {
        if (resp.find(m.second) != string::npos)
            return m.first;
    }
    return SINE;
}

void afg3000::set_freq(unsigned channel, float freq_hz)
{
    return;
}

float afg3000::get_freq(unsigned channel)
{
    return 0;
}

void afg3000::set_duty_cycle(unsigned channel, float dcl)
{
    return;
}

float afg3000::get_duty_cycle(unsigned channel)
{
    return 0;
}

void afg3000::set_phase(unsigned channel, float phase_deg)
{
    return;
}

float afg3000::get_phase(unsigned channel)
{
    return 0;
}

void afg3000::set_ampl(unsigned channel, float ampl_v)
{

    return;
}

float afg3000::get_ampl(unsigned channel)
{
    return 0;
}

void afg3000::set_offset(unsigned channel, float offset_v)
{
    return;
}

float afg3000::get_offset(unsigned channel)
{
    return 0;
}

void afg3000::set_rising(unsigned channel, float rise_s)
{
    return;
}

float afg3000::get_rising(unsigned channel)
{
    return 0;
}

void afg3000::set_falling(unsigned channel, float fall_s)
{
    return;
}

float afg3000::get_falling(unsigned channel)
{
    return 0;
}

void afg3000::set_pulse_width(unsigned channel, float width_s)
{
    return;
}

float afg3000::get_pulse_width(unsigned channel)
{
    return 0;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void afg3000::init() 
{
    m_comm->write("*CLS\n");
    usleep(100e3);
    m_dev_name = m_comm->query("*IDN?\n");
    return;
}

void afg3000::check_channel(unsigned channel) 
{
    if ( (channel == 0) || (channel > this->get_n_channels()) ) {
        fprintf(stderr, "Invalid channel %i\n", channel);
        abort();
    }
    return;
}

}