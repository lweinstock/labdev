#ifndef LD_C_70_1500_HH
#define LD_C_70_1500_HH

#include <labdev/tcpip_iface.hh>
#include <labdev/devices/ld_device.hh>

namespace labdev
{

class c70_1500 : public ld_device
{
public:
    c70_1500() : ld_device("C-70/1500") {};
    c70_1500(std::unique_ptr<tcpip_iface> tcpip);
    ~c70_1500();

    static constexpr unsigned PORT = 1080;

    void connect(std::unique_ptr<ld_iface> comm) override;
    void disconnect() override { m_comm.reset(); }

    // Runs the specified program
    void run_program(unsigned idx);
    // Stops the current program
    void stop_program() { this->run_program(0); }

    // Returns the current temperature in degC
    double get_current_temperature();
    // Returns the set temperature in degC
    double get_set_temperature();    
    // Returns the current humidity in %rH
    double get_current_humidity();
    // Returns the set humidity in %rH
    double get_set_humidity();
    // Returns the set humidity in %rH
    double get_current_water();    
    // Returns the state of the program
    double get_program_state();
    // Returns the state of the chamber
    double get_chamber_state();
    // Returns the state of the error
    double get_error_state();
    // Returns the state of the deep dehumidification
    double get_deep_dehum_state();
    // Returns the state of the humidity
    double get_hum_state();    
    // Returns the state of the dew_p >7
    double get_dewh7_point();
    // Returns the state of the dew_p <7
    double get_dewl7_point();    

    void read_program_state(double& program_no);
    void read_chamber_state(double& chamber_state, double& error, double& pause, 
        double& temp_state, double& hum_state, double& m2, double& m3, 
        double& deep_dehum, double& dig1, double& dig2, double& drain );
    void read_analog_channel(unsigned ch, double& actual, double& set);


private:
};

}

#endif