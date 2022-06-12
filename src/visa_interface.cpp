#include <labdev/visa_interface.hh>
#include "ld_debug.hh"

#include <unistd.h>

namespace labdev {

    ViSession visa_interface::s_default_rm;
    int visa_interface::s_interface_ctr = 0;

    visa_interface::visa_interface():
        m_instr(0),
        m_visa_id(),
        m_connected(false),
        m_timeout(DFLT_TIMEOUT_MS) {
        ViStatus stat;
        if (s_interface_ctr == 0) {
            stat = viOpenDefaultRM(&s_default_rm);
            check_and_throw(stat, "Could not open default resource manager");
            debug_print("%s", "Created default resource manager.\n");
        }
        s_interface_ctr++;
        debug_print("interface constructed, %i objects remain.\n", s_interface_ctr);
        return;
    }

    visa_interface::visa_interface(std::string visa_id) : visa_interface() {
        this->open(visa_id);
        return;
    }

    visa_interface::~visa_interface() {
        ViStatus stat;
        stat = viClose(m_instr);
        check_and_throw(stat, "Could not close instrument");
        s_interface_ctr--;
        debug_print("interface destroyed, %i objects remain.\n", s_interface_ctr);
        if (s_interface_ctr == 0) {
            stat = viClose(s_default_rm);
            check_and_throw(stat, "Could not close default resource manager");
            debug_print("%s\n", "Destroyed default resource manager");
        }
        return;
    }

    void visa_interface::open(const std::string &visa_id) {
        // Throw exception if a device has already been opened
        if (m_instr)
            throw bad_io("Interface is already opened");
        ViStatus stat;
        debug_print("Opening instrument '%s'\n", visa_id.c_str());
        stat = viOpen(s_default_rm, visa_id.c_str(), VI_NULL, VI_NULL, &m_instr);
        check_and_throw(stat, "Could not open instrument '" + visa_id + "'");

        m_visa_id = visa_id;
        m_connected = true;
        return;
    }

    void visa_interface::close() {
        // Do nothing if no device is opened
        if (!m_instr)
            return;
        ViStatus stat;
        debug_print("Closing instrument '%s'\n", m_visa_id.c_str());
        stat = viClose(m_instr);
        check_and_throw(stat, "Failed to close instrument '" + m_visa_id + "'");

        m_connected = false;
        return;
    }

    std::vector<std::string> visa_interface::find_resources(std::string regex) {
        ViFindList rlist;
        unsigned nrsrc;
        char rname[VI_FIND_BUFLEN];
        std::vector<std::string> ret;

        // Find all resources matching the regular expression
        ViStatus stat = viFindRsrc(s_default_rm, regex.c_str(), &rlist, &nrsrc, rname);
        check_and_throw(stat, "Resource '" + regex + "' not found");
        ret.push_back(std::string(rname));

        for (int i = 1; i < nrsrc; i++) {
            stat = viFindNext(rlist, rname);
            check_and_throw(stat, "Next resource in list '" + regex + "' not found");
            ret.push_back(std::string(rname));
            debug_print("Found resource '%s'\n", rname);
        }
        return ret;
    }

    int visa_interface::write_raw(const uint8_t* data, size_t len) {
        size_t bytes_left =len;
        size_t bytes_written = 0;
        ssize_t nbytes = 0;
        ViStatus stat;

        while ( bytes_left > 0 ) {
            stat = viWrite(m_instr, (ViBuf)&data[bytes_written], bytes_left, (ViUInt32*)&nbytes);
            check_and_throw(stat, "Failed to write to device");
            if (nbytes > 0) {
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
        }

        return bytes_written;
    }

    int visa_interface::read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms) {
        ViStatus stat;
        if (timeout_ms != m_timeout) {
            stat = viSetAttribute(m_instr, VI_ATTR_TMO_VALUE, timeout_ms);
            check_and_throw(stat, "viSetAttribute for VI_ATTR_TMO_VALUE failed");
            m_timeout = timeout_ms;
        }

        uint8_t rbuf[s_dflt_buf_size] = {0};
        ssize_t nbytes;
        ssize_t bytes_received = 0;
        stat = VI_SUCCESS_MAX_CNT;

        debug_print("%s", "Reading from device\n");
        while (stat == VI_SUCCESS_MAX_CNT) {
            stat = viRead(m_instr, (ViBuf)rbuf, max_len, (ViUInt32*)&nbytes);
            check_and_throw(stat, "failed to read data from device");
            if (nbytes > 0) {
                // Check for buffer overflow
                if (bytes_received + nbytes > s_dflt_buf_size)
                    throw bad_protocol("Read buffer too small", s_dflt_buf_size);

                debug_print("Read %zi bytes: ", nbytes);
                #ifdef LD_DEBUG
                if (nbytes > 20) {
                    for (int i = 0; i < 10; i++)
                        printf("0x%02X ", rbuf[i]);
                    printf("[...] ");
                    for (int i = nbytes-10; i < nbytes; i++)
                        printf("0x%02X ", rbuf[i]);
                } else {
                    for (int i = 0; i < nbytes; i++)
                        printf("0x%02X ", rbuf[i]);
                }
                printf("\n");
                #endif

                // Append read buffer to output data array
                for (ssize_t ibyte = 0; ibyte < nbytes; ibyte++)
                    data[bytes_received + ibyte] = rbuf[ibyte];

                bytes_received += nbytes;
            }
        }
        return bytes_received;
    }

    void visa_interface::flush_buffer(uint16_t flag) {
        ViStatus stat = viFlush(m_instr, flag);
        check_and_throw(stat, "viFlush failed");
        debug_print("%s", "Flushing I/O buffers.\n");
        return;
    };

    void visa_interface::clear_device() {
        ViStatus stat = viClear(m_instr);
        check_and_throw(stat, "viClear failed");
        debug_print("%s", "Clearing device.\n");
        return;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    void visa_interface::init() {
        ViStatus stat;
        if (s_interface_ctr == 0) {
            stat = viOpenDefaultRM(&s_default_rm);
            check_and_throw(stat, "Could not open default resource manager");
            debug_print("%s", "Created default resource manager.\n");
        }
        s_interface_ctr++;
        debug_print("interface constructed, %i objects remain.\n", s_interface_ctr);
        return;
    }

    void visa_interface::check_and_throw(ViStatus status, const std::string &msg)
        const {
        if (status < VI_SUCCESS) {
            char err_msg[256] = {'\0'}, vi_strerror[256] = {'\0'};
            // Get human readable error message
            viStatusDesc(m_instr, status, vi_strerror);
            sprintf(err_msg, "%s (%s, %i)", msg.c_str(), vi_strerror, status);

            switch (status) {
            case VI_ERROR_TMO:
                throw timeout(err_msg, status);
                break;

            case VI_ERROR_OUTP_PROT_VIOL:   // Protocol violation
            case VI_ERROR_BERR:             // Bus error
                throw bad_connection(err_msg, status);
                break;

            case VI_ERROR_IO:
            default:
                throw bad_io(err_msg, status);
            }
        }
    }

}
