#ifndef LD_INTERFACE_HH
#define LD_INTERFACE_HH

#include <cstring>
#include <string>
#include <vector>
#include <cstdint>

namespace labdev{

/*! \brief Enum for different interface types.
 *
 *  Return type of ld_iface::type(). 
 *  Can be used to identify downcast interfaces.   
 */

enum Interface_type {NONE, SERIAL, TCPIP, USB, USBTMC, VISA, MODBUS_TCP, MODBUS_RTU};

/*! \brief Abstract base class for communication interfaces.
 *
 *  An interface provides a way to send data to and read data from an 
 *  instrument. All interfaces must implement a C-style read_raw(...) and 
 *  write_raw(...) method. Copy ctor and assignment operator are removed since
 *  an interface represents a physical connection to an instrument or device and
 *  should therefore be unique.
 */

class ld_iface {
public:
    //! Default constructor.
    ld_iface() : m_good(false) {};
    //! Default destructor.
    virtual ~ld_iface() {};

    //! No copy constructor; interfaces are unique physical entities
    ld_iface(const ld_iface&) = delete;
    //! No assignment operator; interfaces are unique physical entities
    ld_iface& operator=(const ld_iface&) = delete;

    //! 1MB default buffer size
    static constexpr size_t DFLT_BUF_SIZE = 1024*1024;  
    //! 2s default timeout
    static constexpr unsigned DFLT_TIMEOUT_MS = 2000;

    /*! \brief C-style raw byte write.
     *  \param [in] data Output byte array.
     *  \param [in] len Length of byte array.
     *  \return Number of successfully written bytes.
     */
    virtual int write_raw(const uint8_t* data, size_t len) = 0;

    /*! \brief C++-style byte write.
     *  \param [in] data Output byte vector.
     */
    void write_byte(const std::vector<uint8_t> data);

    /*! \brief C++-style string write.
     *  \param [in] msg Output string.
     */
    void write(const std::string& msg);

    /*! \brief C-style raw byte read.
     *  \param [out] data Input byte array.
     *  \param [in] max_len Maximum length of byte array.
     *  \return Number of successfully read bytes.
     */
    virtual int read_raw(uint8_t* data, size_t max_len, 
        unsigned timeout_ms = DFLT_TIMEOUT_MS) = 0;

    /*! \brief C++-style byte read.
     *  \param [in] timeout_ms Read timeout in milli seconds.
     *  \return Vector with filled bytes.
     */
    std::vector<uint8_t> read_byte(unsigned timeout_ms = DFLT_TIMEOUT_MS);

    /*! \brief C++-style string read.
     *  \param [in] timeout_ms Read timeout in milli seconds.
     *  \return String composed of read bytes.
     */
    std::string read(unsigned timeout_ms = DFLT_TIMEOUT_MS);

    /*! \brief Read until specified delimiter is found in the received message.
     *  \param [in] delim Stop delimiter.
     *  \param [out] pos Position of the delimiter in string.
     *  \param [in] timeout_ms Read timeout in milli seconds.
     *  \return String composed of read bytes.
     */
    std::string read_until(const std::string& delim, size_t& pos, 
        unsigned timeout_ms = DFLT_TIMEOUT_MS);
    std::string read_until(const std::string& delim, 
        unsigned timeout_ms = DFLT_TIMEOUT_MS);

    /*! \brief C++-style string write followed by a read.
     *  \param [in] msg Query message.
     *  \param [in] timeout_ms Read timeout in milli seconds.
     *  \return Response string.
     */
    std::string query(const std::string& msg, 
        unsigned timeout_ms = DFLT_TIMEOUT_MS);

    //! Open interface with stored settings
    virtual void open() = 0;

    //! Close interface
    virtual void close() = 0;

    //! Returns true if interface is usable.
    virtual bool good() const { return m_good; }

    //! Returns human readable string with information.
    virtual std::string get_info() const noexcept = 0;

    //! Returns interface type; can be used to break abstraction.
    virtual Interface_type type() const noexcept = 0;

protected:
    //! Can be set by derived classes if the interface is valid and usable
    bool m_good;
};

}

#endif
