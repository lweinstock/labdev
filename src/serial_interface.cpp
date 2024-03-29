#ifndef LD_SERIAL_INTERFACE_CPP
#define LD_SERIAL_INTERFACE_CPP

#include <labdev/serial_interface.hh>
#include <labdev/exceptions.hh>
#include "ld_debug.hh"

#include <fcntl.h>          // file control definitions
#include <unistd.h>         // open(), close(), read(), write(), ...
#include <errno.h>          // errno, strerr(), ...
#include <sys/ioctl.h>      // ioctl()
#include <sys/select.h>     // select()
#include <iostream>         // cout, cerr, ...

namespace labdev {

    serial_interface::serial_interface():
        m_path(""),
        m_fd(-1),
        m_term_settings(),
        m_timeout(),
        m_stop_bits(0),
        m_nbits(0x00),
        m_baud(0),
        m_connected(false),
        m_par_en(false),
        m_par_even(false),
        m_update_settings(true) {
        return;
    }

    serial_interface::serial_interface(const std::string &path,
            unsigned baud, unsigned nbits, bool par_en,
            bool par_even, unsigned stop_bits):
        serial_interface() {
        this->open(path, baud, nbits, par_en, par_even, stop_bits);
        return;
    }

    serial_interface::~serial_interface() {
        this->close();
        return;
    }

    void serial_interface::open(const std::string &path, unsigned baud,
            unsigned nbits, bool par_en, bool par_even,
            unsigned stop_bits) {
        debug_print("Opening device '%s'\n", path.c_str());
        m_fd = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        check_and_throw(m_fd, "Failed to open device " + path);
        m_path = path;

        int stat = tcgetattr(m_fd, &m_term_settings);
        check_and_throw(stat, "Failed to get termios attributes from " + path);

        // Control modes: no hw flow control
        //m_term_settings.c_cflag &= ~CRTSCTS;

        // Ignore modem control lines, enable receiver
        m_term_settings.c_cflag |= (CLOCAL | CREAD);

        // Input modes: no sw flow control, ignore break conditions
        m_term_settings.c_iflag &= ~(IXON | IXOFF | IXANY | IGNBRK);

        // Output modes: no processing
        m_term_settings.c_oflag &= ~OPOST;

        // Local modes: disable canonical mode, no echo, erasure
        m_term_settings.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHONL | ISIG);

        // Special characters: non-blocking I/O, no timeout
        m_term_settings.c_cc[VMIN] = 0;
        m_term_settings.c_cc[VTIME] = 0;

        this->disable_hw_flow_ctrl();
        this->set_baud(baud);
        this->set_nbits(nbits);
        this->set_parity(par_en, par_even);
        this->set_stop_bits(stop_bits);
        this->apply_settings();

        m_connected = true;
        return;
    }

    void serial_interface::close() {
        debug_print("Closing device '%s'\n", m_path.c_str());
        ::close(m_fd);
        m_connected = false;
        return;
    }

    int serial_interface::write_raw(const uint8_t* data, size_t len) {
        if (m_update_settings) this->apply_settings();

        size_t bytes_left = len;
        size_t bytes_written = 0;
        ssize_t nbytes = 0;

        while ( bytes_left > 0 ) {
            nbytes = ::write(m_fd, &data[bytes_written], bytes_left);
            check_and_throw(nbytes, "Failed to write to device");
            bytes_left -= nbytes;

            debug_print("Written %zu bytes: ", nbytes);
            #ifdef LD_DEBUG
            if (nbytes > 20) {
                for (int i = 0; i < 10; i++)
                    printf("0x%02X ", data[bytes_written + i]);
                printf("[...] ");
                for (int i = nbytes-10; i < nbytes; i++)
                    printf("0x%02X ", data[bytes_written + i]);
            } else {
                for (int i = 0; i < nbytes; i++)
                    printf("0x%02X ", data[bytes_written + i]);
            }
            printf("(%zi bytes left)\n", bytes_left);
            #endif

            bytes_written += nbytes;
        }

        return bytes_written;
    }

    int serial_interface::read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms) {
        if (m_update_settings) this->apply_settings();

        // Wait for I/O
        fd_set rfd_set;
        FD_ZERO(&rfd_set);
        FD_SET(m_fd, &rfd_set);
        m_timeout.tv_sec = timeout_ms / 1000;
        m_timeout.tv_usec = timeout_ms % 1000;

        // Block until data is available or timeout exceeded
        int stat = select(m_fd + 1, &rfd_set, NULL, NULL, &m_timeout);
        check_and_throw(stat, "No data available");
        if (stat ==  0)
            throw timeout("Read timeout occurred", errno);

        // Data is available!
        ssize_t nbytes;
        debug_print("%s", "Reading from device\n");
        nbytes = ::read(m_fd, data, max_len);
        check_and_throw(nbytes, "Failed to read from device");
        
        debug_print("Read %zi bytes: ", nbytes);
        #ifdef LD_DEBUG
        if (nbytes > 20) {
            for (int i = 0; i < 10; i++)
                printf("0x%02X ", data[i]);
            printf("[...] ");
            for (int i = nbytes-10; i < nbytes; i++)
                printf("0x%02X ", data[i]);
        } else {
            for (int i = 0; i < nbytes; i++)
                printf("0x%02X ", data[i]);
        }
        printf("\n");
        #endif

        return nbytes;
    }


    void serial_interface::set_baud(unsigned baud) {
        m_baud = this->check_baud(baud);
        debug_print("Setting baudrate to %i\n", baud);

        int stat = cfsetispeed(&m_term_settings, m_baud);
        check_and_throw(stat, "Failed to set in baudrate " +
            std::to_string(baud));
        stat = cfsetospeed(&m_term_settings, m_baud);
        check_and_throw(stat, "Failed to set out baudrate " +
            std::to_string(baud));

        m_update_settings = true;
        return;
    }

    void serial_interface::set_nbits(unsigned nbits) noexcept {
        m_nbits = this->check_bits(nbits);
        m_term_settings.c_cflag &= ~CSIZE;
        m_term_settings.c_cflag |= m_nbits;
        debug_print("Set number of bits to %i\n", nbits);
        m_update_settings = true;
        return;
    }

    void serial_interface::set_parity(bool en, bool even) noexcept {
        m_par_en = en;
        m_par_even = even;
        if (m_par_en) m_term_settings.c_cflag |= PARENB;
        else m_term_settings.c_cflag &= ~PARENB;

        if (m_par_even) m_term_settings.c_cflag &= ~PARODD;
        else m_term_settings.c_cflag |= PARODD;

        debug_print("%s parity with %s parity\n",
            (m_par_en) ? "enabled" : "disabled", (m_par_even) ? "even" : "odd");
        m_update_settings = true;
        return;
    }

    void serial_interface::set_stop_bits(unsigned stop_bits) noexcept {
        switch (stop_bits) {
        case 1: m_term_settings.c_cflag &= ~CSTOPB; break;
        case 2: m_term_settings.c_cflag |= CSTOPB;  break;
        default:
            fprintf(stderr, "%i stop bits are not supported\n",stop_bits );
            abort();
        }
        m_stop_bits = stop_bits;

        debug_print("set number of stop bits to %i\n", m_stop_bits);
        m_update_settings = true;
        return;
    }

    void serial_interface::apply_settings() {
        debug_print("%s", "Applying termio settings\n");
        int stat = tcsetattr(m_fd, TCSANOW, &m_term_settings);
        check_and_throw(stat, "Failed apply termios settings");
        tcflush(m_fd, TCIOFLUSH);

        m_update_settings = false;
        return;
    }

    void serial_interface::set_hw_flow_ctrl(bool ena) {
        if (ena) m_term_settings.c_cflag |= CRTSCTS;
        else m_term_settings.c_cflag &= ~CRTSCTS;
        debug_print("Hardware flow control %s\n", ena ? "enabled" : "disabled");
        m_update_settings = true;
        return;
    }

    void serial_interface::set_dtr() {
        int flag = TIOCM_DTR;
        int stat = ioctl(m_fd, TIOCMBIS, &flag);
        check_and_throw(stat, "Failed to set DTR");
        return;
    }

    void serial_interface::clear_dtr() {
        int flag = TIOCM_DTR;
        int stat = ioctl(m_fd, TIOCMBIC, &flag);
        check_and_throw(stat, "Failed to clear DTR");
        return;
    }

    void serial_interface::set_rts() {
        int flag = TIOCM_RTS;
        int stat = ioctl(m_fd, TIOCMBIS, &flag);
        check_and_throw(stat, "Failed to set RTS");
        return;
    }

    void serial_interface::clear_rts() {
        int flag = TIOCM_RTS;
        int stat = ioctl(m_fd, TIOCMBIC, &flag);
        check_and_throw(stat, "Failed to clear RTS");
        return;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    void serial_interface::check_and_throw(int status, const std::string &msg)
        const {
        if (status < 0) {
            int error = errno;
            char err_msg[256] = {'\0'};
            sprintf(err_msg, "%s (%s, %i)", msg.c_str(), strerror(error), error);
            debug_print("%s\n", err_msg);

            switch (error) {
            case EAGAIN:
                throw timeout(err_msg, error);
                break;

            case ENXIO:
                throw bad_connection(err_msg, error);
                break;

            default:
                throw bad_io(err_msg, error);
            }
        }
        return;
    }

    speed_t serial_interface::check_baud(unsigned baud) {
        speed_t good_baud;
        switch (baud) {
        case 0:         good_baud = B0;       break;
        case 50:        good_baud = B50;      break;
        case 75:        good_baud = B75;      break;
        case 110:       good_baud = B110;     break;
        case 134:       good_baud = B134;     break;
        case 150:       good_baud = B150;     break;
        case 200:       good_baud = B200;     break;
        case 300:       good_baud = B300;     break;
        case 600:       good_baud = B600;     break;
        case 1200:      good_baud = B1200;    break;
        case 1800:      good_baud = B1800;    break;
        case 2400:      good_baud = B2400;    break;
        case 4800:      good_baud = B4800;    break;
        case 9600:      good_baud = B9600;    break;
        case 19200:     good_baud = B19200;   break;
        case 38400:     good_baud = B38400;   break;
        case 57600:     good_baud = B57600;   break;
        case 115200:    good_baud = B115200;  break;
        case 230400:    good_baud = B230400;  break;
        default:
            fprintf(stderr, "Baudrate %i is not supported\n", baud);
            abort();
        }
        return good_baud;
    }

    uint32_t serial_interface::check_bits(unsigned nbits) {
        uint32_t good_nbits;
        switch (nbits) {
        case 5: good_nbits = CS5; break;
        case 6: good_nbits = CS6; break;
        case 7: good_nbits = CS7; break;
        case 8: good_nbits = CS8; break;
        default:
            fprintf(stderr, "%i-bit format is not supported\n", nbits);
            abort();
        }
        return good_nbits;
    }

}

#endif
