#ifndef LD_SCPI_DEVICE_HH
#define LD_SCPI_DEVICE_HH

#include <string>
#include <cstdint>

#include <labdev/devices/ld_device.hh>

namespace labdev 
{

/** \brief Implementation of Standard Commands for Programmable Instruments (SCPI)
 *
 *  Supports common commands defined in IEEE 488.2
 */
class scpi_device : public ld_device
{
public:
    virtual ~scpi_device() {};

    // No copy constructor or assignment, default move constructor
    scpi_device(const scpi_device&) = delete;
    scpi_device& operator=(const scpi_device&) = delete;

    /// CLear Status
    void CLS() { m_comm->write("*CLS\n"); }

    /// Event Status Enable command
    void set_ESE(uint8_t event_status);

    /// Event Status Enable query
    uint8_t get_ESE();

    /// Event Status Read
    uint8_t get_ESR();

    /// Standard Event Status Register (SESR) definitions
    enum SESR : uint8_t {
        OPC = (1 << 0),     //> Operation Complete
        RQC = (1 << 1),     //> Request Control
        QYE = (1 << 2),     //> Query Error
        DDE = (1 << 3),     //> Device Dependant Error
        EXE = (1 << 4),     //> Execution Error
        CME = (1 << 5),     //> Command Error
        URQ = (1 << 6),     //> User Request
        PON = (1 << 7)      //> Power On
    };

    /// IDeNtification query
    std::string get_IDN() { return m_comm->query("*IDN?\n"); }

    /// OPeration Complete command
    void set_OPC() { m_comm->write("*OPC\n"); }

    /// OPeration Complete query
    bool get_OPC();

    /// Wait for OPC (blocking)
    void wait_for_OPC(unsigned interval_ms = 100);

    /// ReSeT
    void RST() { m_comm->write("*RST\n"); }

    /// Service Request Enable command
    void set_SRE(uint8_t service_request);

    /// Serivce Request Enable query
    uint8_t get_SRE();

    /// STatus Byte query
    uint8_t get_STB();

    /// Self TeST query
    bool TST();

    /// WAIt to continue
    void WAI() { m_comm->write("*WAI\n"); }

protected:
    // Initializer with name for derived classes
    scpi_device() : ld_device() {};
    scpi_device(std::string name) : ld_device(name) {};

};

}

#endif