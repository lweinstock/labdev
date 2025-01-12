#include <labdev/devices/siglent/sdg1000x.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <iomanip>

using namespace std;

namespace labdev {

sdg1000x::sdg1000x() : fgen(2, "Siglent,SDG1000X")
{
    return;
}

sdg1000x::sdg1000x(std::unique_ptr<tcpip_iface> tcpip) : sdg1000x()
{
    this->connect(std::move(tcpip));
    return;
}

sdg1000x::sdg1000x(std::unique_ptr<usbtmc_iface> usbtmc) : sdg1000x()
{
    this->connect(std::move(usbtmc));
    return;
}

sdg1000x::sdg1000x(std::unique_ptr<visa_iface> visa) : sdg1000x()
{
    this->connect(std::move(visa));
    return;
}

sdg1000x::~sdg1000x() 
{
    if (this->connected())
        this->disconnect();
    return;
}

void sdg1000x::connect(std::unique_ptr<ld_iface> comm)
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
        
        if (tcpip->get_port() != sdg1000x::PORT) {
            fprintf(stderr, "SDG1000X only supports port %i\n", sdg1000x::PORT);
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
        usbtmc->set_endpoint_in(0);
        usbtmc->set_endpoint_out(1);

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

void sdg1000x::disconnect()
{
    m_comm.reset();
    return;
}

void sdg1000x::enable_channel(unsigned channel, bool ena)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":OUTP " << (ena ? "ON" : "OFF") << "\n";
    m_comm->write(msg.str()); 
    return;
}

bool sdg1000x::get_state(unsigned channel)
{
    this->check_channel(channel);
    string ret = m_comm->query("C" + to_string(channel) + ":OUTP?\n");
    if (ret.find("ON") != string::npos)
        return true;
    return false;
}

void sdg1000x::set_wvfm(unsigned channel, waveform wvfm)
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
    msg << "C" << channel << ":BSWV WVTP," << wvfm_to_str(wvfm) << "\n";
    m_comm->write(msg.str()); 
    return;
}

fgen::waveform sdg1000x::get_wvfm(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string wvfm = this->get_bswv_val(bswv, "FRQ");
    if (wvfm.empty())  // Parameter not found
        return SINE;
    debug_print("Received waveform %s\n", wvfm.c_str());
    for (auto m : m_wvfm_string) {
        if (wvfm.find(m.second) != string::npos)
            return m.first;
    }
    return SINE;
}

void sdg1000x::set_freq(unsigned channel, float freq_hz)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV FRQ,";
    msg << setprecision(3) << fixed << freq_hz << "\n";
    m_comm->write(msg.str());
    return;
}

float sdg1000x::get_freq(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string freq = this->get_bswv_val(bswv, "FRQ");
    if (freq.empty())  // Parameter not found
        return 0.;
    debug_print("Received frequency %s\n", freq.c_str());
    return stof(freq);
}

void sdg1000x::set_duty_cycle(unsigned channel, float dcl)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV DUTY,";
    msg << setprecision(3) << fixed << 100.*dcl << "\n";
    m_comm->write(msg.str());
    return;
}

float sdg1000x::get_duty_cycle(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string dcl = this->get_bswv_val(bswv, "DUTY");
    if (dcl.empty())  // Parameter not found
        return 0.;
    debug_print("Received duty cycle %s\n", dcl.c_str());
    return 0.01*stof(dcl);
}

void sdg1000x::set_phase(unsigned channel, float phase_deg)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV PHSE,";
    msg << setprecision(3) << fixed << phase_deg << "\n";
    m_comm->write(msg.str());
    return;
}

float sdg1000x::get_phase(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string phase = this->get_bswv_val(bswv, "PHSE");
    if (phase.empty())  // Parameter not found
        return 0.;
    debug_print("Received phase %s\n", phase.c_str());
    return stof(phase);
}

void sdg1000x::set_ampl(unsigned channel, float ampl_v)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV AMP,";
    msg << setprecision(3) << fixed << ampl_v << "\n";
    m_comm->write(msg.str());
    return;
}

float sdg1000x::get_ampl(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string ampl = this->get_bswv_val(bswv, "AMP");
    if (ampl.empty())  // Parameter not found
        return 0.;
    debug_print("Received amplitude %s\n", ampl.c_str());
    return stof(ampl);
}

void sdg1000x::set_offset(unsigned channel, float offset_v)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV OFST,";
    msg << setprecision(3) << fixed << offset_v << "\n";
    m_comm->write(msg.str());
    return;
}

float sdg1000x::get_offset(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string offset = this->get_bswv_val(bswv, "OFST");
    if (offset.empty())  // Parameter not found
        return 0.;
    debug_print("Received amplitude %s\n", offset.c_str());
    return stof(offset);
}

void sdg1000x::set_rising(unsigned channel, float rise_s)
{
    return;
}

float sdg1000x::get_rising(unsigned channel)
{
    return 0;
}

void sdg1000x::set_falling(unsigned channel, float fall_s)
{
    return;
}

float sdg1000x::get_falling(unsigned channel)
{
    return 0;
}

void sdg1000x::set_pulse_width(unsigned channel, float width_s)
{
    return;
}

float sdg1000x::get_pulse_width(unsigned channel)
{
    return 0;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void sdg1000x::init() 
{
    m_comm->write("*CLS\n");
    usleep(100e3);
    m_dev_name = m_comm->query("*IDN?\n");
    return;
}

void sdg1000x::check_channel(unsigned channel) 
{
    if ( (channel == 0) || (channel > this->get_n_channels()) ) {
        fprintf(stderr, "Invalid channel %i\n", channel);
        abort();
    }
    return;
}

std::string sdg1000x::get_bswv_val(std::string bswv, std::string par) 
{
    size_t pos1 = bswv.find(par);
    if (pos1 == string::npos)   // Parameter not found, return empty string
        return {};
    pos1 += par.size() + 1;
    // Values and parameters are separated by commas
    size_t pos2 = bswv.find(',', pos1 + 1);
    return bswv.substr(pos1, pos2 - pos1);
}

}