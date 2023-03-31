#include <labdev/devices/baumer/om70_l.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/exceptions.hh>

using namespace std;

namespace labdev {

om70_l::om70_l() 
    : device("Baumer,OM70-L"), m_modbus(nullptr), m_quality(0),  m_dist(0),
      m_sr(0), m_exp(0), m_quality_vec(), m_dist_vec(), m_sr_vec(), m_exp_vec(),
      m_config_mode(false)
{

};

om70_l::om70_l(ip_address &ip) : om70_l() 
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

void om70_l::enable_laser(bool ena)
{
    if (!m_config_mode) {
        m_modbus->write_single_holding_reg(UNIT_ID, ADDR_CONF_ON, 0x0001);
        m_config_mode = true;
    }

    if (ena)
        m_modbus->write_single_holding_reg(UNIT_ID, ADDR_ENA_LASER, 0x0001);
    else
        m_modbus->write_single_holding_reg(UNIT_ID, ADDR_ENA_LASER, 0x0000);

    return;
}

float om70_l::get_measurement()
{
    // Turn off config mode to increase sample rate
    if (m_config_mode) {
        m_modbus->write_single_holding_reg(UNIT_ID, ADDR_CONF_OFF, 0x0001);
        m_config_mode = false;
    }

    vector<uint16_t> resp{};
    resp = m_modbus->read_input_regs(UNIT_ID, ADDR_ALL_MEAS, 17);
    m_quality = resp.at(1);
    m_dist  = static_cast<float>( resp.at(3) | (resp.at(4) << 16) );
    m_sr    = static_cast<float>( resp.at(5) | (resp.at(6) << 16) );
    m_exp   = static_cast<float>( resp.at(7) | (resp.at(8) << 16) );
    return 0.;
}

int om70_l::get_quality()
{
    return m_quality;
}

float om70_l::get_sample_rate()
{
    return m_sr;
}

float om70_l::get_exposure()
{
    return m_exp;
}

vector<float> om70_l::get_measurement_mem()
{
    return m_dist_vec;
}

vector<int> om70_l::get_quality_mem()
{
    return m_quality_vec;
}

vector<float> om70_l::get_sample_rate_mem()
{
    return m_sr_vec;
}

vector<float> om70_l::get_exposure_mem()
{
    return m_exp_vec;
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