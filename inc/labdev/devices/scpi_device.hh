#ifndef SCPI_DEVICE_HH
#define SCPI_DEVICE_HH

#include <labdev/interface.hh>
#include <labdev/devices/device.hh>

namespace labdev {

    /*
     *      Base class for devices using Standard Commands for Programmable
     *      Instruments (SCPI)
     */

    class scpi_device : public device {
    public:
        scpi_device() : device() {};
        virtual ~scpi_device();

        // Clear read/write buffers and status register
        void clear_status();

        // Returns a unique identifier string for the device
        std::string get_identifier();

        // Returns true if all pending operations have been completed on the
        // device
        bool operation_complete(unsigned timeout_ms = 10000);

        // Returns when all pending operations have been completed on the device
        // (blocking)
        void wait_to_complete(unsigned timeout_ms = 10000);

        // Resets the device
        void reset();

        // Perfoms a self-test, returns true on success
        bool test();

        // Wait until all pending operations have been completed before execute
        // new commands
        void wait_to_continue();

        // Read error queue and return error value
        int get_error();

        // Returns error string for current error in queue
        std::string get_strerror() { return m_strerror; }

        // Read and reset the Standard Event Status Register (SESR)
        uint8_t get_event_status_register(unsigned timeout_ms = 1000);

    protected:
        // Initializer with name for derived classes
        scpi_device(std::string name) : device(name) {};

        // Holds current error information
        int m_error {0};
        std::string m_strerror {""};

    private:
        // Standard Event Status Register (SESR) definitions
        enum SESR : uint8_t {
            OPC = (1 << 0),     // Operation Complete
            RQC = (1 << 1),     // Request Control
            QYE = (1 << 2),     // Query Error
            DDE = (1 << 3),     // Device Dependant Error
            EXE = (1 << 4),     // Execution Error
            CME = (1 << 5),     // Command Error
            URQ = (1 << 6),     // User Request
            PON = (1 << 7)      // Power On
        };
    };
}

#endif