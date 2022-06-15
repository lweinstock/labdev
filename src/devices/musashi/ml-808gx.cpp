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
        return;
    }

    ml_808gx::~ml_808gx() {
        return;
    }

    void ml_808gx::dispense() {
        std::string cmd = "DI  ";
        this->download_command(cmd);
        return;
    }

    void ml_808gx::select_channel(unsigned ch) {
        std::string cmd = "CH  ";
        std::stringstream data("");
        data << std::setfill('0') << std::setw(3) << ch;
        this->download_command(cmd, data.str());
        return;
    }

    void ml_808gx::set_channel_params(unsigned ch, unsigned pressure, 
    unsigned dur, unsigned on_delay, unsigned off_delay) {
        std::string cmd = "SC  ";
        std::stringstream data("");
        data << "CH" << std::setfill('0') << std::setw(3) << ch;
        data << "P" << std::setfill('0') << std::setw(4) << pressure;
        data << "T" << std::setfill('0') << std::setw(4) << dur;
        data << "OD" << std::setfill('0') << std::setw(5) << on_delay;
        data << "OF" << std::setfill('0') << std::setw(5) << off_delay;
        this->download_command(cmd, data.str());
        return;
    }

    void ml_808gx::get_channel_params(unsigned &ch, unsigned &pressure, 
    unsigned &dur, unsigned &on_delay, unsigned &off_delay) {
        return;
    }

    void ml_808gx::manual_mode() {
        return;
    }
    void ml_808gx::timed_mode() {
        return;
    }

    void ml_808gx::set_vacuum(bool ena) {
        return;
    }
    void ml_808gx::set_vacuum_interval(unsigned on_ms, unsigned off_ms) {
        return;
    }

    void ml_808gx::download_command(std::string cmd, std::string data) {
        // Initialize enquary
        comm->write_raw(&ENQ, 1);
        std::string resp = comm->read();
        if (resp.size() != 1 || resp[0] != ACK)
            throw bad_protocol("Did not receive ACK", resp.size());

        if (cmd.size() != 4)
            throw bad_protocol("Command has to have 4 bytes", cmd.size());
        
        // Build message: stx(1) + nchars(2) + cmd(4) + data(n) + checksum(2) + etx(1)
        unsigned nchars = data.size() + 4;
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

        // Check response (TODO)
        resp = comm->read();

        // End transmission
        comm->write_raw(&EOT, 1);
        return;
    }

    int ml_808gx::upload_command(std::string cmd, std::string &data) {
        return 0;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    void ml_808gx::init() {
        return;
    }

    // Static variables
    const uint8_t ml_808gx::STX = 0x02;
    const uint8_t ml_808gx::ETX = 0x03;
    const uint8_t ml_808gx::EOT = 0x04;
    const uint8_t ml_808gx::ENQ = 0x05;    
    const uint8_t ml_808gx::ACK = 0x06;
    const char ml_808gx::A0[] = {STX, '0', '2', 'A', '0', '2', 'D', ETX};
    const char ml_808gx::A2[] = {STX, '0', '2', 'A', '2', '2', 'B', ETX};
}