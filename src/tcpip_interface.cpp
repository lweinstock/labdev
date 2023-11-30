#include <labdev/tcpip_interface.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <errno.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sstream>
#include <iostream>

using namespace std;

namespace labdev {

tcpip_interface::tcpip_interface() 
    : ld_interface(), m_socket_fd(-1), m_instr_addr(), m_timeout(), 
      m_ip_addr("127.0.0.1"), m_port(0)
{
    return;
}

tcpip_interface::tcpip_interface(std::string ip_addr, unsigned port) 
    : tcpip_interface() 
{
    this->open(ip_addr, port);
    return;
}

tcpip_interface::~tcpip_interface()
{
    this->close();
    return;
}

void tcpip_interface::open()
{
    this->open(m_ip_addr, m_port);
    return;
}

void tcpip_interface::open(std::string ip_addr, unsigned port)
{
    // Create TCP/IP socket
    m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    check_and_throw(m_socket_fd, "Could not open socket.");

    set_buffer_size(s_dflt_buf_size);
    set_timeout(0); // never time out -> we will use select() for timeout

    // Set up instrument ip address
    m_instr_addr.sin_family = AF_INET;
    m_instr_addr.sin_port = htons(port);
    int stat = inet_aton(ip_addr.c_str(), &m_instr_addr.sin_addr);
    stringstream err_msg("");
    err_msg << "Address " << ip_addr << ":" << port << " is not supported.";
    check_and_throw(stat, err_msg.str());

    // Connect to instrument...
    stat = connect(m_socket_fd, (struct sockaddr *)&m_instr_addr,
        sizeof(m_instr_addr));
    err_msg.str("");
    err_msg << "Failed to connect to " << ip_addr << ":" << port << ".";
    check_and_throw(stat, err_msg.str());
    debug_print("connected to IP address = %s:%i\n",
        inet_ntoa(m_instr_addr.sin_addr), m_instr_addr.sin_port);
    m_ip_addr = ip_addr;
    m_port = port;

    m_good = true;
    return;
}

void tcpip_interface::close()
{
    debug_print("Closing connection to %s:%i\n", m_ip_addr.c_str(), m_port);
    shutdown(m_socket_fd, SHUT_WR);
    while ( !this->read().empty() ) { usleep(10e3); }   // Read remaining messages
    shutdown(m_socket_fd, SHUT_RD);
    int stat = ::close(m_socket_fd);
    check_and_throw(stat, "Failed to close connection to " + m_ip_addr + ":" 
        + to_string(m_port));

    m_good = false;
    return;
}

int tcpip_interface::write_raw(const uint8_t* data, size_t len) 
{
    size_t bytes_left = len;
    size_t bytes_written = 0;
    ssize_t nbytes = 0;

    while ( bytes_left > 0 ) {
        nbytes = send(m_socket_fd, &data[bytes_written], bytes_left, 0);
        check_and_throw(nbytes, "Failed to write to device");
        bytes_left -= nbytes;

        debug_print_byte_data(data, nbytes, "Written %zu bytes: ", nbytes);

        bytes_written += nbytes;
    }

    return bytes_written;
}

int tcpip_interface::read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms)
{
    // Wait for I/O
    fd_set rfd_set;
    FD_ZERO(&rfd_set);
    FD_SET(m_socket_fd, &rfd_set);
    m_timeout.tv_sec = timeout_ms / 1000;
    m_timeout.tv_usec = timeout_ms % 1000;

    // Block until data is available or timeout exceeded
    int stat = select(m_socket_fd + 1, &rfd_set, NULL, NULL, &m_timeout);
    check_and_throw(stat, "No data available");
    if (stat ==  0)
        throw timeout("Read timeout occurred", errno);

    ssize_t nbytes = recv(m_socket_fd, data, max_len, 0);
    check_and_throw(nbytes, "Failed to read from device");
    debug_print_byte_data(data, nbytes, "Read %zu bytes: ", nbytes);

    return nbytes;
}

void tcpip_interface::set_buffer_size(size_t size) 
{
    // Receive buffer
    int stat = setsockopt(m_socket_fd, SOL_SOCKET, SO_RCVBUF, (char*)&size,
        sizeof(int));
    stringstream err_msg("");
    err_msg << "Set receive buffer length to " << size << " failed.";
    check_and_throw(stat, err_msg.str());

    // Send buffer
    stat = setsockopt(m_socket_fd, SOL_SOCKET, SO_SNDBUF, (char *)&size,
        sizeof(int));
    err_msg.str("");
    err_msg << "Set send buffer length to " << size << "failed.";
    check_and_throw(stat, err_msg.str());

    return;
}

void tcpip_interface::set_timeout(unsigned timeout_ms) 
{
    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = 1000 * (timeout_ms % 1000);

    int stat = setsockopt(m_socket_fd, SOL_SOCKET, SO_RCVTIMEO,
        (char*)&timeout, sizeof(timeout));
    stringstream err_msg("");
    err_msg << "Set receive timeout to " << timeout_ms << "ms failed.";
    check_and_throw(stat, err_msg.str());

    stat = setsockopt(m_socket_fd, SOL_SOCKET, SO_SNDTIMEO,
        (char *)&timeout, sizeof(timeout));
    err_msg.str("");
    err_msg << "Set send timeout to " << timeout_ms << "ms failed.";
    check_and_throw(stat, err_msg.str());

    return;
}

string tcpip_interface::get_info() const 
{
    // Format example: tcpip;192.168.0.1;10001
    string ret("tcpip;" + m_ip_addr + ";" + to_string(m_port));
    return ret;
}

/*
 *      P R I V A T E   M E T H O D S
 */


void tcpip_interface::check_and_throw(int status, const string &msg) const 
{
    if (status < 0) {
        int error = errno;
        stringstream err_msg;
        err_msg << msg << " (" << strerror(error) << ", " << error << ")";
        debug_print("%s\n", err_msg.str().c_str());

        switch (error) {
        case EAGAIN:
            throw timeout(err_msg.str().c_str(), error);
            break;

        case ENXIO:
        case ECONNREFUSED:
            throw bad_connection(err_msg.str().c_str(), error);
            break;

        default:
            throw bad_io(err_msg.str().c_str(), error);
        }
    }
    return;
}

}