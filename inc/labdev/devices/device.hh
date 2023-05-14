#ifndef DEVICE_HH
#define DEVICE_HH

#include <memory>
#include <labdev/ld_interface.hh>

namespace labdev {

/*
 *  Abstract class for basic devices
 */

class device {
public:
    virtual ~device() {};

    // No copy constructor or assignment, default move constructor
    device(const device&) = delete;
    device& operator=(const device&) = delete;

    // Returns human readable information string to identify the device
    std::string get_info() const { return m_dev_name + ";" + m_comm->get_info(); }
 
protected:
    // Communication interface
    std::shared_ptr<ld_interface> m_comm;

    // Device name
    std::string m_dev_name;

    // Initializer with name for derived classes
    device() : m_comm(nullptr), m_dev_name("?") {};
    device(std::string name) : m_comm(nullptr), m_dev_name(name) {};
};

}

#endif