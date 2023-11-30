#include <labdev/devices/ld_device.hh>
#include <labdev/exceptions.hh>

#include <iostream>

namespace labdev{

using namespace std;

std::string ld_device::get_info() const 
{
    return m_dev_name + (m_comm ? ";" + m_comm->get_info() : "");
}

/*
 *      P R O T E C T E D   M E T H O D S
 */

void ld_device::set_comm(ld_interface* comm)
{
    // Only connect if not already connected
    if ( this->connected() ) {
        string err = m_dev_name + ": device is already connected";
        throw device_error(err);
        return;
    }

    // Check if pointer is valid
    if (comm == nullptr) {
        cerr << m_dev_name << ": received nullptr interface" << endl;
        abort();
    }

    // Check if communication interface is ready to use
    if ( !comm->good() ) {
        throw bad_connection(this->get_info() + ": interface not ready");
        return;
    }

    // Everything seems to be in order
    m_comm = comm;
    return;
}

ld_interface* ld_device::get_comm() const
{
    if ( !this->connected() ) {
        std::string err = this->get_info() + ": device is not connected";
        throw bad_connection(err);
        return nullptr;
    }
    return m_comm;
}

}