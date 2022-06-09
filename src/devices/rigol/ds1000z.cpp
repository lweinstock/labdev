#include <labdev/devices/rigol/ds1000z.hh>
#include <labdev/utils/utils.hh>
#include "ld_debug.hh"

#include <sstream>
#include <unistd.h>

namespace labdev {

    ds1000z::ds1000z():
    oscilloscope(4) {
        return;
    }

    ds1000z::ds1000z(tcpip_interface* tcpip):
    oscilloscope(4), 
    scpi_device(tcpip) {
        // General initialization
        init();
        return;
    }

    ds1000z::ds1000z(visa_interface* visa):
    oscilloscope(4), 
    scpi_device(visa) {
        // General initialization
        init();
        return;
    }

    ds1000z::ds1000z(usbtmc_interface* usbtmc):
    oscilloscope(4), 
    scpi_device(usbtmc) {
        // USB initialization
        usbtmc->claim_interface(0);
        usbtmc->set_endpoint_in(0x02);
        usbtmc->set_endpoint_out(0x03);

        // General initialization
        init();
        return;
    }

    ds1000z::~ds1000z() {
        return;
    }

    void ds1000z::enable_channel(unsigned channel, bool enable) {
        this->check_channel(channel);
        std::stringstream msg("");
        msg << ":CHAN" << channel << ":DISP ";
        if (enable) msg << "1\n";
        else msg << "0\n";
        comm->write(msg.str());
        return;
    }

    void ds1000z::set_atten(unsigned channel, double att) {
        this->check_channel(channel);
        std::stringstream msg("");
        msg << ":CHAN" << channel << ":PROB " << att << "\n";
        comm->write(msg.str());
        return;
    }

    double ds1000z::get_atten(unsigned channel) {
        this->check_channel(channel);
        std::stringstream msg("");
        msg << ":CHAN" << channel << ":PROB?\n";
        std::string resp = comm->query(msg.str());
        return std::stof(resp);
    }

    void ds1000z::set_vert_base(unsigned channel, double volts_per_div) {
        this->check_channel(channel);
        std::stringstream msg("");
        msg << ":CHAN" << channel << ":SCAL " << volts_per_div << "\n";
        comm->write(msg.str());
        return;
    }

    double ds1000z::get_vert_base(unsigned channel) {
        this->check_channel(channel);
        std::stringstream msg("");
        msg << ":CHAN" << channel << ":SCAL?\n";
        std::string resp = comm->query(msg.str());
        return std::stof(resp);
    }

    void ds1000z::set_horz_base(double sec_per_div) {
        std::stringstream msg("");
        msg << ":TIM:SCAL " << sec_per_div << "\n";
        comm->write(msg.str());
        return;
    }

    double ds1000z::get_horz_base() {
        std::string msg = comm->query(":TIM:SCAL?\n");
        return std::stof(msg);
    }

    void ds1000z::start_acquisition() {
        comm->write(":RUN\n");
        return;
    }

    void ds1000z::stop_acquisition() {
        comm->write(":STOP\n");
        return;
    }

    void ds1000z::single_shot() {
        comm->write(":SING\n");
        return;
    }

    void ds1000z::set_trigger_type(trigger_type trig) {
        std::stringstream msg("");
        msg << ":TRIG:MODE EDGE\n";
        comm->write(msg.str().c_str());

        msg.str("");
        msg << ":TRIG:EDG:SLOP ";
        switch (trig) {
        case RISE: 
            msg << "POS\n"; 
            break;
        case FALL: 
            msg << "NEG\n"; 
            break;
        case BOTH: 
            msg << "RFAL\n"; 
            break;
        default:
            fprintf(stderr, "Invalid trigger type received: %02X\n", trig);
            abort();
        }
        comm->write(msg.str().c_str());

        return;
    }

    void ds1000z::set_trigger_level(double level) {
        std::stringstream msg("");
        msg << ":TRIG:EDG:LEV " << level << "\n";
        comm->write(msg.str().c_str());
        return;
    }

    void ds1000z::set_trigger_source(unsigned channel) {
        this->check_channel(channel);
        std::stringstream msg("");
        msg << ":TRIG:EDG:SOUR CHAN" << channel << "\n";
        comm->write(msg.str().c_str());
        return;
    }


    bool ds1000z::triggered() {
        std::string status = comm->query(":TRIG:STAT?\n");
        if ( status.find("TD") != std::string::npos )
            return true;
        return false;
    }

    bool ds1000z::stopped() {
        std::string status = comm->query(":TRIG:STAT?\n");
        if ( status.find("STOP") != std::string::npos )
            return true;
        return false;
    }
    
    void ds1000z::read_sample_data(unsigned channel,
    std::vector<double> &horz_data, std::vector<double> &vert_data) {
        // Switch channel
        this->check_channel(channel);
        comm->write(":WAV:SOUR CHAN" + std::to_string(channel) + "\n");

        // Clear vectors
        horz_data.clear();
        vert_data.clear();

        // Get waveform preamble
        std::string data = comm->query(":WAV:PRE?\n");
        std::vector<std::string> preamble = split(data, ",", 10);
        if (preamble.size() != 10) {
            debug_print("Received wrong preamble size: %s (%lu)\n",
                data.c_str(), preamble.size());
            throw device_error("Received incomplete preamble.", -1);
        }

        // Extract data from preamble
        m_npts  = std::stoi( preamble.at(2) );
        m_xincr = std::stof( preamble.at(4) );
        m_xorg  = std::stof( preamble.at(5) );
        m_xref  = std::stof( preamble.at(6) );
        m_yinc  = std::stof( preamble.at(7) );
        m_yorg  = std::stoi( preamble.at(8) );
        m_yref  = std::stoi( preamble.at(9) );

        debug_print("pts = %i\n", m_npts);
        debug_print("xincr = %e\n", m_xincr);
        debug_print("xorg = %e\n", m_xorg);
        debug_print("xref = %e\n", m_xref);
        debug_print("yinc = %e\n", m_yinc);
        debug_print("yorg = %i\n", m_yorg);
        debug_print("yref = %i\n", m_yref);

        // Read waveform in chunks of 250kSa
        std::vector<uint8_t> mem_data, temp;
        unsigned start = 1, stop = 250000;
        while (mem_data.size() < m_npts) {
            temp = this->read_mem_data(start, stop);
            start += temp.size();
            stop += temp.size();
            if (stop > m_npts) stop = m_npts;
            mem_data.insert(mem_data.end(), temp.begin(), temp.end());
        }
        debug_print("Total points read from memory: %lu\n", mem_data.size());

        // Convert byte data using preamble
        double xval, yval;
        for (int i = 0; i < mem_data.size(); i++) {
            xval = i*m_xincr + m_xorg; 
            yval = (mem_data.at(i) - m_yref - m_yorg) * m_yinc;
            vert_data.push_back(xval);
            horz_data.push_back(yval);
        }

        return;
    }

    void ds1000z::set_measurement(unsigned channel, unsigned item) {
        this->set_measurement(channel, channel, item);
        return;
    }

    void ds1000z::set_measurement(unsigned channel1, unsigned channel2,
    unsigned item) {
        this->check_channel(channel1);
        this->check_channel(channel2);
        std::stringstream msg("");
        msg << ":MEAS:STAT:ITEM " << s_meas_item_string[item]
            << ",CHAN" << channel1 << ",CHAN" << channel2 << "\n";
        comm->write(msg.str());
        return;
    }

    double ds1000z::get_measurement(unsigned channel1, unsigned channel2,
    unsigned item, unsigned type) {
        this->check_channel(channel1);
        std::stringstream msg("");
        msg << ":MEAS:STAT:ITEM? "
            << s_meas_type_string[type] << ","
            << s_meas_item_string[item] << ",CHAN"
            << channel1 << ",CHAN"
            << channel2 << "\n";
        std::string resp = comm->query(msg.str());
        return std::stod(resp);
    }

    double ds1000z::get_measurement(unsigned channel, unsigned item,
    unsigned type) {
        return this->get_measurement(channel, channel, item, type);
    }

    void ds1000z::clear_measurements() {
        comm->write(":MEAS:CLE ALL\n");
        return;
    }

    void ds1000z::reset_measurements() {
        comm->write(":MEAS:STAT:RES\n");
        return;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    const std::string ds1000z::s_meas_item_string[] = {"VMAX", "VMIN", "VPP",
        "VTOP", "VBAS", "VAMP", "VAVG", "VRMS", "OVER", "PRES", "MAR", "MPAR",
        "PER", "FREQ", "RTIM", "FTIM", "PWID", "NWID", "PDUT", "NDUT", "RDEL",
        "FDEL", "RPH", "FPH"};

    const std::string ds1000z::s_meas_type_string[] = {"MAX", "MIN", "CURR",
        "AVER", "DEV"};

    void ds1000z::init() {
        this->clear_status();
        // Set waveform format
        comm->write(":WAV:FORM BYTE\n");
        comm->write(":WAV:MODE MAX\n");
        // Initialize members
        m_xincr = 0.;
        m_xorg = 0.;
        m_yinc = 0.;
        m_yorg = 0.;
        m_yref = 0.;
        return;
    }

    void ds1000z::check_channel(unsigned channel) {
        if ( (channel == 0) || (channel > this->get_n_channels()) ) {
            fprintf(stderr, "Invalid channel %i\n", channel);
            abort();
        }
        return;
    }

    std::vector<uint8_t> ds1000z::read_mem_data(unsigned sta, unsigned sto) {
        // Set start and stop address
        comm->write(":WAV:STAR " + std::to_string(sta) + "\n");
        comm->write(":WAV:STOP " + std::to_string(sto) + "\n");

        // Read data block
        std::string data = comm->query(":WAV:DATA?\n");
        // Extract header
        int len = 0;
        std::string header = data.substr(0, 11);
        sscanf(header.c_str(), "#9%9d", &len);
        debug_print("len = %i\n", len);

        // Read the waveform
        while (data.size() < len)
            data.append( comm->read() );

        std::vector<uint8_t> ret;
        for (int i = 0; i < len; i++)
            ret.push_back( (uint8_t)data.at(i + header.size()) );

        return ret;   
    }

}