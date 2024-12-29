#ifndef LD_ETH_TO_SER_HH
#define LD_ETH_TO_SER_HH

#include <labdev/serial_iface.hh>
#include <labdev/tcpip_iface.hh>

namespace labdev{

/** \brief Communication interface for an ethernet to serial converter. 
 *
 *  The Waveshare RS232-485-422 TO POE ETH (B) ethernet to serial converter
 *  provides a serial communication interface via TCP/IP.
 */
class eth_to_ser : public serial_iface {
public:
    eth_to_ser();
    /** \brief Open TCP/IP socket with specified baud rate and frame format.
     *  
     *  \param ip_addr IPv4 address of the converter (e.g. "192.168.2.100")
     *  \param port Port of the TCP/IP socket.
     *  \param baud Baud rate in bits per second.
     *  \param nbits Number of data bits (8/7/6/5) per the frame.
     *  \param par_ena En-/disable parity for the frame.
     *  \param par_even Use even/off parity for the frame.
     *  \param stop_bits Number of stop bits (1/2) per frame.
     */
    eth_to_ser(std::string ip_addr, unsigned port, unsigned baud = 9600, 
        unsigned nbits = 8, bool par_ena = false, bool par_even = false, 
        unsigned stop_bits = 1);
    ~eth_to_ser();

    void open() override;
    /// \copydoc eth_to_ser::eth_to_ser(std::string, unsigned, unsigned, unsigned, bool, bool, unsigned)
    void open(std::string ip_addr, unsigned port, unsigned baud = 9600, 
        unsigned nbits = 8, bool par_ena = false, bool par_even = false, 
        unsigned stop_bits = 1);
    void close() override;

    int write_raw(const uint8_t* data, size_t len) override;
    int read_raw(uint8_t* data, size_t max_len,
        unsigned timeout_ms = DFLT_TIMEOUT_MS) override;

    // Returns human readable info string
    std::string get_info() const noexcept override;

    /// Set ip address
    void set_ip(std::string ip_addr);
    /// Returns ip address
    std::string get_ip() const { return m_ip_addr; }

    /// Set port number
    void set_port(unsigned port);
    /// Returns port number
    unsigned get_port() const { return m_port; }

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
    tcpip_iface m_tcpip_cfg, m_tcpip_ser;
    std::string m_ip_addr;
    unsigned m_port;
    unsigned m_flc;
    bool m_update_settings;

    static constexpr unsigned HTTP_PORT = 80;

    std::string get_bdr(unsigned baud);
    unsigned get_dtb(unsigned nbits);
    unsigned get_prt(bool par_en, bool par_even);
    unsigned get_stb(unsigned stop_bits);
};

}

#endif