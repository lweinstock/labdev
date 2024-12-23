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

}