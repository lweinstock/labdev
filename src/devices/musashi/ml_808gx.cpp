#include <sstream>
#include <iomanip>
#include <tuple>

#include <labdev/devices/musashi/ml_808gx.hh>
#include <labdev/serial_interface.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

using namespace std;

namespace labdev {

// Static variables
const string ml_808gx::STX = "\x02";
const string ml_808gx::ETX = "\x03";
const string ml_808gx::EOT = "\x04";
const string ml_808gx::ENQ = "\x05";    
const string ml_808gx::ACK = "\x06";
const string ml_808gx::A0 = STX + "02A02D" + ETX;
const string ml_808gx::A2 = STX + "02A22B" + ETX; 
const string ml_808gx::CAN = STX + "0218186E" + ETX;

ml_808gx::ml_808gx(serial_interface* ser) : ml_808gx()
{
    this->connect(ser);
    return;
}

ml_808gx::~ml_808gx()
{
    if (this->connected())
        this->disconnect();
    return;
}

void ml_808gx::connect(serial_interface* ser)
{
    if ( this->connected() ) {
        string err = this->get_info() + " : device is already connected";
        throw device_error(err);
        return;
    }

    // Check and assign interface
    this->set_comm(ser);

    // 8N1, supported baud = 9600/19200/38400 (see manual p. 24)
    unsigned baud = ser->get_baud();
    if ( baud != 9600  && baud != 19200 && baud != 38400) {
        fprintf(stderr, "Baud %i is not supported by ML-808GX\n", baud);
        abort();
    }
    if (ser->get_nbits() != 8) {
        fprintf(stderr, "Invalid number of bits %u (8N1 required)\n", 
            ser->get_nbits());
        abort();
    }
    if (ser->get_parity() != false) {
        fprintf(stderr, "Invalid parity (8N1 required)\n");
        abort();
    }
    if (ser->get_stop_bits() != 1) {
        fprintf(stderr, "Invalid number of stop bits %u (8N1 required)\n", 
            ser->get_stop_bits());
        abort();
    }

    // Everything seems to be in order...
    this->init();
    return;
}

void ml_808gx::disconnect()
{
    this->reset_comm();
    return;
}


void ml_808gx::dispense() 
{
    debug_print("%s\n", "Dispensing");
    string cmd = "DI  ";
    this->download_command(cmd);
    return;
}

void ml_808gx::set_channel(unsigned ch)
{
    if (ch > 399) {
        printf("Invalid channel %i (allowed 0 - 399)\n", ch);
        abort();
    }
    debug_print("Switching channel to %i ...\n", ch);
    string cmd = "CH  ";
    stringstream data("");
    data << setfill('0') << setw(3) << ch;
    this->download_command(cmd, data.str());
    m_cur_ch = ch;
    return;
}

unsigned ml_808gx::get_channel()
{
    debug_print("%s\n", "Reading current channel number...");
    string resp("");
    this->upload_command("UA   ", resp);
    resp = resp.substr(resp.find("D0")+2, 3);
    m_cur_ch = strtoul(resp.c_str(), NULL, 10);
    return m_cur_ch;
}

void ml_808gx::set_channel_params(float pressure_kPa, float dur_ms, 
    float on_delay_ms, float off_delay_ms) 
{
    // Pressure in units of 100 Pa, duratio in ms, on/off delay in 0.1ms
    unsigned p = static_cast<unsigned>(10*pressure_kPa);
    unsigned dt = static_cast<unsigned>(dur_ms);
    unsigned ton = static_cast<unsigned>(10*on_delay_ms);
    unsigned toff = static_cast<unsigned>(10*off_delay_ms);
    if ( (p < 200) || (p > 8000) ) {
        fprintf(stderr, "Invalid pressure %.2f kPa (valid range: 0.2 - 8.0 kPa)\n", 
            pressure_kPa);
        abort();
    }
    if ( (dt < 10) || (dt > 9999) ) {
        fprintf(stderr, "Invalid duration %.2f ms (valid range: 10 - 9999 ms)\n", 
            dur_ms);
        abort();
    }
    if ( (ton < 0) || (ton > 99999) || (ton < 0) || (toff > 99999) ) {
        fprintf(stderr, "Invalid delay %.2f/%.2f (valid range: 0 - 9.9999 ms)\n", 
            on_delay_ms, off_delay_ms);
        abort();
    }

    debug_print("Setting parameters of ch %i to:\n", m_cur_ch);
    debug_print("p = %i x 100Pa\n", p);
    debug_print("t = %i ms\n", dt);
    debug_print("don/off = %i/%i x 0.1ms\n", ton, toff);
    string cmd = "SC  ";
    stringstream data("");
    data << "CH" << setfill('0') << setw(3) << m_cur_ch;
    data << "P"  << setfill('0') << setw(4) << p;
    data << "T"  << setfill('0') << setw(4) << dt;
    data << "OD" << setfill('0') << setw(5) << ton;
    data << "OF" << setfill('0') << setw(5) << toff;
    this->download_command(cmd, data.str());
    return;
}

void ml_808gx::get_channel_params(float& pressure_kPa, float& dur_ms, 
    float& on_delay_ms, float& off_delay_ms) 
{
    debug_print("Reading parameters of ch %i...\n", m_cur_ch);
    string resp("");
    stringstream cmd("");
    cmd << "GC" << setfill('0') << setw(3) << m_cur_ch;

    this->upload_command(cmd.str(), resp);

    // Extract data from string
    unsigned p    = stoi( resp.substr(resp.find('P')+1, 4) );
    unsigned dt   = stoi( resp.substr(resp.find('T')+1, 4) );
    unsigned ton  = stoi( resp.substr(resp.find("OD")+2, 7) );
    unsigned toff = stoi( resp.substr(resp.find("OF")+2, string::npos) );
    debug_print("p = %i x 100Pa\n", p);
    debug_print("t = %i ms\n", dt);
    debug_print("don/off = %i/%i x 0.1ms\n", ton, toff);

    // Convert into floats
    pressure_kPa = 1e-1*static_cast<float>(p);
    dur_ms       = static_cast<float>(dt);
    on_delay_ms  = 1e-1*static_cast<float>(ton);
    off_delay_ms = 1e-1*static_cast<float>(toff);
    
    return;
}

tuple<float, float, float, float> ml_808gx::get_channel_params()
{
    float p = .0, dt = .0, on = .0, off = .0;
    this->get_channel_params(p, dt, on, off);
    return make_tuple(p, dt, on, off);
}

void ml_808gx::set_pressure(float pressure_kPa)
{
    unsigned p = static_cast<unsigned>(10*pressure_kPa);
    if ( (p < 200) || (p > 8000) ) {
        fprintf(stderr, "Invalid pressure %.2f kPa (valid range: 0.2 - 8.0 kPa)\n", 
            pressure_kPa);
        abort();
    }

    debug_print("Setting pressure of ch %i to %i x 100 Pa\n", m_cur_ch, p);
    string cmd = "PH  ";
    stringstream data("");
    data << "CH" << setfill('0') << setw(3) << m_cur_ch;
    data << "P"  << setfill('0') << setw(4) << p;
    this->download_command(cmd, data.str());
    return;
}

float ml_808gx::get_pressure()
{
    float p = .0, dt = .0, on = .0, off = .0;
    this->get_channel_params(p, dt, on, off);
    return p;
}

void ml_808gx::set_duration(float dur_ms)
{
    unsigned dt = static_cast<unsigned>(dur_ms);
    if ( (dt < 9.999) || (dt > 9999) ) {
        fprintf(stderr, "Invalid duration %.2f ms (valid range: 10 - 9999 ms)\n", 
            dur_ms);
        abort();
    }

    debug_print("Setting duration of ch %i to %i ms\n", m_cur_ch, dt);
    string cmd = "DH  ";
    stringstream data("");
    data << "CH" << setfill('0') << setw(3) << m_cur_ch;
    data << "T"  << setfill('0') << setw(4) << dt;
    this->download_command(cmd, data.str());
    return;
}

float ml_808gx::get_duration()
{   
    float p = .0, dt = .0, on = .0, off = .0;
    this->get_channel_params(p, dt, on, off);
    return dt;
}

void ml_808gx::set_delays(float on_delay_ms, float off_delay_ms)
{
    unsigned ton = static_cast<unsigned>(10*on_delay_ms);
    unsigned toff = static_cast<unsigned>(10*off_delay_ms);
    if ( (ton < 0) || (ton > 99999) || (ton < 0) || (toff > 99999) ) {
        fprintf(stderr, "Invalid delay %.2f/%.2f (valid range: 0 - 9.9999 ms)\n", 
            on_delay_ms, off_delay_ms);
        abort();
    }


    debug_print("Setting delays of ch %i to %i (on) %i (off) x 0.1 ms\n", 
        m_cur_ch, ton, toff);
    string cmd = "DD  ";
    stringstream data("");
    data << "CH" << setfill('0') << setw(3) << m_cur_ch;
    data << "N"  << setfill('0') << setw(5) << ton;
    data << "F"  << setfill('0') << setw(5) << toff;
    this->download_command(cmd, data.str());
    return;
}

void ml_808gx::get_delays(float &on_delay_ms, float &off_delay_ms)
{
    float p = 0, dt = 0;
    this->get_channel_params(p, dt, on_delay_ms, off_delay_ms);
}

tuple<float, float> ml_808gx::get_delays()
{
    float ton = .0, toff = .0;
    this->get_delays(ton, toff);
    return make_tuple(ton, toff);
}


void ml_808gx::manual_mode() 
{
    this->download_command("MT  ");
    return;
}
void ml_808gx::timed_mode() 
{
    this->download_command("TT  ");
    return;
}

void ml_808gx::enable_vacuum(bool ena) 
{
    this->download_command("VO  ", (ena ?  "1" : "0"));
    return;
}

void ml_808gx::set_vacuum_interval(unsigned on_ms, unsigned off_ms) 
{
    if (on_ms > 4000) {
        printf("Vacuum time %i out of range\n", on_ms);
        abort();
    }
    if (off_ms > 4000) {
        printf("Vacuum interval time %i out of range\n", off_ms);
        abort();
    }

    stringstream data("");
    data << "V" << setw(4) << setfill('0') << on_ms;
    data << "I" << setw(4) << setfill('0') << off_ms;
    this->download_command("VI  ", data.str());
    return;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void ml_808gx::init() 
{   
    // Update current channel
    this->get_channel();
    return;
}

void ml_808gx::send_command(string cmd, string data) 
{
    if (cmd.size() < 4)
        throw bad_protocol("Invalid command", cmd.size());
    
    // Build message: 
    //      stx(1) + nchars(2) + cmd(4) + data(n) + checksum(2) + etx(1)
    unsigned nchars = data.size() + cmd.size();
    stringstream msg("");
    msg << STX;
    msg << setw(2) << setfill('0') << uppercase << hex << nchars;
    msg << cmd;
    msg << data;
    // Calculate and add checksum to message
    uint8_t checksum = 0x00;
    for (size_t i = 1; i < msg.str().size(); i++)
        checksum -= msg.str()[i];
    msg << uppercase << hex << (int)checksum;
    msg << ETX;
    get_comm()->write(msg.str());  

    return;
}

void ml_808gx::download_command(string cmd, string data) 
{
    // Initialize enquary
    get_comm()->write(ENQ);
    string resp = get_comm()->read();
    if (resp != ACK)
        throw bad_protocol("Did not receive ACK", resp.size());
    
    this->send_command(cmd, data);
    resp = get_comm()->read_until(ETX);
    
    // Check response
    if ( resp == A2) {
        // Handle error and throw
        get_comm()->write(CAN);
        throw bad_protocol("Received error A2", -1);
    }

    // End transmission
    get_comm()->write(EOT);
    return;
}

void ml_808gx::upload_command(string cmd, string& payload) 
{    
    // Initialize enquary
    get_comm()->write(ENQ);
    string resp = get_comm()->read();
    if (resp != ACK)
        throw bad_protocol("Did not receive ACK", resp.size());
    
    this->send_command(cmd);
    resp = get_comm()->read_until(ETX);

    // Check response
    if (resp == A2) {
        // Handle error and throw
        get_comm()->write(CAN);
        throw bad_protocol("Received error A2", -1);
    }

    get_comm()->write(ACK);
    payload = get_comm()->read_until(ETX);

    // End transmission
    get_comm()->write(EOT);
    return;
}

}
