#include <labdev/devices/rigol/dg4000.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <unistd.h>

using namespace std;

namespace labdev {

dg4000::dg4000(tcpip_interface* tcpip): dg4000()
{
    this->connect(tcpip);
    return;
}

dg4000::dg4000(visa_interface* visa): dg4000()
{
    this->connect(visa);
    return;
}

dg4000::dg4000(usbtmc_interface* usbtmc): dg4000()
{
    this->connect(usbtmc);
    return;
}

dg4000::~dg4000()
{
    if (this->connected())
        this->disconnect();
    return;
}

void dg4000::connect(tcpip_interface* tcpip)
{
    // Check and set comm interface
    this->set_comm(tcpip);

    if ( tcpip->get_port() != dg4000::PORT ) {
        fprintf(stderr, "DG4000 only supports port %i\n", dg4000::PORT);
        abort();
    }
    this->init();
    return;
}

void dg4000::connect(usbtmc_interface* usbtmc)
{
    // Check and set comm interface
    this->set_comm(usbtmc);

    // USB initialization
    usbtmc->claim_interface(0);
    usbtmc->set_endpoint_out(1);
    usbtmc->set_endpoint_in(2);

    this->init();
    return;
}

void dg4000::connect(visa_interface* visa)
{
    // Check and set comm interface
    this->set_comm(visa);
    this->init();
    return;
}

void dg4000::disconnect()
{
    if (m_scpi) {
        delete m_scpi;
        m_scpi = nullptr;
    }
    this->reset_comm();
    return;
}


void dg4000::enable_channel(unsigned channel, bool enable) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":OUTP" << channel << ":STAT" << (enable? " ON" : " OFF") << "\n";
    get_comm()->write(msg.str());
    return;
}

bool dg4000::get_state(unsigned channel)
{
    return false;
}

void dg4000::set_sine(unsigned channel,float freq_hz, float ampl_v, 
    float offset_v, float phase_deg)
{
    this->check_channel(channel);
    stringstream msg;
    msg << ":SOUR" << channel << ":APPL:SIN ";
    msg << freq_hz << ",";
    msg << ampl_v << ","; 
    msg << offset_v << ",";
    msg << phase_deg << "\n";
    //get_comm()->write_at_least(msg.str(), 10);
    get_comm()->write(msg.str());
    m_scpi->wait_to_complete();
    return;
}

void dg4000::set_square(unsigned channel,float freq_hz, float ampl_v, 
    float offset_v, float phase_deg, float duty_cycle)
{
    return;
}

void dg4000::set_ramp(unsigned channel,float freq_hz, float ampl_v, 
    float offset_v, float phase_deg, float symm)
{
    return;
}

void dg4000::set_pulse(unsigned channel,float period_s, float width_s, 
    float delay_s, float high_v, float low_v, float rise_s, float fall_s)
{
    return;
}

void dg4000::set_noise(unsigned channel,float mean_v, float stdev_v)
{
    return;
}

// TODO: test get waveform mehtods

bool dg4000::is_sine(unsigned channel)
{
    if (this->get_waveform_str(channel) == "SINUSOID")
        return true;
    return false;
}

bool dg4000::is_square(unsigned channel)
{
    if (this->get_waveform_str(channel) == "SQUARE")
        return true;
    return false;
}

bool dg4000::is_ramp(unsigned channel)
{
    if (this->get_waveform_str(channel) == "RAMP")
        return true;
    return false;
}

bool dg4000::is_pulse(unsigned channel)
{
    if (this->get_waveform_str(channel) == "PULSE")
        return true;
    return false;
}

bool dg4000::is_noise(unsigned channel)
{
    if (this->get_waveform_str(channel) == "NOISE")
        return true;
    return false;
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
    string resp = get_comm()->query(msg.str());
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
    string resp = get_comm()->query(msg.str());
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
    string resp = get_comm()->query(msg.str());
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
    string resp = get_comm()->query(msg.str());
    return stof(resp);
}

/*
 *      P R I V A T E   M E T H O D S
 */

void dg4000::init() 
{
    if (m_scpi)
        delete m_scpi;
    m_scpi = new scpi( get_comm() );
    m_scpi->clear_status();
    m_scpi->wait_to_complete();
    return;
}

void dg4000::check_channel(unsigned channel) 
{
    if ( (channel == 0) || (channel > this->get_n_channels()) ) {
        fprintf(stderr, "Invalid channel %i\n", channel);
        abort();
    }
    return;
}

std::string dg4000::get_waveform_str(unsigned channel)
{
    this->check_channel(channel);
    string resp = get_comm()->query(":SOUR" + to_string(channel) + ":APPL?\n");
    string waveform = resp.substr(resp.find_first_of(','));
    return waveform;
}

void dg4000::write_at_least(string msg, unsigned time_ms) {

    /*
     * Known issue: while processing queries the DG4000 apparently ignores
     *  all following queries instead of storing them in an internal m_command
     *  queue. This makes it neccesary for some write m_commands to take at
     *  least a few milliseconds before returning.
     */

    struct timeval sta, sto;
    gettimeofday(&sta, NULL);
    get_comm()->write(msg);
    gettimeofday(&sto, NULL);

    unsigned diff_ms = (sto.tv_sec-sta.tv_sec)*1000 + (sto.tv_usec-sta.tv_usec)/1000;
    if (time_ms > diff_ms)
        usleep( (time_ms - diff_ms)*1000 );
    return;
}

}