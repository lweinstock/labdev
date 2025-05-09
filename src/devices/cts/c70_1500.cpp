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

class c70_1500_exception : public labdev::exception 
{
public:
    c70_1500_exception(const std::string& msg, int err = 0)
      : exception(msg, err) {};
    ~c70_1500_exception() {};
};


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

double c70_1500::get_set_temperature()
{
    double set_temperature {-1}, dummy {-1};
    this->read_analog_channel(0, dummy, set_temperature);
    return set_temperature;
}


double c70_1500::get_current_humidity()
{
    double humidity {-1}, dummy {-1};
    this->read_analog_channel(1, humidity, dummy);
    return humidity;
}

double c70_1500::get_set_humidity()
{
    double set_humidity {-1}, dummy {-1};
    this->read_analog_channel(1, dummy, set_humidity);
    return set_humidity;
}

double c70_1500::get_current_water()
{
    double water {-1}, dummy {-1};
    this->read_analog_channel(2, water, dummy);
    return water;
}

double c70_1500::get_program_state() // if there is a program running, returns the number of the program
{
    double program_state {-1};
    this->read_program_state(program_state);
    return program_state;
}

double c70_1500::get_chamber_state() // Wheter chamber is running or not
{
    double chamber_state {-1}, dummy{-1};
    this->read_chamber_state(chamber_state, dummy, dummy , dummy, dummy, dummy , dummy, dummy, dummy , dummy, dummy );
    return chamber_state;
}

double c70_1500::get_error_state()
{
    double error_state {-1}, dummy{-1};
    this->read_chamber_state(dummy, error_state, dummy , dummy, dummy, dummy , dummy, dummy, dummy , dummy, dummy );
    return error_state;
}


double c70_1500::get_deep_dehum_state()
{
    double deep_dehum {-1}, dummy{-1};
    this->read_chamber_state(dummy, dummy, dummy , dummy, dummy, dummy , dummy, deep_dehum, dummy , dummy, dummy );
    return deep_dehum;
}


double c70_1500::get_hum_state() // if the chamber controls the humidity, its =1
{
    double hum_state {-1}, dummy{-1};
    this->read_chamber_state(dummy, dummy, dummy , dummy, hum_state, dummy , dummy, dummy, dummy , dummy, dummy );
    return hum_state;
}

double c70_1500::get_dewh7_point() // dew point >7
{
    double dew_pointh7 {-1}, dummy{-1};
    this->read_chamber_state(dummy, dummy, dummy , dummy, dummy, dew_pointh7 , dummy, dummy, dummy , dummy, dummy );
    return dew_pointh7;
}

double c70_1500::get_dewl7_point() // dew point <7
{
    double dew_pointl7 {-1}, dummy{-1};
    this->read_chamber_state(dummy, dummy, dummy , dummy, dummy, dummy , dew_pointl7, dummy, dummy , dummy, dummy );
    return dew_pointl7;
}


void c70_1500::read_program_state(double& program_no)
{
    string query = "P";
    auto resp = m_comm->query(query);
    // 4 chararacters expected, first one is a " header" can be ignored, last 3 char. indicates program state
    vector<string> respv = {resp.substr(0 , 1), resp.substr(1)}; 

    // Expect two entry (see ASCII protocol manual p. 28)
    if (respv.size() != 2)
        throw bad_protocol("Expected 2 value, received " + to_string(respv.size()));    
    //  see ASCII protocol manual p. 28, maybe handle it without interrupting
    //if (respv.at(1) == "000")
        //throw bad_protocol("No program is running");
    
    // Second entry = program state indicator
    bool success = false;
    program_no = convert_to<double>(respv.at(1), success);
    if (!success)
        throw bad_protocol("Failed to convert program to '" + respv.at(1) + "' to double");


    return;    
}
/*struct chamber_state
{
    bool chamber_good {false};
    bool has_error {false};
    bool is_paused {false};
    ...
};

chamber_state c70_1500::read_chamber_state()
{
    chamber_state state;
    ...
}

auto state = my_chamber.read_chamber_state();
if (state.chamber_good)
    ...
else if (state.has_error)
    ...*/


void c70_1500::read_chamber_state(double& chamber_state, double& error, double& pause, 
    double& temp_state, double& hum_state, double& m2, double& m3, 
    double& deep_dehum, double& dig1, double& dig2, double& drain ) 
    
// use bool maybe instead of double(?), all the 
{
    string query = "O";
    auto resp = m_comm->query(query);
    vector<string> respv; 
    // First char. skipped, its a " headaer " , see ASCII protocol manual page 25
    for (int i = 1; i <= 11; ++i) {
        respv.push_back(resp.substr(i, 1));
    }
    vector<double*> targets = {&chamber_state, &error, &pause, 
        &temp_state, &hum_state, &m2, &m3, 
        &deep_dehum, &dig1, &dig2, &drain};
        // Functions of Digital Out1&2 is unknown
    vector<string> labels = {
        "Chamber ON/OFF", "Error", "Chamber interrupt", "Temperature state", "Humidity state",
        "Dew point >7°C", "Dew point <7°C", "Deep dehumidity", "Digital Output 1", "Digital Output 2", "Drain"
    };
    for (size_t i = 0; i < targets.size(); ++i) {
        bool success = false;
        *targets[i] = convert_to<double>(respv.at(i), success);
        if (!success)
            throw bad_protocol("Failed to convert " + labels[i] + " '" + respv.at(i) + "' to double");

        if (i == 1 && respv.at(1) == "1") // Error state check
            throw c70_1500_exception("Error is present");
        }
    return;    

    /*try 
    {
        // do thing
        mychamber.read_chamber_state(...);
    }
    catch (c70_1500_exception &ex)
    {
        cerr << "Error with chamber " << ex.what() << endl;
    }*/

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