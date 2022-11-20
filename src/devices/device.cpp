#include <labdev/devices/device.hh>

namespace labdev {

    device::~device() {
        this->close();
        return;
    }

    void device::close() {
        m_comm->close();
        return;
    }

    std::string device::get_info() const { 
        if ( this->good() )
            return m_comm->get_info();
        return "no interface connected";
    }
    
}