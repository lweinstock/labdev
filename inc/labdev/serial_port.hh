#ifndef LD_SERIAL_PORT_HH
#define LD_SERIAL_PORT_HH

#include <labdev/serial_iface.hh>
#include <termios.h>

namespace labdev{

/** \brief Communication interface based on the UNIX serial port
 *
 *  This class is a C++ wrapper for the C UNIX serial port (termios).
 *  It can be used for all serial devices (RS232, RS422, RS485, UART, 
 *  USB-UART bridged, etc.) that create a tty device file.
 */
class serial_port : public serial_iface {
public:
    serial_port();
    /** \brief Open device file with specified baud rate and frame format.
     *  
     *  \param path Path to device file (e.g. "/dev/ttyUSB0").
     *  \param baud Baud rate in bits per second.
     *  \param nbits Number of data bits (8/7/6/5) per the frame.
     *  \param par_ena En-/disable parity for the frame.
     *  \param par_even Use even/off parity for the frame.
     *  \param stop_bits Number of stop bits (1/2) per frame.
     */
    serial_port(std::string path, unsigned baud = 9600, unsigned nbits = 8,
        bool par_ena = false, bool par_even = false, unsigned stop_bits = 1);
    ~serial_port();

    void open() override;
    /// \copydoc serial_port::serial_port(std::string, unsigned baud, unsigned, bool, bool, unsigned)
    void open(std::string path, unsigned baud = 9600, unsigned nbits = 8,
        bool par_ena = false, bool par_even = false, unsigned stop_bits = 1);
    void close() override;

    int write_raw(const uint8_t* data, size_t len) override;
    int read_raw(uint8_t* data, size_t max_len,
        unsigned timeout_ms = DFLT_TIMEOUT_MS) override;

    // Returns human readable info string
    std::string get_info() const noexcept override;

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

    /// Check return value and throw corresponding exception
    void check_and_throw(int status, const std::string &msg) const;
    static speed_t check_baud(unsigned baud);
    static uint32_t check_bits(unsigned nbits);

};

}

#endif