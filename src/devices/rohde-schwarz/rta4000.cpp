#include <labdev/devices/rohde-schwarz/rta4000.hh>
#include "ld_debug.hh"

#include <sstream>
#include <unistd.h>

namespace labdev {

    rta4000::rta4000():
    oscilloscope(4) {
        return;
    }

    rta4000::rta4000(tcpip_interface* tcpip):
    oscilloscope(4), 
    scpi_device(tcpip) {
        // General initialization
        init();
        return;
    }

    rta4000::rta4000(visa_interface* visa):
    oscilloscope(4), 
    scpi_device(visa) {
        // General initialization
        init();
        return;
    }

    rta4000::rta4000(usbtmc_interface* usbtmc):
    oscilloscope(4), 
    scpi_device(usbtmc) {
        // USB initialization
        usbtmc->claim_interface(0);
        usbtmc->set_endpoint_in(0x01);
        usbtmc->set_endpoint_out(0x02);

        // General initialization
        init();
        return;
    }

    rta4000::rta4000(serial_interface* serial):
    oscilloscope(4), 
    scpi_device(serial) {
        // General initialization
        init();
        return;
    }

    void rta4000::enable_channel(unsigned channel, bool enable) {
        return;
    };

    void rta4000::set_atten(unsigned channel, double att) {
        return;
    };

    double rta4000::get_atten(unsigned channel) {
        return -1.;
    };

    void rta4000::set_vert_base(unsigned channel, double volts_per_div) {
        return;
    };

    double rta4000::get_vert_base(unsigned channel) {
        return -1.;
    };

    void rta4000::set_horz_base(double sec_per_div) {
        return;
    };

    double rta4000::get_horz_base() {
        return -1.;
    };

    void rta4000::start_acquisition() {
        return;
    };

    void rta4000::stop_acquisition() {
        return;
    };

    void rta4000::single_shot() {
        return;
    };

    void rta4000::set_trigger_type(trigger_type trig) {
        return;
    };

    void rta4000::set_trigger_level(double level) {
        return;
    };

    void rta4000::set_trigger_source(unsigned channel) {
        return;
    };

    bool rta4000::triggered() {
        return false;
    };

    bool rta4000::stopped() {
        return false;
    };

    void rta4000::read_sample_data(unsigned channel, 
    std::vector<double> &horz_data, std::vector<double> &vert_data) {
        return;
    };

    /*
     *      P R I V A T E   M E T H O D S
     */

    void rta4000::init() {
        this->clear_status();
        // Initialize members
        m_xincr = 0.;
        m_xorg = 0.;
        m_yinc = 0.;
        m_yorg = 0.;
        m_yref = 0.;
        return;
    }

}