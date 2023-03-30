#include <labdev/devices/baumer/om70_l.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/exceptions.hh>

using namespace std;

namespace labdev {

om70_l::om70_l(ip_address &ip) : device() 
{
    this->connect(ip);
    return;
}

om70_l::~om70_l()
{
    if (m_modbus) {
        delete m_modbus;
        m_modbus = nullptr;
    }
    return;
}

void om70_l::connect(ip_address &ip) 
{
    if ( this->connected() ) {
        fprintf(stderr, "Device is already connected!\n");
        abort();
    }
    // Default port 502
    if (ip.port != om70_l::PORT) {
        fprintf(stderr, "OM70-L only supports port %u.\n", om70_l::PORT);
        abort();
    }
    m_comm = new tcpip_interface(ip);
    m_modbus = new modbus_tcp(static_cast<tcpip_interface*>(m_comm));
    return;
}

float om70_l::get_measurement()
{
    return 0.;
}

int om70_l::get_quality()
{
    return 0;
}

float om70_l::get_sample_rate()
{
    return 0.;
}

float om70_l::get_exposure()
{
    return 0.;
}

float om70_l::get_delay()
{
    return 0.;
}


vector<float> om70_l::get_measurement_mem()
{
    vector<float> ret;
    return ret;
}

vector<int> om70_l::get_quality_mem()
{
    vector<int> ret;
    return ret;
}

vector<float> om70_l::get_sample_rate_mem()
{
    vector<float> ret;
    return ret;
}

vector<float> om70_l::get_exposure_mem()
{
    vector<float> ret;
    return ret;
}

vector<float> om70_l::get_delay_mem()
{
    vector<float> ret;
    return ret;
}

/*
 *      P R I V A T E   M E T H O D S
 */

/*
void om70_l::get_measurement() {
    uint16_t msg[MAX_MSG_LEN];
    // Input register "All Measurements" (manual p. 60)
    this->read_input_registers(0x00C8, 0x0011, msg);

    // Reg 1 = signal quality
    m_signal_quality = msg[1];

    // Reg 3 & 4 = distance measurement [mm]
    uint32_t val = (uint32_t)(msg[3] | (msg[4] << 16));
    memcpy(&m_distance, &val, sizeof(float));
    // Reg 5 & 6 = sample rate [Hz]
    val = (uint32_t)(msg[5] | (msg[6] << 16));
    memcpy(&m_sample_rate, &val, sizeof(float));
    // Reg 7 & 8 = exposure reserve [a.u.]
    val = (uint32_t)(msg[7] | (msg[8] << 16));
    memcpy(&m_exposure, &val, sizeof(float));
    // Reg 9 & 10 = responde delay seconds [s]
    uint32_t delay_s = (uint32_t)(msg[9] | (msg[10] << 16));
    // Reg 11 & 12 = responde delay microseconds [us]
    uint32_t delay_us = (uint32_t)(msg[11] | (msg[12] << 16));
    m_response_delay_ms = delay_s*1000. + delay_us/1000.;
    return;
}
*/

}