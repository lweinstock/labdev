#ifndef LD_SERIAL_PORT_HH
#define LD_SERIAL_PORT_HH

#include <labdev/serial_interface.hh>
#include <termios.h>

namespace labdev{

/*
 *      RS232 interface based on UNIX serial port
 */

class serial_port : public serial_interface {
public:
    serial_port();
    serial_port(std::string path, unsigned baud = 9600, unsigned nbits = 8,
        bool par_ena = false, bool par_even = false, unsigned stop_bits = 1);
    ~serial_port();

    void open() override;
    void open(std::string path, unsigned baud = 9600, unsigned nbits = 8,
        bool par_ena = false, bool par_even = false, unsigned stop_bits = 1);
    void close() override;

    int write_raw(const uint8_t* data, size_t len) override;
    int read_raw(uint8_t* data, size_t max_len,
        unsigned timeout_ms = s_dflt_timeout_ms) override;

    // Returns human readable info string
    std::string get_info() const override;

    // Returns path to device file
    std::string get_path() { return m_path; }

    // Set baud rate for serial interface
    void set_baud(unsigned baud) override;

    // Set number of data bits per packet
    void set_nbits(unsigned nbits) override;

    // Send 1 or 2 stop bits
    void set_stop_bits(unsigned stop_bits) override;

    // Enable and set parity
    void set_parity(bool en = true, bool even = true) override;

    // Apply termios settings
    void apply_settings() override;

    // Enable and set parity
    void enable_rts_cts() override;
    void enable_dtr_dsr() override;
    void disable_hw_flow_ctrl() override;

    // Data Terminal Ready (DTR) for manual flow control
    void set_dtr() override;
    void clear_dtr() override;

    // Request To Send (RTS) for manual flow control
    void set_rts() override;
    void clear_rts() override;

private:
    std::string m_path;
    int m_fd;
    struct termios m_term_settings;
    struct timeval m_timeout;
    bool m_update_settings;

    void check_and_throw(int status, const std::string &msg) const;
    static speed_t check_baud(unsigned baud);
    static uint32_t check_bits(unsigned nbits);

};

}

#endif
