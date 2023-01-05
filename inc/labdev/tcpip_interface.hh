#ifndef LD_TCPIP_INTERFACE_HH
#define LD_TCPIP_INTERFACE_HH

#include <labdev/interface.hh>

#include <sys/socket.h>
#include <arpa/inet.h>

namespace labdev {

    // brief struct for tcpip configuration
    struct ip_address {
        ip_address() : ip("127.0.0.1"), port(0) {};
        ip_address(std::string ip, unsigned port) : ip(ip), port(port) {};
        std::string ip;
        unsigned port;
    };

    class tcpip_interface : public interface {
    public:
        tcpip_interface();
        tcpip_interface(const std::string& ip_addr, unsigned port);
        tcpip_interface(ip_address &ip_addr) : 
            tcpip_interface(ip_addr.ip, ip_addr.port) {};
        virtual ~tcpip_interface();

        // Open TCP/IP socket with given IP and port
        void open(const std::string& ip_addr, unsigned port);
        // Open TCP/IP socket with stored information
        void open() override;

        // Close TCP/IP socket
        void close() override;

        int write_raw(const uint8_t* data, size_t len) override;
        int read_raw(uint8_t* data, size_t max_len, 
            unsigned timeout_ms = s_dflt_timeout_ms) override;

        // Set/get ip address
        void set_ip(std::string ip_addr) noexcept { m_ip_addr = ip_addr; }
        std::string get_ip() const { return m_ip_addr; }

        // Set/get port number
        void set_port(unsigned port) noexcept { m_port =  port; }
        unsigned get_port() const { return m_port; }

        // Set read/write buffer size
        void set_buffer_size(size_t buf_size);

        // Set read/write timeout in milliseconds
        void set_timeout(unsigned timeout_ms);

        // Returns true if socket is ready for IO
        bool good() const override { return m_connected; }

        // Returns interface type
        Interface_type type() const override { return tcpip; }

        // Returns human readable info string
        std::string get_info() const override;

    private:
        int m_socket_fd;
        struct sockaddr_in m_instr_addr;
        struct timeval m_timeout;
        bool m_connected;
        std::string m_ip_addr;
        unsigned m_port;

        void check_and_throw(int stat, const std::string& msg) const;
    };
}

#endif
