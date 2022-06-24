#ifndef LD_TCPIP_INTERFACE_HH
#define LD_TCPIP_INTERFACE_HH

#include <labdev/interface.hh>

#include <sys/socket.h>
#include <arpa/inet.h>

namespace labdev {
    class tcpip_interface : public interface {
    public:
        tcpip_interface();
        tcpip_interface(const std::string& ip_addr, unsigned port);
        virtual ~tcpip_interface();

        // Open TCP/IP socket with given IP and port
        void open(const std::string& ip_addr, unsigned port);

        // Close TCP/IP socket
        void close() override;

        int write_raw(const uint8_t* data, size_t len) override;
        int read_raw(uint8_t* data, size_t max_len, 
            unsigned timeout_ms = s_dflt_timeout_ms) override;

        // Set read/write buffer size
        void set_buffer_size(size_t buf_size);

        // Set read/write timeout in milliseconds
        void set_timeout(unsigned timeout_ms);

        // Returns true if socket is ready for IO
        bool connected() const override { return m_connected; }

        // Returns interface type
        Interface_type type() const override { return tcpip; }

    private:
        int m_socket_fd;
        struct sockaddr_in m_instr_addr;
        struct timeval m_timeout;
        bool m_connected;

        void check_and_throw(int stat, const std::string& msg) const;
    };
}

#endif
