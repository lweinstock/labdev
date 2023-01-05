#include <labdev/devices/device.hh>
#include <unistd.h>

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

    void device::reconnect(unsigned wait_ms)
    {
        if (m_comm)
            m_comm->close();
        if (wait_ms)
            usleep(wait_ms*1e3);
        m_comm->open();
        return;
    }

    std::string device::get_info() const 
    {
        if ( this->connected() )
            return m_dev_name + ";" + m_comm->get_info();
        return m_dev_name + ";" + "no interface connected";
    }
    
}
