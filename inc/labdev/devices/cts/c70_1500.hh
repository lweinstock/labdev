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
    // Returns the current humidity in %rH
    double get_current_humidity();

    void read_analog_channel(unsigned ch, double& actual, double& set);

private:
};

}

#endif