#include <labdev/devices/scpi_device.hh>
#include <labdev/utils/utils.hh>
#include <labdev/exceptions.hh>

#include <unistd.h>

using namespace std;

namespace labdev {

void scpi_device::set_ESE(uint8_t event_status)
{
    string msg = "*ESE " + to_string(event_status) + "\n";
    m_comm->write(msg);
    return;
}

uint8_t scpi_device::get_ESE()
{
    string resp = m_comm->query("*ESE?\n");
    bool success {false};
    uint8_t ese = convert_to<uint8_t>(resp, success);
    if (!success)
        throw bad_protocol("Failed to convert '*ESE?' query response: " + resp);
    return ese;
}

uint8_t scpi_device::get_ESR()
{
    string resp = m_comm->query("*ESR?\n");
    bool success {false};
    uint8_t esr = convert_to<uint8_t>(resp, success);
    if (!success)
        throw bad_protocol("Failed to convert '*ESR?' query response: " + resp);
    return esr;
}

bool scpi_device::get_OPC()
{
    string resp = m_comm->query("*OPC?\n");
    bool success {false}, opc {false};
    opc = convert_to<uint8_t>(resp, success);
    if (!success)
        throw bad_protocol("Failed to convert '*OPC?' query response: " + resp);
    return opc;
}

void scpi_device::wait_for_OPC(unsigned interval_ms)
{
    bool opc {false};
    while (!opc) {
        opc = this->get_OPC();
        usleep(interval_ms * 1e3);  // To avoid excessive polling
    }
    return;
}

void scpi_device::set_SRE(uint8_t service_request)
{
    string msg = "*SRE " + to_string(service_request) + "\n";
    m_comm->write(msg);
    return;
}

uint8_t scpi_device::get_SRE()
{
    string resp = m_comm->query("*OPC?\n");
    bool success {false};
    uint8_t sre = convert_to<uint8_t>(resp, success);
    if (!success)
        throw bad_protocol("Failed to convert '*SRE?' query response: " + resp);
    return sre;
}

uint8_t scpi_device::get_STB()
{
    string resp = m_comm->query("*OPC?\n");
    bool success {false};
    uint8_t stb = convert_to<uint8_t>(resp, success);
    if (!success)
        throw bad_protocol("Failed to convert '*STB?' query response: " + resp);
    return stb;
}

bool scpi_device::TST()
{
    m_comm->write("*TST?\n");
    string resp {};
    while (resp.empty()) {  // Wait until self test is done
        resp = m_comm->read();
    }
    bool success {false}, tst {false};
    tst = convert_to<uint8_t>(resp, success);
    if (!success)
        throw bad_protocol("Failed to convert '*TST?' query response: " + resp);
    return !tst;    // 0 = success, everything else means failed
}

}