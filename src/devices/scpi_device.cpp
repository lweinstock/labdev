#include <labdev/devices/scpi_device.hh>
#include <labdev/exceptions.hh>

#include <sys/time.h>   // struct timeval

namespace labdev {

    scpi_device::scpi_device(interface* interface):
    m_error(0),
    m_strerror("No error") {
        if (!interface) {
            fprintf(stderr, "Invalid interface pointer\n");
            abort();
        }
        if (!interface->connected()) {
            fprintf(stderr, "Interface is not connected\n");
            abort();
        }
        comm = interface;
        return;
    }

    scpi_device::~scpi_device() {
        return;
    }

    void scpi_device::clear_status() {
        comm->write("*CLS\n");
        return;
    }

    std::string scpi_device::get_identifier() {
        return comm->query("*IDN?\n");
    }

    bool scpi_device::operation_complete(unsigned timeout_ms) {
        std::string msg = comm->query("*OPC?\n", timeout_ms);
        if (msg.find("1") != std::string::npos)
            return true;
        else
            return false;
    }

    void scpi_device::wait_to_complete(unsigned timeout_ms) {
        // timeout setup
        struct timeval tsta, tsto;
        gettimeofday(&tsta, NULL);
        double tdiff;

        // Check status event status register for OPC-flag
        comm->write("*OPC\n");
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

    void scpi_device::reset() {
        comm->write("*RST\n");
        return;
    }

    bool scpi_device::good() {
        std::string msg = comm->query("*TST?\n");
        if (msg.find("0") != std::string::npos) return true;
        else return false;
    }

    void scpi_device::wait_to_continue() {
        comm->write("*WAI\n");
        return;
    }

    int scpi_device::get_error() {
        std::string msg = comm->query("SYST:ERR?\n");
        m_error = std::stoi( msg.substr(0, msg.find(',')) );
        m_strerror = msg.substr(msg.find('"') + 1,
            msg.rfind('"') - msg.find('"') - 1);
        return m_error;
    }

    uint8_t scpi_device::get_event_status_register(unsigned timeout_ms) {
        return (uint8_t)std::stoi( comm->query("*ESR?\n", timeout_ms) );
    }

}