#include <labdev/devices/wenglor/p3pc.hh>
#include <labdev/exceptions.hh>
#include <unistd.h>

using namespace std;

namespace labdev 
{

p3pc::p3pc(unique_ptr<serial_iface> ser) : ld_device("P3PC")
{
    this->connect(std::move(ser));
    return;
}

p3pc::~p3pc()
{
    if (this->connected()) 
        this->disconnect();
    return;
}

void p3pc::connect(unique_ptr<ld_iface> comm)
{
    if ( this->connected() ) {
        string err = this->get_info() + " : device is already connected";
        throw device_error(err);
        return;
    }

    Interface_type type = comm->type();
    if (type == SERIAL) {
        // Convert to tcpip interface
        unique_ptr<serial_iface> ser(
            dynamic_cast<serial_iface*>(comm.release()));
        
         // Check baud -> 115200 baud
         if ( ser->get_baud() != p3pc::BAUD ) {
            fprintf(stderr, "FY6900 only supports %i baud 8N1\n", p3pc::BAUD);
            abort();
        }

        // Everything seems to be in order
        m_comm = std::move(ser);
    } else {
        string err = this->get_info() + " : interface is not supported";
        throw device_error(err); 
    } 

    this->init();
}

void p3pc::disconnect()
{
    // Shutdown sequence; maybe port is closed? (see above)
    vector<uint8_t> resp = m_comm->query_byte({0x03, 0x40, 0x21, 0x00});
    m_comm.reset();
    return;
}

float p3pc::get_distance()
{
    auto resp = this->get_process_data();
    uint32_t dist = (resp[3] << 24) | (resp[4] << 16) 
                  | (resp[5] <<  8) | (resp[6] <<  0);
    return 1e-3 * static_cast<float>(dist);
}

std::string p3pc::get_vendor_name()
{
    // Vendor name: index 0x0010
    vector<uint8_t> resp = m_comm->query_byte({0x05, 0x76, 0x26, 0x10, 0x00, 0x00});
    usleep(10e3);

    // Update On-request Data buffer
    this->get_process_data();

    // Get On-request Data
    auto name = this->get_on_request_data();
    return string(name.begin() + 3, name.end());
}

std::string p3pc::get_vendor_text()
{
    // Vendor name: index 0x0010
    vector<uint8_t> resp = m_comm->query_byte({0x05, 0x67, 0x26, 0x11, 0x00, 0x00});
    usleep(10e3);

    // Update On-request Data buffer
    this->get_process_data();
    
    // Get On-request Data
    auto name = this->get_on_request_data();
    return string(name.begin() + 3, name.end());
}

/*
 *  P R I V A T E   M E T H O D S
 */

void p3pc::init()
{
    vector<uint8_t> resp {};
    // Both queries are required for the LDS to work;
    // According to the IO-Link spec there is a Standardized Master
    // Interface (SMI), that does Port Identification and Configuration
    // (see overview on p.228). Maybe this is whats happening here?
    resp = m_comm->query_byte({0x04, 0x73, 0x11, 0x09, 0x03});
    resp = m_comm->query_byte({0x03, 0x70, 0x21, 0x03});
    usleep(100e3);
    return;
}

std::vector<uint8_t> p3pc::get_data(std::vector<uint8_t> request)
{
    m_comm->write_byte(request);
    usleep(100e3);              // give it some time...

    uint8_t len {};
    m_comm->read_raw(&len, 1);  // First byte is length of message

    vector<uint8_t> resp {};
    while (resp.size() < len)   // Read until full message was received
    {
        auto temp = m_comm->read_byte(len);
        resp.insert(resp.end(), temp.begin(), temp.end());
        usleep(10e3);
    }
    return resp;
}

std::vector<uint8_t> p3pc::get_process_data()
{
    return this->get_data({0x02, 0x70, 0x22});  // Magic numbers!
}

std::vector<uint8_t> p3pc::get_on_request_data()
{
    return this->get_data({0x02, 0x73, 0x27});  // Magic numbers!
}

}