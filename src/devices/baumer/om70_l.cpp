#include <labdev/devices/baumer/om70_l.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <cmath>

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
    this->extract_measurements(resp, m_dist, m_quality, m_sr, m_exp);

    return m_dist;
}

vector<float> om70_l::get_measurement_mem()
{
    return m_dist_vec;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void om70_l::extract_measurements(std::vector<uint16_t> data, float &dist, 
    int &quality, float &sample_rate, float &exposure)
{
    ld_assert(data.size() == 17, "OM70-L measurements are constructed from 17x"
        "16-bit registers, but received %zux 16-bit.", data.size());

    quality = data.at(1);
    // memcopy is required, casting results in wrong conversion to integer and
    // attaches a comma (bitwise conversion vs. cast)
    uint32_t val {0x0000};
    val = data.at(3) | (data.at(4) << 16);
    memcpy(&dist, &val, sizeof(float));
    val = data.at(5) | (data.at(6) << 16);
    memcpy(&sample_rate, &val, sizeof(float));
    val = data.at(7) | (data.at(8) << 16);
    memcpy(&exposure, &val, sizeof(float));

    if (isnan(m_dist)) dist = -1.;
    
    return;
}

}