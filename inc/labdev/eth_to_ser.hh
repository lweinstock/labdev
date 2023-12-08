#ifndef LD_ETH_TO_SER_HH
#define LD_ETH_TO_SER_HH

#include <labdev/serial_interface.hh>
#include <labdev/tcpip_interface.hh>

namespace labdev{

/*
 *      Serial interface provided by the Waveshare RS232-485-422 TO POE ETH (B)
 *      ethernet to serial converter
 */

class eth_to_ser : public serial_interface {
public:
    eth_to_ser();
    eth_to_ser(std::string ip_addr, unsigned port, unsigned baud = 9600, 
        unsigned nbits = 8, bool par_ena = false, bool par_even = false, 
        unsigned stop_bits = 1);
    ~eth_to_ser();

    void open() override;
    void open(std::string ip_addr, unsigned port, unsigned baud = 9600, 
        unsigned nbits = 8, bool par_ena = false, bool par_even = false, 
        unsigned stop_bits = 1);
    void close() override;

    int write_raw(const uint8_t* data, size_t len) override;
    int read_raw(uint8_t* data, size_t max_len,
        unsigned timeout_ms = s_dflt_timeout_ms) override;

    // Returns human readable info string
    std::string get_info() const override;

    // Set/get ip address
    void set_ip(std::string ip_addr);
    std::string get_ip() const { return m_ip_addr; }

    // Set/get port number
    void set_port(unsigned port);
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
    tcpip_interface m_tcpip_cfg, m_tcpip_ser;
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
