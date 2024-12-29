#ifndef UT61B_H
#define UT61B_H

#include <labdev/serial_iface.hh>
#include <labdev/devices/ld_device.hh>
#include <memory>

namespace labdev {

/*
 *      UNI-T UT61B series multimeter
 */
class ut61b : public ld_device {
public:
    ut61b() : ld_device("Uni-T,UT61B"), m_unit("?"), m_serial(nullptr) {};
    ut61b(std::unique_ptr<serial_iface> ser);
    ~ut61b() {};

    void connect(std::unique_ptr<ld_iface> comm) override;
    void disconnect() override { m_serial.reset(); }

    static constexpr unsigned BAUD = 2400;

    // Get current value from screen
    double get_value();

    // Get current unit from screen
    std::string get_unit() { return m_unit; }

private:
    std::string m_unit;
    std::unique_ptr<serial_iface> m_serial;

    // UT61b end of message character
    static constexpr const char* EOM = "\r\n";

    enum prefix_flag : uint8_t {
        micro   = (1 << 7),
        milli   = (1 << 6),
        kilo    = (1 << 5),
        mega    = (1 << 4),
        none    = (1 << 0)
    };

    enum unit_flag : uint8_t {
        volt    = (1 << 7),
        amp     = (1 << 6),
        ohm     = (1 << 5),
        hertz   = (1 << 3),
        degf    = (1 << 1),
        degc    = (1 << 0)
    };

};

}

#endif