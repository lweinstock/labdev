#include <labdev/devices/cts/c70_1500.hh>
#include <labdev/ld_debug.hh>
#include <labdev/exceptions.hh>
#include <labdev/utils/utils.hh>

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;

namespace labdev
{

c70_1500::c70_1500(std::unique_ptr<tcpip_iface> tcpip)
{
    this->connect(std::move(tcpip));
    return;
}

c70_1500::~c70_1500()
{
    if (this->connected())
        this->disconnect();
    return;
}

void c70_1500::connect(std::unique_ptr<ld_iface> comm)
{
    if ( this->connected() ) 
    {
        string err = this->get_info() + " : device is already connected";
        throw device_error(err);
        return;
    }

    Interface_type type = comm->type();
    if (type == TCPIP) 
    {
        // Convert to tcpip interface
        unique_ptr<tcpip_iface> tcpip(dynamic_cast<tcpip_iface*>(comm.release()));
        
        // Default port 1080
        if (tcpip->get_port() != c70_1500::PORT) 
        {
            fprintf(stderr, "C-70/1500 only supports port %u.\n", c70_1500::PORT);
            abort();
        }

        // Everything seems to be in order
        m_comm = std::move(tcpip);
    } 
    else 
    {
        string err = this->get_info() + " : interface is not supported";
        throw device_error(err); 
    } 
    return;
}

void c70_1500::run_program(unsigned idx)
{
    stringstream program;
    program << "p" << setw(3) << setfill('0') << idx;
    debug_print("Running program '%s' ...", program.str().c_str());
    auto resp = m_comm->query(program.str());
    if (resp != program.str())
        throw bad_protocol("Received wrong echo '" + resp + "'");
    return;
}


double c70_1500::get_current_temperature()
{
    double temperature {-1}, dummy {-1};
    this->read_analog_channel(0, temperature, dummy);
    return temperature;
}

double c70_1500::get_current_humidity()
{
    double humidity {-1}, dummy {-1};
    this->read_analog_channel(1, humidity, dummy);
    return humidity;
}

void c70_1500::read_analog_channel(unsigned ch, double& actual, double& set)
{
    if (ch > 6) 
    {
        fprintf(stderr, "Invalid channel %u", ch);
        abort();
    }

    string query = "A" + to_string(ch);
    auto resp = m_comm->query(query);
    // Response is split by whitespaces " "
    auto respv = labdev::split(resp, " ");

    // Expect three entries (see ASCII protocol manual p. 7)
    if (respv.size() != 3)
        throw bad_protocol("Expected 3 values, received " + to_string(respv.size()));
    // First entry = "A" + analog channel number
    if (respv.at(0) != query)
        throw bad_protocol("Expected " + query + ", received " + respv.at(0));
    // Second entry = actual value
    bool success = false;
    actual = convert_to<double>(respv.at(1), success);
    if (!success)
        throw bad_protocol("Failed to convert actual value '" + respv.at(1) + "' to double");
    // Third entry = set value
    set = convert_to<double>(respv.at(2), success);
    if (!success)
        throw bad_protocol("Failed to convert set value '" + respv.at(2) + "' to double");

    debug_print("Read '%s': actual = %.3f, set = %.3f\n", resp.c_str(), 
        actual, set);

    return;
}

}