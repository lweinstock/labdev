#include <labdev/protocols/scpi.hh>
#include <labdev/exceptions.hh>

#include <sys/time.h>   // struct timeval

using namespace std;

namespace labdev {

    scpi::~scpi() {
        return;
    }

    void scpi::clear_status() {
        m_comm->write("*CLS\n");
        return;
    }

    string scpi::get_identifier() {
        string id = m_comm->query("*IDN?\n");
        // Remove last character from return value ('\n')
        return id.substr(0, id.size() - 1);
    }

    bool scpi::operation_complete(unsigned timeout_ms) {
        string msg = m_comm->query("*OPC?\n", timeout_ms);
        if (msg.find("1") != string::npos)
            return true;
        else
            return false;
    }

    void scpi::wait_to_complete(unsigned timeout_ms) {
        // timeout setup
        struct timeval tsta, tsto;
        gettimeofday(&tsta, NULL);
        double tdiff;

        // Check status event status register for OPC-flag
        m_comm->write("*OPC\n");
        while ( (this->get_event_status_register(timeout_ms) & OPC) == 0) {
            // Check for timeout
            gettimeofday(&tsto, NULL);
            tdiff = (tsto.tv_sec - tsta.tv_sec) * 1000.
                + (tsto.tv_usec - tsta.tv_usec)/1000.;
            if (tdiff > timeout_ms)
                throw timeout("*OPC timeout occurred");
        }
        return;
    }

    void scpi::reset() {
        m_comm->write("*RST\n");
        return;
    }

    bool scpi::test() {
        string msg = m_comm->query("*TST?\n");
        if (msg.find("0") != string::npos) return true;
        else return false;
    }

    void scpi::wait_to_continue() {
        m_comm->write("*WAI\n");
        return;
    }

    int scpi::get_error() {
        string msg = m_comm->query("SYST:ERR?\n");
        m_error = stoi( msg.substr(0, msg.find(',')) );
        m_strerror = msg.substr(msg.find('"') + 1,
            msg.rfind('"') - msg.find('"') - 1);
        return m_error;
    }

    uint8_t scpi::get_event_status_register(unsigned timeout_ms) {
        return (uint8_t)stoi( m_comm->query("*ESR?\n", timeout_ms) );
    }

}