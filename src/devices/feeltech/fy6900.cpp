#include <labdev/devices/feeltech/fy6900.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <unistd.h>
#include <math.h>
#include <sstream>
#include <iomanip>

using namespace std;

namespace labdev {

fy6900::fy6900(serial_interface* ser) : fy6900()
{
    this->connect(ser);
    return;
}

void fy6900::connect(serial_interface* ser)
{
    // Check and assign interface
    this->set_comm(ser);

    if ( ser->get_baud() != fy6900::BAUD ) {
        fprintf(stderr, "FY6900 only supports %i baud 8N1\n", fy6900::BAUD);
        abort();
    }

    return;
}

void fy6900::enable_channel(unsigned channel, bool on_off) 
{
    stringstream msg;
    if (channel == 1) {
        msg << "WMN" << (on_off ? "1" : "0") << "\n";
    } else if (channel == 2) {
        msg << "WFN" << (on_off ? "1" : "0") << "\n";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string ret = get_comm()->query(msg.str());

    if ( ret.compare("\n") != 0 )
        throw device_error("Did not receive EOM response\n", -1);

    return;
}

bool fy6900::get_state(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMN\n";
    } else if (channel == 2) {
        msg = "RFN\n";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = get_comm()->query(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    int stat = stoi(resp);
    return (stat == 0) ? false : true;
}

void fy6900::set_sine(unsigned channel, float freq_hz, float ampl_v, 
    float offset_v, float phase_deg)
{
    this->set_waveform(channel, 0);    
    this->set_freq(channel, freq_hz);
    this->set_ampl(channel, ampl_v);
    this->set_offset(channel, offset_v);
    this->set_phase(channel, phase_deg);
    return;
}


void fy6900::set_square(unsigned channel, float freq_hz, float ampl_v, 
    float offset_v, float phase_deg, float duty_cycle)
{
    this->set_waveform(channel, 2);
    this->set_freq(channel, freq_hz);
    this->set_ampl(channel, ampl_v);
    this->set_offset(channel, offset_v);
    this->set_phase(channel, phase_deg);
    this->set_duty_cycle(channel, duty_cycle);
    return;
}

void fy6900::set_ramp(unsigned channel, float freq_hz, float ampl_v, 
    float offset_v, float phase_deg, float symm)
{
    // FY6800 can only do triangle or pos/neg ramp (see manual p.6)
    if (symm == 0.0) 
        this->set_waveform(channel, 9);
    else if (symm == 1.0) 
        this->set_waveform(channel, 8);
    else 
        this->set_waveform(channel, 7);
    this->set_freq(channel, freq_hz);
    this->set_ampl(channel, ampl_v);
    this->set_offset(channel, offset_v);
    this->set_phase(channel, phase_deg);
    return;
}

void fy6900::set_pulse(unsigned channel, float period_s, float width_s, 
    float delay_s, float high_v, float low_v, float rise_s, float fall_s)
{
    // Note: unfortunately there is no option to set rising or falling edges 
    // for the FY6900...
    float freq_hz = 1/period_s;
    float offset_v = low_v;
    float ampl_v = high_v - low_v;
    float phase_deg = delay_s/period_s * 360.;
    this->set_waveform(channel, 5);
    this->set_freq(channel, freq_hz);
    this->set_ampl(channel, ampl_v);
    this->set_offset(channel, offset_v);
    this->set_phase(channel, phase_deg);
    this->set_pulse_width(channel, width_s);
    return;
}

void fy6900::set_noise(unsigned channel, float mean_v, float stdev_v)
{
    this->set_waveform(channel, 27);
    this->set_offset(channel, mean_v);
    this->set_ampl(channel, stdev_v);
    return;
}


bool fy6900::is_sine(unsigned channel)
{
    if (this->get_waveform(channel) == 0)
        return true;
    return false;
}

bool fy6900::is_square(unsigned channel)
{
    if (this->get_waveform(channel) == 2)
        return true;
    return false;
}

bool fy6900::is_ramp(unsigned channel)
{
    unsigned wvfm = this->get_waveform(channel);
    if ( (wvfm == 7) || (wvfm == 8) || (wvfm == 9) )
        return true;
    return false;
}

bool fy6900::is_pulse(unsigned channel)
{
     if (this->get_waveform(channel) == 5)
        return true;
    return false;
}

bool fy6900::is_noise(unsigned channel)
{
     if (this->get_waveform(channel) == 27)
        return true;
    return false;
}

void fy6900::set_freq(unsigned channel, float freq_hz) 
{
    stringstream msg;
    if (channel == 1) {
        msg << "WMF";
    } else if (channel == 2) {
        msg << "WFF";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    if ( (freq_hz < 0) || (freq_hz > 60e6) ) {
        fprintf(stderr, "Invalid frequency %f\n", freq_hz);
        abort();
    }
    // Frequency is set in steps of uHz!
    msg << setw(14) << setfill('0') << fixed << setprecision(0); 
    msg << 1e6*freq_hz << "\n";
    string ret = get_comm()->query(msg.str());
    if ( ret.compare("\n") != 0 )
        throw device_error("Did not receive EOM response\n", -1);

    return;
}

void fy6900::set_duty_cycle(unsigned channel, float dcl) 
{
    stringstream msg;
    if (channel == 1) {
        msg << "WMD";
    } else if (channel == 2) {
        msg << "WFD";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    if ( (dcl < 0) || (dcl > 1.) ) {
        fprintf(stderr, "Invalid duty cycle %.3f\n", dcl);
        abort();
    }
    // duty cycle is given in % (format see manual p. 9)
    msg << setw(8) << setfill('0') << fixed << setprecision(3);
    msg << 100*dcl << "\n";
    string ret = get_comm()->query(msg.str());

    if ( ret.compare("\n") != 0 )
        throw device_error("Did not receive EOM response\n", -1);

    return;
}

void fy6900::set_phase(unsigned channel, float phase_deg) 
{
    stringstream msg;
    if (channel == 1) {
        msg << "WMP";
    } else if (channel == 2) {
        msg << "WFP";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    if ( (phase_deg < 0) || (phase_deg >= 360.) ) {
        fprintf(stderr, "Invalid phase value %.3f\n", phase_deg);
        abort();
    }
    // Phase is given in degree (format see manual p. 9)
    msg << setw(9) << setfill('0') << fixed << setprecision(3);
    msg << phase_deg << "\n";
    string ret = get_comm()->query(msg.str());

    if ( ret.compare("\n") != 0 )
        throw device_error("Did not receive EOM response\n", -1);

    return;
}

void fy6900::set_ampl(unsigned channel, float ampl_v) 
{
    stringstream msg;
    if (channel == 1) {
        msg << "WMA";
    } else if (channel == 2) {
        msg << "WFA";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    if ( (ampl_v < 0) || (ampl_v > 24.) ) {
        fprintf(stderr, "Invalid amplitude %.4f\n", ampl_v);
        abort();
    }
    // Amplitude in volts (format see manual p. 8)
    msg << setw(11) << setfill('0') << fixed << setprecision(4);
    msg << ampl_v << "\n";
    string ret = get_comm()->query(msg.str());

    if ( ret.compare("\n") != 0 )
        throw device_error("Did not receive EOM response\n", -1);

    return;
}

void fy6900::set_offset(unsigned channel, float offset_v) 
{
    stringstream msg;
    if (channel == 1) {
        msg << "WMO";
    } else if (channel == 2) {
        msg << "WFO";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    if ( (offset_v < -12.) || (offset_v > 12.0) ) {
        fprintf(stderr, "Invalid offset %.4f\n", offset_v);
        abort();
    }
    // Offset voltage in volts (format see manual p. 8)
    msg << setw(4) << setfill('0') << fixed << setprecision(2);
    msg << offset_v << "\n";
    string ret = get_comm()->query(msg.str());

    if ( ret.compare("\n") != 0 )
        throw device_error("Did not receive EOM response\n", -1);

    return;
}

float fy6900::get_freq(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMF\n";
    } else if (channel == 2) {
        msg = "RFF\n";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = get_comm()->query(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    return stof(resp);
}

float fy6900::get_duty_cycle(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMD\n";
    } else if (channel == 2) {
        msg = "RFD\n";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = get_comm()->query(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    float ret = stoi(resp)/10000.;
    return ret;
}

float fy6900::get_phase(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMP\n";
    } else if (channel == 2) {
        msg = "RFP\n";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = get_comm()->query(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    return stoi(resp)/1000.;
}

float fy6900::get_ampl(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMA\n";
    } else if (channel == 2) {
        msg = "RFA\n";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = get_comm()->query(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    return stoi(resp)/10000.;
}

float fy6900::get_offset(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMO\n";
    } else if (channel == 2) {
        msg = "RFO\n";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = get_comm()->query(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    float ret = (int32_t)stol(resp)/1000.;
    return ret;
}


/*
 *      P R I V A T E   M E T H O D S
 */

void fy6900::set_waveform(unsigned channel, unsigned wvfm)
{
    stringstream msg;
    if (channel == 1) {
        msg << "WMW" << wvfm << "\n";
    } else if (channel == 2) {
        msg << "WFW" << wvfm << "\n";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string ret = get_comm()->query(msg.str());
    if ( ret.compare("\n") != 0 )
        throw device_error("Did not receive EOM response\n", -1);
    return;
}

unsigned fy6900::get_waveform(unsigned channel)
{
    string msg;
    if (channel == 1) {
        msg = "RMW\n";
    } else if (channel == 2) {
        msg = "RFW\n";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = get_comm()->query(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
        
    return stoi(resp);
}

void fy6900::set_pulse_width(unsigned channel, float width_s)
{
    stringstream msg;
    if (channel == 1) {
        msg << "WMS";
    } else if (channel == 2) {
        msg << "WFS";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    msg << setw(10) << setfill('0') << setprecision(0) << fixed;
    msg << 1e9*width_s << "\n"; // Pulse width in ns (manual p.9)
    string ret = get_comm()->query(msg.str());
    if ( ret.compare("\n") != 0 )
        throw device_error("Did not receive EOM response\n", -1);
    return;
}

}
