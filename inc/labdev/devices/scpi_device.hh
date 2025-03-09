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
    scpi_device() {};
    ~scpi_device() {};

    /// CLear Status
    void CLS() { m_comm->write("*CLS\n"); }

    /// Event Status Enable command
    void set_ESE(uint8_t event_status);

    /// Event Status Enable query
    uint8_t get_ESE();

    /// Event Status Read
    uint8_t get_ESR();

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

private:

};

}

#endif