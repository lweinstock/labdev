#include <labdev/devices/rigol/dg4000.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <unistd.h>

using namespace std;

namespace labdev {

dg4000::dg4000(std::unique_ptr<tcpip_iface> tcpip): dg4000()
{
    this->connect(std::move(tcpip));
    return;
}

dg4000::dg4000(std::unique_ptr<visa_iface> visa): dg4000()
{
    this->connect(std::move(visa));
    return;
}

dg4000::dg4000(std::unique_ptr<usbtmc_iface> usbtmc): dg4000()
{
    this->connect(std::move(usbtmc));
    return;
}

dg4000::~dg4000()
{
    if (this->connected())
        this->disconnect();
    return;
}

void dg4000::connect(std::unique_ptr<ld_iface> comm)
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
        
        if ( tcpip->get_port() != dg4000::PORT ) {
            fprintf(stderr, "DG4000 only supports port %i\n", dg4000::PORT);
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
        usbtmc->set_endpoint_out(1);
        usbtmc->set_endpoint_in(2);

        // Everything seems to be in order
        m_comm = std::move(usbtmc);
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

void dg4000::disconnect()
{
    m_comm.reset();
    return;
}


void dg4000::enable_channel(unsigned channel, bool enable) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":OUTP" << channel << ":STAT" << (enable? " ON" : " OFF") << "\n";
    m_comm->write(msg.str());
    return;
}

bool dg4000::get_state(unsigned channel)
{
    this->check_channel(channel);
    string resp = m_comm->query(":OUTP" + to_string(channel) + ":STAT?\n");
    if ( resp.find("ON") != string::npos )
        return true;
    return false;
}

void dg4000::set_wvfm(unsigned channel, waveform wvfm)
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
    m_comm->write(msg.str());
    return;
}

fgen::waveform dg4000::get_wvfm(unsigned channel)
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

void dg4000::set_freq(unsigned channel, float freq_hz)
{
    this->check_channel(channel);
    if (freq_hz < 0) {
        fprintf(stderr, "Invalid frequency %f\n", freq_hz);
        abort();
    }
    stringstream msg("");
    msg << ":SOUR" << channel << ":FREQ " << freq_hz << "\n";
    this->write_at_least(msg.str(), 5);
    return;
}

float dg4000::get_freq(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":FREQ?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dg4000::set_duty_cycle(unsigned channel, float dcl)
{
    this->check_channel(channel);
    if (dcl < 0. || dcl > 1.) {
        fprintf(stderr, "Invalid duty cycle %f\n", dcl);
        abort();
    }
    stringstream msg("");
    msg << ":SOUR" << channel << ":PULS:DCYC " << 100*dcl << "\n";
    this->write_at_least(msg.str(), 5);
    return;
}

float dg4000::get_duty_cycle(unsigned channel)
{
    return 0.;
}

void dg4000::set_phase(unsigned channel, float phase_deg)
{
    this->check_channel(channel);
    if (phase_deg < 0. || phase_deg > 360.) {
        fprintf(stderr, "Invalid phase %f\n", phase_deg);
        abort();
    }
    stringstream msg("");
    msg << ":SOUR" << channel << ":PHAS " << phase_deg << "\n";
    this->write_at_least(msg.str(), 5);
    return;
}

float dg4000::get_phase(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":PHAS?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dg4000::set_ampl(unsigned channel, float ampl_v)
{
    this->check_channel(channel);
    if (ampl_v < 0. || ampl_v > 20.) {
        fprintf(stderr, "Amplitude %f out of range\n", ampl_v);
        abort();
    }
    stringstream msg("");
    msg << ":SOUR" << channel << ":VOLT " << ampl_v << "\n";
    this->write_at_least(msg.str(), 5);
    return;
}

float dg4000::get_ampl(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":VOLT?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dg4000::set_offset(unsigned channel, float offset_v)
{
    this->check_channel(channel);
    if (offset_v < -7.5 || offset_v > 7.5) {
        fprintf(stderr, "Offset %f out of range\n", offset_v);
        abort();
    }
    stringstream msg("");
    msg << ":SOUR" << channel << ":VOLT:OFFS " << offset_v << "\n";
    this->write_at_least(msg.str(), 5);
    return;
}

float dg4000::get_offset(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":VOLT:OFFS?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dg4000::set_rising(unsigned channel, float rise_s)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":PULS:TRAN:LEAD " << rise_s << "\n";
    m_comm->write(msg.str());
    return;
}

float dg4000::get_rising(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":PULS:TRAN:LEAD?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dg4000::set_falling(unsigned channel, float fall_s)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":PULS:TRAN:TRA " << fall_s << "\n";
    m_comm->write(msg.str());
    return;
}

float dg4000::get_falling(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":PULS:TRAN:TRA?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dg4000::set_pulse_width(unsigned channel, float width_s)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":PULS:WIDT " << width_s << "\n";
    m_comm->write(msg.str());
    return;
}

float dg4000::get_pulse_width(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":SOUR" << channel << ":PULS:WIDT?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

/*
 *      P R I V A T E   M E T H O D S
 */

void dg4000::init() 
{
    m_comm->write("*CLS\n");
    usleep(100e3);
    m_dev_name = m_comm->query("*IDN?\n");
    return;
}

void dg4000::check_channel(unsigned channel) 
{
    if ( (channel == 0) || (channel > this->get_n_channels()) ) {
        stringstream msg("");
        msg << this->get_info() << " - invalid channel " << channel << "\n"; 
        fprintf(stderr, "%s\n", msg.str().c_str());
        abort();
    }
    return;
}
/*
std::string dg4000::get_waveform_str(unsigned channel)
{
    this->check_channel(channel);
    string resp = m_comm->query(":SOUR" + to_string(channel) + ":APPL?\n");
    string waveform = resp.substr(resp.find_first_of(','));
    return waveform;
}
*/
void dg4000::write_at_least(string msg, unsigned time_ms) {

    /*
     * Known issue: while processing queries the DG4000 apparently ignores
     *  all following queries instead of storing them in an internal m_command
     *  queue. This makes it neccesary for some write m_commands to take at
     *  least a few milliseconds before returning.
     */

    struct timeval sta, sto;
    gettimeofday(&sta, NULL);
    m_comm->write(msg);
    gettimeofday(&sto, NULL);

    unsigned diff_ms = (sto.tv_sec-sta.tv_sec)*1000 + (sto.tv_usec-sta.tv_usec)/1000;
    if (time_ms > diff_ms)
        usleep( (time_ms - diff_ms)*1000 );
    return;
}

}