#ifndef LD_SERIAL_INTERFACE_HH
#define LD_SERIAL_INTERFACE_HH

#include <labdev/ld_interface.hh>
#include <termios.h>

namespace labdev{

class serial_interface : public ld_interface {
public:
    serial_interface();
    serial_interface(std::string path, unsigned baud = 9600, unsigned nbits = 8,
        bool par_ena = false, bool par_even = false, unsigned stop_bits = 1);
    ~serial_interface();

    virtual void open() override;
    void open(std::string path, unsigned baud = 9600, unsigned nbits = 8,
        bool par_ena = false, bool par_even = false, unsigned stop_bits = 1);
    virtual void close() override;

    virtual int write_raw(const uint8_t* data, size_t len) override;
    virtual int read_raw(uint8_t* data, size_t max_len,
        unsigned timeout_ms = s_dflt_timeout_ms) override;

    // Returns human readable info string
    virtual std::string get_info() const noexcept override;

    // Returns path to device file
    std::string get_path() const { return m_path; }

    // Set baud rate for serial interface
    virtual void set_baud(unsigned baud); 
    unsigned get_baud() const { return m_baud; }

    // Set number of data bits per packet
    virtual void set_nbits(unsigned nbits);
    unsigned get_nbits() const { return m_nbits; }

    // Send 1 or 2 stop bits
    virtual void set_stop_bits(unsigned stop_bits);
    unsigned get_stop_bits() const { return m_sbits; }

    // Enable and set parity
    virtual void set_parity(bool en = true, bool even = true);
    bool get_parity() const { return m_par_en; }
    bool parity_even() const { return m_par_even; }

    // Apply changed settings
    virtual void apply_settings();

    // En-/disable hardware flow control
    virtual void enable_rts_cts();
    virtual void enable_dtr_dsr();
    virtual void disable_hw_flow_ctrl();

    // Data Terminal Ready (DTR) for manual flow control
    virtual void set_dtr();
    virtual void clear_dtr();

    // Request To Send (RTS) for manual flow control
    virtual void set_rts();
    virtual void clear_rts();

    Interface_type type() const noexcept override { return SERIAL; }

protected:
    std::string m_path;
    unsigned m_baud, m_nbits, m_sbits ;
    bool m_par_en, m_par_even, m_update_settings;

    int m_fd;
    struct termios m_term_settings;
    struct timeval m_timeout;

    void check_and_throw(int status, const std::string &msg) const;
    static speed_t check_baud(unsigned baud);
    static uint32_t check_bits(unsigned nbits);
};

}

#endif
