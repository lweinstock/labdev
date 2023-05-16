#include <labdev/devices/siglent/sdg1000x.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <iomanip>

using namespace std;

namespace labdev {

sdg1000x::sdg1000x(const ip_address ip) : sdg1000x()
{
    if ( ip.port != sdg1000x::PORT )
    {
        fprintf(stderr, "SDG1000X only supports port %i\n", sdg1000x::PORT);
        abort();
    }
    m_comm = std::make_shared<tcpip_interface>(ip);
    this->init();
    return;
}

sdg1000x::sdg1000x(const usb_config conf) : sdg1000x()
{
    auto usbtmc = std::make_unique<usbtmc_interface>(conf);
    // USB initialization
    usbtmc->claim_interface(0);
    usbtmc->set_endpoint_in(0x01);
    usbtmc->set_endpoint_out(0x01);
    m_comm = std::move(usbtmc);
    this->init();
    return;
}

sdg1000x::sdg1000x(const visa_identifier visa) : sdg1000x()
{
    m_comm = std::make_shared<visa_interface>(visa);
    this->init();
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

void sdg1000x::set_sine(unsigned channel, float freq_hz, float ampl_v, 
    float offset_v, float phase_deg)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV WVTP,SINE,";
    msg << setprecision(3) << fixed;
    msg << "FRQ,"   << freq_hz << ",";
    msg << "AMP,"   << ampl_v << ",";
    msg << "OFST,"  << offset_v << ",";
    msg << "PHSE,"  << phase_deg << "\n";
    m_comm->write(msg.str());
    m_scpi->wait_to_complete();
    return;
}


void sdg1000x::set_square(unsigned channel, float freq_hz, float ampl_v, 
    float offset_v, float phase_deg, float duty_cycle)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV WVTP,SQUARE,";
    msg << setprecision(3) << fixed;
    msg << "FRQ,"   << freq_hz << ",";
    msg << "AMP,"   << ampl_v << ",";
    msg << "OFST,"  << offset_v << ",";
    msg << "PHSE,"  << phase_deg << ",";
    msg << "DUTY,"  << 100*duty_cycle << "\n";
    m_comm->write(msg.str());
    m_scpi->wait_to_complete();
    return;
}

void sdg1000x::set_ramp(unsigned channel, float freq_hz, float ampl_v, 
    float offset_v, float phase_deg, float symm)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV WVTP,RAMP,";
    msg << setprecision(3) << fixed;
    msg << "FRQ,"   << freq_hz << ",";
    msg << "AMP,"   << ampl_v << ",";
    msg << "OFST,"  << offset_v << ",";
    msg << "PHSE,"  << phase_deg << ",";
    msg << "SYM,"  << 100*symm << "\n";
    m_comm->write(msg.str());
    m_scpi->wait_to_complete();
    return;
}

void sdg1000x::set_pulse(unsigned channel, float period_s, float width_s, 
    float delay_s, float high_v, float low_v, float rise_s, float fall_s)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV WVTP,PULSE,";
    msg << setprecision(10) << fixed;
    msg << "PERI,"  << period_s << ",";
    msg << "WIDTH," << width_s << ",";
    msg << "DLY,"   << delay_s << ",";
    msg << "HLEV,"  << high_v << ",";
    msg << "LLEV,"  << low_v << ",";
    msg << "RISE,"  << rise_s << ",";
    msg << "FALL,"  << fall_s << "\n";
    m_comm->write(msg.str());
    m_scpi->wait_to_complete();
    return;
}

void sdg1000x::set_noise(unsigned channel, float mean_v, float stdev_v)
{
    this->check_channel(channel);
    stringstream msg;
    msg << "C" << channel << ":BSWV WVTP,NOISE,";
    msg << setprecision(4) << fixed;
    msg << "MEAN,"  << mean_v << ",";
    msg << "STDEV,"  << stdev_v << "\n";
    m_comm->write(msg.str());
    m_scpi->wait_to_complete();
    return;
}


bool sdg1000x::is_sine(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string waveform = this->get_bswv_val(bswv, "WVTP");
    debug_print("Received waveform %s\n", waveform.c_str());
    if ( waveform.compare("SINE") == 0)
        return true;
    return false;
}

bool sdg1000x::is_square(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string waveform = this->get_bswv_val(bswv, "WVTP");
    debug_print("Received waveform %s\n", waveform.c_str());
    if ( waveform.compare("SQUARE") == 0)
        return true;

    return false;
}

bool sdg1000x::is_ramp(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string waveform = this->get_bswv_val(bswv, "WVTP");
    debug_print("Received waveform %s\n", waveform.c_str());
    if ( waveform.compare("RAMP") == 0)
        return true;
    return false;
}

bool sdg1000x::is_pulse(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string waveform = this->get_bswv_val(bswv, "WVTP");
    debug_print("Received waveform %s\n", waveform.c_str());
    if ( waveform.compare("PULSE") == 0)
        return true;
    return false;
}

bool sdg1000x::is_noise(unsigned channel)
{
    this->check_channel(channel);
    string bswv = m_comm->query("C" + to_string(channel) + ":BSWV?\n");
    string waveform = this->get_bswv_val(bswv, "WVTP");
    debug_print("Received waveform %s\n", waveform.c_str());
    if ( waveform.compare("NOISE") == 0)
        return true;
    return false;
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

/*
 *      P R I V A T E   M E T H O D S
 */

void sdg1000x::init() 
{
    // Setup SCPI
    m_scpi = std::make_unique<scpi>(m_comm.get());
    m_scpi->clear_status();
    m_dev_name = m_scpi->get_identifier();
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