#ifndef DEVICE_HH
#define DEVICE_HH

#include <labdev/ld_interface.hh>

namespace labdev {

    /*
     *  Abstract class for basic devices
     */

    class device {
    public:
        device() : m_comm(nullptr), m_dev_name("?") {};
        device(const device&) = delete;
        virtual ~device() { this->disconnect(); }

        // Returns true if the communication interface is ready and working
        bool connected() const;

        // Closes and deletes the communication interface;
        // every device needs to implement an connect(...) method to create and
        // open a communication interface
        void disconnect();

        // Dis- and reconnects the communication interface
        void reconnect(unsigned wait_ms = 0);

        // Returns human readable information string to identify the device
        std::string get_info() const;

    protected:
        // Initializer with name for derived classes
        device(std::string name) : m_comm(nullptr), m_dev_name(name) {};

        // Communication interface
        ld_interface* m_comm;

        // Device name
        std::string m_dev_name;
    };
}

#endif