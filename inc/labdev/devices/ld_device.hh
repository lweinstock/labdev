#ifndef DEVICE_HH
#define DEVICE_HH

#include <labdev/ld_interface.hh>

namespace labdev {

/*
 *  Abstract class for basic devices
 */

class ld_device {
public:
    virtual ~ld_device() {};

    // No copy constructor or assignment, default move constructor
    ld_device(const ld_device&) = delete;
    ld_device& operator=(const ld_device&) = delete;

    // Resets the communication interface
    void disconnect() { m_comm = nullptr; }

    // Returns true if the device has a valid connection
    bool connected() const { return (m_comm ? m_comm->good() : false); }

    // Returns human readable information string to identify the device
    std::string get_info() const;

protected:
    // Device name
    std::string m_dev_name;

    // Initializer with name for derived classes
    ld_device() : m_dev_name("unkown device"), m_comm(nullptr) {};
    ld_device(std::string name) : m_dev_name(name), m_comm(nullptr) {};

    // Check and set/get the current communication interface
    void set_comm(ld_interface* comm);
    ld_interface* get_comm() const;

private:
    // Communication interface
    ld_interface* m_comm;
};

}

#endif