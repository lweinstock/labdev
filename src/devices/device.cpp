#include <labdev/devices/device.hh>

namespace labdev {

    device::~device() {
        this->close();
    }

    bool device::good() {
        return (m_comm ? m_comm->connected() : false);
    }

    void device::close() {
        delete m_comm;
        return;
    }

    interface* device::get_interface() { 
        return m_comm;
    }

}