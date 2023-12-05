#ifndef LD_INTERFACE_HH
#define LD_INTERFACE_HH

#include <cstring>
#include <string>
#include <vector>

namespace labdev{

/*
 *  Interface types
 */

enum Interface_type {none, rs232, tcpip, usb, usbtmc, visa};

/*
 *  Abstract base class for all interfaces
 */

class ld_interface {
public:
    ld_interface() : m_good(false) {};
    virtual ~ld_interface() {};

    // No copy ctor or assignment; interfaces are unique physical entities
    ld_interface(const ld_interface&) = delete;
    ld_interface& operator=(const ld_interface&) = delete;

    /*
     *      Default values
     */

    // 1mb default buffer size
    static constexpr size_t s_dflt_buf_size = 1024*1024;
    // 2s default timeout
    static constexpr unsigned s_dflt_timeout_ms = 2000;

    /*
     *      Basic read and write methods
     */

    // C-style raw byte write
    virtual int write_raw(const uint8_t* data, size_t len) = 0;
    // C++-style byte write
    void write_byte(const std::vector<uint8_t> data);
    // C++-style string write
    void write(const std::string& msg);

    // C-style raw byte read
    virtual int read_raw(uint8_t* data, size_t max_len, 
        unsigned timeout_ms = s_dflt_timeout_ms) = 0;
    // C++-style byte read
    std::vector<uint8_t> read_byte(unsigned timeout_ms = s_dflt_timeout_ms);
    // C++-style string read
    std::string read(unsigned timeout_ms = s_dflt_timeout_ms);

    // Read until specified delimiter is found in the received message
    std::string read_until(const std::string& delim, size_t& pos, 
        unsigned timeout_ms = s_dflt_timeout_ms);
    std::string read_until(const std::string& delim, 
        unsigned timeout_ms = s_dflt_timeout_ms);

    // C++-style string write followed by a read
    std::string query(const std::string& msg, 
        unsigned timeout_ms = s_dflt_timeout_ms);

    /*
     *      Utility methods
     */

    // Open interface with stored settings
    virtual void open() = 0;

    // Close interface
    virtual void close() = 0;

    // Returns true if interface is usable
    virtual bool good() const { return m_good; }

    // Returns human readable string with information
    virtual std::string get_info() const = 0;

    // Returns interface type; can be used to break abstraction
    virtual Interface_type type() const = 0;

protected:
    // Can be set by derived classes if the interface is valid and usable
    bool m_good;
};

}

#endif
