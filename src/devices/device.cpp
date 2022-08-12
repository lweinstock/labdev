#include <labdev/devices/device.hh>

namespace labdev {

    device::~device() {
        this->close();
    }

    void device::close() {
        delete m_comm;
        return;
    }

    std::string device::get_info() const { 
        if ( this->good() )
            return m_comm->get_info();
        return "no interface connected";
    }
    
}