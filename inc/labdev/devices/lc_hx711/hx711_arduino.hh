#ifndef HX711_ARDUINO_H
#define HX711_ARDUINO_H

#include <labdev/serial_iface.hh>
#include <labdev/devices/ld_device.hh>
#include <memory>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <labdev/exceptions.hh>

namespace labdev {

/*
 *      Arduino Mega + HX711 readout 
 *
 * Protocol (from firmware):
 *   PC sends:  'g'
 *   MCU replies: "HX711,RAW=<long>,AVG10=<long>\r\n"
 */
class hx711_arduino : public ld_device {
public:
    hx711_arduino()  : ld_device("HX711,Arduino"), m_serial(nullptr) {}

    hx711_arduino(std::unique_ptr<serial_iface> ser)
    : hx711_arduino() { connect(std::move(ser)); }

    ~hx711_arduino() {}

    void connect(std::unique_ptr<ld_iface> comm) override;
    void disconnect() override { m_serial.reset(); }

    /// Baud rate used by Arduino sketch
    static constexpr unsigned BAUD = 115200;

    /// Trigger one measurement and cache results; returns true on success
    int read_lc();

    /// Getters (last cached values; call update() to refresh)


private:
    std::unique_ptr<serial_iface> m_serial;

    // ASCII protocol bits
    static constexpr const char* EOM = "\r\n";
    static constexpr char CMD_GET   = 'g';

    
};

} // namespace labdev

#endif // HX711_ARDUINO_H