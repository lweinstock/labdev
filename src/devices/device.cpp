#include <labdev/devices/device.hh>

namespace labdev {

    void device::disconnect() 
    {
        // Disconnect and clear communication interface
        if(m_comm) 
        {
			m_comm->close();
            delete m_comm;
        }
        return;
    }

    std::string device::get_info() const 
    {
        if ( this->connected() )
            return m_dev_name + ";" + m_comm->get_info();
        return m_dev_name + ";" + "no interface connected";
    }
    
}
