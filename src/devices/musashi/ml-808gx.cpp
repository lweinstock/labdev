#include <sstream>
#include <iomanip>

#include <labdev/devices/musashi/ml-808gx.hh>
#include <labdev/exceptions.hh>
#include "ld_debug.hh"

using std::setw;
using std::setfill;
using std::hex;
using std::uppercase;

namespace labdev {

    ml_808gx::ml_808gx(serial_interface* ser) {
        if (!ser) {
            fprintf(stderr, "Invalid interface pointer\n");
            abort();
        }
        if (!ser->connected()) {
            fprintf(stderr, "Interface is not connected\n");
            abort();
        }
        comm = ser;
        init();
        return;
    }

    ml_808gx::~ml_808gx() {
        return;
    }

    void ml_808gx::dispense() {
        debug_print("%s\n", "Dispensing");
        std::string cmd = "DI  ";
        this->download_command(cmd);
        return;
    }

    void ml_808gx::select_channel(unsigned ch) {
        if (ch > 399) {
            printf("Invalid channel %i (allowed 0 - 399)\n", ch);
            abort();
        }
        debug_print("Switching channel to %i ...\n", ch);
        std::string cmd = "CH  ";
        std::stringstream data("");
        data << setfill('0') << setw(3) << ch;
        this->download_command(cmd, data.str());
        m_cur_ch = ch;
        return;
    }

    void ml_808gx::set_channel_params(unsigned pressure, 
    unsigned dur, unsigned on_delay, unsigned off_delay) {
        debug_print("Setting parameters of ch %i to:\n", m_cur_ch);
        debug_print("p = %i x 100Pa\n", pressure);
        debug_print("t = %i ms\n", dur);
        debug_print("don/off = %i/%i x 0.1ms\n", on_delay, off_delay);
        std::string cmd = "SC  ";
        std::stringstream data("");
        data << "CH" << setfill('0') << setw(3) << m_cur_ch;
        data << "P"  << setfill('0') << setw(4) << pressure;
        data << "T"  << setfill('0') << setw(4) << dur;
        data << "OD" << setfill('0') << setw(5) << on_delay;
        data << "OF" << setfill('0') << setw(5) << off_delay;
        this->download_command(cmd, data.str());
        return;
    }

    void ml_808gx::get_channel_params(unsigned& pressure, 
    unsigned& dur, unsigned& on_delay, unsigned& off_delay) {
        debug_print("Reading parameters of ch %i...\n", m_cur_ch);
        std::string resp("");
        std::stringstream cmd("");
        cmd << "GC" << setfill('0') << setw(3) << m_cur_ch;

        this->upload_command(cmd.str(), resp);

        printf("%s\n", resp.c_str());
        // Extract data from string
        pressure  = std::stoi( resp.substr(resp.find('P')+1, 4) );
        dur       = std::stoi( resp.substr(resp.find('T')+1, 4) );
        on_delay  = std::stoi( resp.substr(resp.find("OD")+2, 7) );
        off_delay = std::stoi( resp.substr(resp.find("OF")+2, std::string::npos) );
        debug_print("p = %i x 100Pa\n", pressure);
        debug_print("t = %i ms\n", dur);
        debug_print("don/off = %i/%i x 0.1ms\n", on_delay, off_delay);
        

        return;
    }

    void ml_808gx::manual_mode() {
        this->download_command("MT  ");
        return;
    }
    void ml_808gx::timed_mode() {
        this->download_command("TT  ");
        return;
    }

    void ml_808gx::enable_vacuum(bool ena) {
        this->download_command("VO  ", (ena ?  "1" : "0"));
        return;
    }

    void ml_808gx::set_vacuum_interval(unsigned on_ms, unsigned off_ms) {
        if (on_ms > 4000) {
            printf("Vacuum time %i out of range\n", on_ms);
            abort();
        }
        if (off_ms > 4000) {
            printf("Vacuum interval time %i out of range\n", off_ms);
            abort();
        }

        std::stringstream data("");
        data << "V" << setw(4) << setfill('0') << on_ms;
        data << "I" << setw(4) << setfill('0') << off_ms;
        this->download_command("VI  ", data.str());
        return;
    }

    void ml_808gx::send_command(std::string cmd, std::string data) {
        if (cmd.size() < 4)
            throw bad_protocol("Invalid command", cmd.size());
        
        // Build message: stx(1) + nchars(2) + cmd(4) + data(n) + checksum(2) + etx(1)
        unsigned nchars = data.size() + cmd.size();
        std::stringstream msg("");
        msg << STX;
        msg << setw(2) << setfill('0') << uppercase << hex << nchars;
        msg << cmd;
        msg << data;
        // Calculate and add checksum to message
        uint8_t checksum = 0x00;
        for (int i = 1; i < msg.str().size(); i++)
            checksum -= msg.str()[i];
        msg << uppercase << hex << (int)checksum;
        msg << ETX;
        comm->write(msg.str());  

        return;
    }

    void ml_808gx::download_command(std::string cmd, std::string data) {
        // Initialize enquary
        comm->write(ENQ);
        std::string resp = comm->read();
        if (resp != ACK)
            throw bad_protocol("Did not receive ACK", resp.size());
        
        this->send_command(cmd, data);
        resp = comm->read_until(ETX);
        
        // Check response
        if ( resp == A2) {
            // Handle error and throw
            comm->write(CAN);
            throw bad_protocol("Received error A2", -1);
        }

        // End transmission
        comm->write(EOT);
        return;
    }

    void ml_808gx::upload_command(std::string cmd, std::string& payload) {    
        // Initialize enquary
        comm->write(ENQ);
        std::string resp = comm->read();
        if (resp != ACK)
            throw bad_protocol("Did not receive ACK", resp.size());
        
        this->send_command(cmd);
        resp = comm->read();

        // Check response
        if (resp == A2) {
            // Handle error and throw
            comm->write(CAN);
            throw bad_protocol("Received error A2", -1);
        }

        comm->write(ACK);
        payload = comm->read_until(ETX);

        // End transmission
        comm->write(EOT);
        return;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    void ml_808gx::init() {
        m_cur_ch = 0;
        this->select_channel(m_cur_ch);
        return;
    }

    // Static variables
    const std::string ml_808gx::STX = "\x02";
    const std::string ml_808gx::ETX = "\x03";
    const std::string ml_808gx::EOT = "\x04";
    const std::string ml_808gx::ENQ = "\x05";    
    const std::string ml_808gx::ACK = "\x06";
    const std::string ml_808gx::A0 = STX + "02A02D" + ETX;
    const std::string ml_808gx::A2 = STX + "02A22B" + ETX; 
    const std::string ml_808gx::CAN = STX + "0218186E" + ETX;
}