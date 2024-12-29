#ifndef LD_TCPIP_INTERFACE_HH
#define LD_TCPIP_INTERFACE_HH

#include <labdev/ld_iface.hh>

#include <sys/socket.h>
#include <arpa/inet.h>

namespace labdev {

/** \brief Communication interface based on UNIX TCP sockets
 *
 *  This class is a C++ wrapper for the C UNIX socket api (sys/socket.h).
 */
class tcpip_iface : public ld_iface {
public:
    tcpip_iface();
    /** \brief Open socket at specified ip address and port.
     *  
     *  \param ip_addr IPv4 address (e.g. "192.168.2.200").
     *  \param port Port of socket to connect to.
     */
    tcpip_iface(std::string ip_addr, unsigned port);
    virtual ~tcpip_iface();

    void open() override;
    /// \copydoc labdev::tcpip_iface(std::string, unsigned)
    void open(std::string ip_addr, unsigned port);
    void close() override;

    int write_raw(const uint8_t* data, size_t len) override;
    int read_raw(uint8_t* data, size_t max_len, 
        unsigned timeout_ms = DFLT_TIMEOUT_MS) override;

    /// Set ip address
    void set_ip(std::string ip_addr) noexcept { m_ip_addr = ip_addr; }
    /// Returns ip address
    std::string get_ip() const { return m_ip_addr; }

    /// Set port number
    void set_port(unsigned port) noexcept { m_port =  port; }
    /// Returns port number
    unsigned get_port() const { return m_port; }

    /// Set read/write buffer size
    void set_buffer_size(size_t buf_size);

    /// Set read/write timeout in milliseconds
    void set_timeout(unsigned timeout_ms);

    // Returns interface type
    Interface_type type() const noexcept override { return TCPIP; }

    // Returns human readable info string
    std::string get_info() const noexcept override;

private:
    int m_socket_fd;
    struct sockaddr_in m_instr_addr;
    struct timeval m_timeout;
    std::string m_ip_addr;
    unsigned m_port;

    void check_and_throw(int stat, const std::string& msg) const;
};

}

#endif
