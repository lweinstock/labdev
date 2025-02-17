#ifndef DEVICE_HH
#define DEVICE_HH

#include <memory>

#include <labdev/ld_iface.hh>

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

    // Connect to provided communication interface
    virtual void connect(std::unique_ptr<ld_iface> comm) = 0;

    // Disconnect device from communication interface
    virtual void disconnect() = 0;

    // Re-establish connection
    void reconnect();

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

    // Communication interface
    std::unique_ptr<ld_iface> m_comm;
};

}

#endif