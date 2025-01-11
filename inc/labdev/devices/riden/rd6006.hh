#ifndef RD6006_HH
#define RD6006_HH

#include <labdev/devices/ld_device.hh>
#include <labdev/modbus_rtu_iface.hh>

namespace labdev {

class rd6006 : public ld_device
{
public:
    rd6006() : ld_device("Riden,RD6006") {};
    rd6006(std::unique_ptr<modbus_rtu_iface> modbus);
    ~rd6006();

    void connect(std::unique_ptr<ld_iface> comm) override;
    void disconnect() override;

    /// Enable output.
    void enable_output(bool ena = true);
    /// Disable output.
    void disable_output() { this->enable_output(false); }
    /// Returns true, if output is enabled.
    bool output_enabled();

    /// Set output voltage in [V].
    void set_voltage(double volts);
    /// Get output voltage in [V].
    double get_voltage();
    /// Set current limit in [A].
    void set_current_limit(double amps);
    /// Get current limit in [A].
    double get_current_limit();

    /// Measure output voltage in [V].
    double measure_voltage();
    /// Measure output current in [A].
    double measure_current();

    /// Set over voltage protection (OVP) in [V]
    void set_ovp(double volts);
    /// Returns true if OPV was tripped
    bool ovp_tripped();

private:
    void init();
    std::unique_ptr<modbus_rtu_iface> m_modbus;

    static constexpr uint16_t UID  = 0x01;      ///< Default unit id of RD6006
    // Register addresses
    static constexpr uint16_t VOUT  = 0x000A;   ///< Output voltage x100
    static constexpr uint16_t IOUT  = 0x000B;   ///< Output current x1000
    static constexpr uint16_t STAT  = 0x0010;   ///< 0 = OK, 1 = OVP, 2 = OCP, 3 = OTP
    static constexpr uint16_t OUTP  = 0x0012;   ///< Output 0 = OFF, 1 = ON
    static constexpr uint16_t MEMID = 0x0013;   ///< Memory settings (ID = 0 - 9)
    static constexpr uint16_t VSET0 = 0x0050;   ///< Set voltage mem id 0
    static constexpr uint16_t ISET0 = 0x0051;   ///< Set current mem id 0
    static constexpr uint16_t OVP0  = 0x0052;   ///< Over voltage protection mem id 0
    static constexpr uint16_t OCP0  = 0x0053;   ///< Over current protection mem id 0



};

}

#endif