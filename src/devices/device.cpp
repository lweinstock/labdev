#include <labdev/devices/device.hh>

namespace labdev {

    bool device::connected() const { 
        if (m_comm)
            return m_comm->good();
        return false;
    }

    void device::disconnect() 
    {
        // Disconnect and clear communication interface
        if(m_comm) 
        {
			m_comm->close();
            delete m_comm;
            m_comm = nullptr;   // delete does not set pointer to nullptr!
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
