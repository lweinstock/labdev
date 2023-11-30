#include <labdev/devices/rigol/ds1000z.hh>
#include <labdev/utils/utils.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <unistd.h>

using namespace std;

namespace labdev {

ds1000z::ds1000z()
    : osci(4, "Rigol,DS1000Z"), m_scpi(nullptr), m_npts(0), m_xincr(0), 
      m_xorg(0), m_xref(0), m_yinc(0), m_yorg(0), m_yref(0)
{
    return;
}

ds1000z::ds1000z(tcpip_interface* tcpip) : ds1000z()
{
    this->connect(tcpip);
    return;
}

ds1000z::ds1000z(usbtmc_interface* usbtmc) : ds1000z()
{
    this->connect(usbtmc);
    return;
}

ds1000z::ds1000z(visa_interface* visa) : ds1000z()
{
    return;
}

void ds1000z::connect(tcpip_interface* tcpip)
{
    // Check and set comm interface
    this->set_comm(tcpip);

    // Default port 5555
    if (tcpip->get_port() != ds1000z::PORT) {
        fprintf(stderr, "Rigol DS1000z only supports port %u.\n", ds1000z::PORT);
        abort();
    }

    this->init();
    return;
}
void ds1000z::connect(usbtmc_interface* usbtmc)
{
    // Check and set comm interface
    this->set_comm(usbtmc);

    // USB initialization
    usbtmc->claim_interface(0);
    usbtmc->set_endpoint_in(1);
    usbtmc->set_endpoint_out(2);

    this->init();
    return;
}

void ds1000z::connect(visa_interface* visa)
{
    // Check and set comm interface
    this->set_comm(visa);

    this->init();
    return;
}

void ds1000z::enable_channel(unsigned channel, bool enable) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":DISP ";
    if (enable) msg << "1\n";
    else msg << "0\n";
    get_comm()->write(msg.str());
    return;
}

void ds1000z::set_atten(unsigned channel, double att) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":PROB " << att << "\n";
    get_comm()->write(msg.str());
    return;
}

double ds1000z::get_atten(unsigned channel) {
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":PROB?\n";
    string resp = get_comm()->query(msg.str());
    return stof(resp);
}

void ds1000z::set_vert_base(unsigned channel, double volts_per_div) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":SCAL " << volts_per_div << "\n";
    get_comm()->write(msg.str());
    return;
}

double ds1000z::get_vert_base(unsigned channel) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":SCAL?\n";
    string resp = get_comm()->query(msg.str());
    return stof(resp);
}

void ds1000z::set_horz_base(double sec_per_div) 
{
    stringstream msg("");
    msg << ":TIM:SCAL " << sec_per_div << "\n";
    get_comm()->write(msg.str());
    return;
}

double ds1000z::get_horz_base() 
{
    string msg = get_comm()->query(":TIM:SCAL?\n");
    return stof(msg);
}

void ds1000z::start_acquisition() 
{
    get_comm()->write(":RUN\n");
    return;
}

void ds1000z::stop_acquisition() 
{
    get_comm()->write(":STOP\n");
    return;
}

void ds1000z::single_shot() 
{
    get_comm()->write(":SING\n");
    return;
}

void ds1000z::set_trigger_type(trigger_type trig) 
{
    stringstream msg("");
    msg << ":TRIG:MODE EDGE\n";
    get_comm()->write(msg.str().c_str());

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
    get_comm()->write(msg.str().c_str());

    return;
}

void ds1000z::set_trigger_level(double level) 
{
    stringstream msg("");
    msg << ":TRIG:EDG:LEV " << level << "\n";
    get_comm()->write(msg.str().c_str());
    return;
}

void ds1000z::set_trigger_source(unsigned channel) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":TRIG:EDG:SOUR CHAN" << channel << "\n";
    get_comm()->write(msg.str().c_str());
    return;
}


bool ds1000z::triggered() 
{
    string status = get_comm()->query(":TRIG:STAT?\n");
    if ( status.find("TD") != string::npos )
        return true;
    return false;
}

bool ds1000z::stopped() 
{
    string status = get_comm()->query(":TRIG:STAT?\n");
    if ( status.find("STOP") != string::npos )
        return true;
    return false;
}

void ds1000z::read_sample_data(unsigned channel, vector<double> &horz_data, 
    vector<double> &vert_data) 
{
    // Switch channel
    this->check_channel(channel);
    get_comm()->write(":WAV:SOUR CHAN" + to_string(channel) + "\n");

    // Clear vectors
    horz_data.clear();
    vert_data.clear();

    // Get waveform preamble
    string data = get_comm()->query(":WAV:PRE?\n");
    vector<string> preamble = split(data, ",", 10);
    if (preamble.size() != 10) {
        debug_print("Received wrong preamble size (%lu): '%s'\n",
            preamble.size(), data.c_str());
        throw device_error("Received incomplete preamble.", -1);
    }

    // Extract data from preamble
    m_npts  = stoi( preamble.at(2) );
    m_xincr = stof( preamble.at(4) );
    m_xorg  = stof( preamble.at(5) );
    m_xref  = stof( preamble.at(6) );
    m_yinc  = stof( preamble.at(7) );
    m_yorg  = stoi( preamble.at(8) );
    m_yref  = stoi( preamble.at(9) );

    debug_print("pts = %i\n", m_npts);
    debug_print("xincr = %e\n", m_xincr);
    debug_print("xorg = %e\n", m_xorg);
    debug_print("xref = %e\n", m_xref);
    debug_print("yinc = %e\n", m_yinc);
    debug_print("yorg = %i\n", m_yorg);
    debug_print("yref = %i\n", m_yref);

    // Read waveform in chunks of 250kSa
    vector<uint8_t> mem_data, temp;
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
    for (size_t i = 0; i < mem_data.size(); i++) {
        xval = i*m_xincr + m_xorg; 
        yval = (mem_data.at(i) - m_yref - m_yorg) * m_yinc;
        vert_data.push_back(xval);
        horz_data.push_back(yval);
    }

    return;
}

void ds1000z::set_measurement(unsigned channel, unsigned item) 
{
    this->set_measurement(channel, channel, item);
    return;
}

void ds1000z::set_measurement(unsigned channel1, unsigned channel2, 
    unsigned item) 
{
    this->check_channel(channel1);
    this->check_channel(channel2);
    stringstream msg("");
    msg << ":MEAS:STAT:ITEM " << s_meas_item_string[item]
        << ",CHAN" << channel1 << ",CHAN" << channel2 << "\n";
    get_comm()->write(msg.str());
    return;
}

double ds1000z::get_measurement(unsigned channel1, unsigned channel2, 
    unsigned item, unsigned type) 
{
    this->check_channel(channel1);
    stringstream msg("");
    msg << ":MEAS:STAT:ITEM? "
        << s_meas_type_string[type] << ","
        << s_meas_item_string[item] << ",CHAN"
        << channel1 << ",CHAN"
        << channel2 << "\n";
    string resp = get_comm()->query(msg.str());
    return stod(resp);
}

double ds1000z::get_measurement(unsigned channel, unsigned item, unsigned type) 
{
    return this->get_measurement(channel, channel, item, type);
}

void ds1000z::clear_measurements() 
{
    get_comm()->write(":MEAS:CLE ALL\n");
    return;
}

void ds1000z::reset_measurements() 
{
    get_comm()->write(":MEAS:STAT:RES\n");
    return;
}

/*
 *      P R I V A T E   M E T H O D S
 */

const string ds1000z::s_meas_item_string[] = {"VMAX", "VMIN", "VPP",
    "VTOP", "VBAS", "VAMP", "VAVG", "VRMS", "OVER", "PRES", "MAR", "MPAR",
    "PER", "FREQ", "RTIM", "FTIM", "PWID", "NWID", "PDUT", "NDUT", "RDEL",
    "FDEL", "RPH", "FPH"};

const string ds1000z::s_meas_type_string[] = {"MAX", "MIN", "CURR",
    "AVER", "DEV"};

void ds1000z::init() 
{
    // Setup SCPI
    m_scpi = std::make_unique<scpi>( this->get_comm() );
    m_scpi->clear_status();
    m_dev_name = m_scpi->get_identifier();

    // Set waveform format
    get_comm()->write(":WAV:FORM BYTE\n");
    get_comm()->write(":WAV:MODE MAX\n");
    // Initialize members
    m_xincr = 0.;
    m_xorg = 0.;
    m_yinc = 0.;
    m_yorg = 0.;
    m_yref = 0.;
    return;
}

void ds1000z::check_channel(unsigned channel) 
{
    if ( (channel == 0) || (channel > this->get_n_channels()) ) {
        fprintf(stderr, "Invalid channel %i\n", channel);
        abort();
    }
    return;
}

vector<uint8_t> ds1000z::read_mem_data(unsigned sta, unsigned sto) 
{
    // Set start and stop address
    get_comm()->write(":WAV:STAR " + to_string(sta) + "\n");
    get_comm()->write(":WAV:STOP " + to_string(sto) + "\n");

    // Read data block
    string data = get_comm()->query(":WAV:DATA?\n");
    // Extract header
    size_t len = 0;
    string header = data.substr(0, 11);
    sscanf(header.c_str(), "#9%9zd", &len);
    debug_print("len = %zu\n", len);

    // Read the waveform
    while (data.size() < len)
        data.append( get_comm()->read() );

    vector<uint8_t> ret;
    for (size_t i = 0; i < len; i++)
        ret.push_back( (uint8_t)data.at(i + header.size()) );

    return ret;   
}

}