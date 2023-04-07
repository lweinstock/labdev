#include <labdev/devices/feeltech/fy6900.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <unistd.h>
#include <math.h>
#include <sstream>
#include <iomanip>

using namespace std;

namespace labdev {

fy6900::fy6900() :
fgen(2, "FeelTech,FY6900")
{
    return;
}

fy6900::fy6900(serial_config &ser) :
fgen(2, "FeelTech,FY6900")
{
    this->connect(ser);
    return;
}

void fy6900::connect(serial_config &ser)
{
    if ( this->connected() ) {
        fprintf(stderr, "Device is already connected!\n");
        abort();
    }
    if ( ser.baud != fy6900::BAUD ) {
        fprintf(stderr, "FY6900 only supports %i baud 8N1\n", fy6900::BAUD);
        abort();
    }
    m_comm = new serial_interface(ser);
    return;
}

void fy6900::enable_channel(unsigned channel, bool on_off) 
{
    string msg;
    if (channel == 1) {
        msg = "WMN" + (on_off ? string("1") : string("0") );
    } else if (channel == 2) {
        msg = "WFN" + (on_off ? string("1") : string("0") );
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string ret = this->query_cmd(msg);
    // Response should be empty
    if (ret.size() > 0)
        throw device_error("Received unexpected non-empty response", -1);

    return;
}

bool fy6900::get_state(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMN";
    } else if (channel == 2) {
        msg = "RFN";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = this->query_cmd(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    int stat = stoi(resp);
    return (stat == 0) ? false : true;
}

void fy6900::set_sine(unsigned channel, float freq_hz, float ampl_v, 
    float offset_v, float phase_deg)
{

    return;
}


void fy6900::set_square(unsigned channel, float freq_hz, float ampl_v, 
    float offset_v, float phase_deg, float duty_cycle)
{
    return;
}

void fy6900::set_ramp(unsigned channel, float freq_hz, float ampl_v, 
    float offset_v, float phase_deg, float symm)
{
    return;
}

void fy6900::set_pulse(unsigned channel, float period_s, float width_s, 
    float delay_ms, float high_v, float low_v, float rise_s, float fall_s)
{
    return;
}

void fy6900::set_noise(unsigned channel, float mean_v, float stdev_v)
{
    return;
}


bool fy6900::is_sine(unsigned channel)
{
    return false;
}

bool fy6900::is_square(unsigned channel)
{
    return false;
}

bool fy6900::is_ramp(unsigned channel)
{
    return false;
}

bool fy6900::is_pulse(unsigned channel)
{
    return false;
}

bool fy6900::is_noise(unsigned channel)
{
    return false;
}

void fy6900::set_freq(unsigned channel, float freq_hz) 
{
    string msg;
    if (channel == 1) {
        msg = "WMF";
    } else if (channel == 2) {
        msg = "WFF";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    if ( (freq_hz < 0) || (freq_hz > 60e6) ) {
        fprintf(stderr, "Invalid frequency %f\n", freq_hz);
        abort();
    }
    // Frequency is set in steps of uHz!
    long unsigned freq_uhz = static_cast<long unsigned>(freq_hz*1e6);
    msg += to_string(freq_uhz);
    string ret = this->query_cmd(msg);
    // Response should be empty
    if (ret.size() > 0)
        throw device_error("Received unexpected non-empty response", -1);

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
    msg << setw(8) << setfill('0') << fixed << setprecision(3) << 100*dcl;
    string ret = this->query_cmd(msg.str());
    // Response should be empty
    if (ret.size() > 0)
        throw device_error("Received unexpected non-empty response", -1);

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
    msg << setw(9) << setfill('0') << fixed << setprecision(3) << phase_deg;
    string ret = this->query_cmd(msg.str());
    // Response should be empty
    if (ret.size() > 0)
        throw device_error("Received unexpected non-empty response", -1);

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
    msg << setw(11) << setfill('0') << fixed << setprecision(4) << ampl_v;
    string ret = this->query_cmd(msg.str());
    // Response should be empty
    if (ret.size() > 0)
        throw device_error("Received unexpected non-empty response", -1);

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
    msg << setw(4) << setfill('0') << fixed << setprecision(2) << offset_v;
    string ret = this->query_cmd(msg.str());
    // Response should be empty
    if (ret.size() > 0)
        throw device_error("Received non-empty response", -1);

    return;
}

float fy6900::get_freq(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMF";
    } else if (channel == 2) {
        msg = "RFF";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = this->query_cmd(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    return stof(resp);
}

float fy6900::get_duty_cycle(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMD";
    } else if (channel == 2) {
        msg = "RFD";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = this->query_cmd(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    float ret = stoi(resp)/10000.;
    return ret;
}

float fy6900::get_phase(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMP";
    } else if (channel == 2) {
        msg = "RFP";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = this->query_cmd(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    return stoi(resp)/1000.;
}

float fy6900::get_ampl(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMA";
    } else if (channel == 2) {
        msg = "RFA";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = this->query_cmd(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    return stoi(resp)/10000.;
}

float fy6900::get_offset(unsigned channel) 
{
    string msg;
    if (channel == 1) {
        msg = "RMO";
    } else if (channel == 2) {
        msg = "RFO";
    } else {
        fprintf(stderr, "Invalid channel number %i\n", channel);
        abort();
    }
    string resp = this->query_cmd(msg);
    if (resp.size() == 0)
        throw device_error("Received empty response", -1);
    float ret = (int32_t)stol(resp)/1000.;
    return ret;
}


/*
    *      P R I V A T E   M E T H O D S
    */


string fy6900::query_cmd(string cmd) 
{
    size_t pos = string::npos;
    string ret(""), buf("");
    m_comm->write(cmd + '\n');

    // Read until EOM character is found
    while ( pos == string::npos ) {
        usleep(SLEEP_US);
        buf = m_comm->read();
        if (buf.size() > 0) {
            ret.append(buf);
            pos = ret.find(EOM);
        } else {
            throw device_error("Received no response for query\n");
        }
    }
    // Remove EOM character and return value
    ret = ret.substr(0, pos);
    debug_print("Queried '%s' and received %lu bytes: '%s'\n",
        cmd.c_str(), ret.size(), ret.c_str());
    return ret;
}

}
