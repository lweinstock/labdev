#ifndef LD_RS232_INTERFACE_HH
#define LD_RS232_INTERFACE_HH

#include <labdev/ld_interface.hh>
#include <termios.h>

namespace labdev{

class serial_interface : public ld_interface {
public:
    serial_interface();
    serial_interface(std::string path, unsigned baud = 9600, unsigned nbits = 8,
        bool par_ena = false, bool par_even = false, unsigned stop_bits = 1);
    ~serial_interface();

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
    void set_baud(unsigned baud);
    unsigned get_baud() const { return m_baud; }

    // Set number of data bits per packet
    void set_nbits(unsigned nbits) noexcept;
    unsigned get_nbits() const { return m_nbits; }

    // Send 1 or 2 stop bits
    void set_stop_bits(unsigned stop_bits) noexcept;
    unsigned get_stop_bits() const { return m_stop_bits; }

    // Apply termios settings
    void apply_settings();

    // Enable and set parity
    void set_parity(bool en = true, bool even = true) noexcept;
    bool get_parity() const { return m_par_en; }
    bool parity_even() const { return m_par_even; }

    // En-/disable hardware flow control
    void set_hw_flow_ctrl(bool ena);
    void enable_hw_flow_ctrl() { this->set_hw_flow_ctrl(true); }
    void disable_hw_flow_ctrl() { this->set_hw_flow_ctrl(false); }

    // Data Transfer Ready (DTR) for manual flow control
    void set_dtr();
    void clear_dtr();

    // Request To Send (RTS) for manual flow control
    void set_rts();
    void clear_rts();

    Interface_type type() const override { return serial; }

private:
    std::string m_path;
    int m_fd;
    struct termios m_term_settings;
    struct timeval m_timeout;
    unsigned m_stop_bits, m_nbits, m_baud;
    bool m_par_en, m_par_even, m_update_settings;

    void check_and_throw(int status, const std::string &msg) const;
    static speed_t check_baud(unsigned baud);
    static uint32_t check_bits(unsigned nbits);

};

}

#endif
