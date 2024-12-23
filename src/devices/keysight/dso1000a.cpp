#include <labdev/devices/keysight/dso1000a.hh>
#include <labdev/utils/utils.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <unistd.h>

using namespace std;

namespace labdev {

dso1000a::dso1000a()
    : osci(4, "Keysight,DSO1000A")
{
    return;
}

dso1000a::dso1000a(std::unique_ptr<usbtmc_interface> usbtmc) : dso1000a()
{
    this->connect(std::move(usbtmc));
    return;
}

dso1000a::dso1000a(std::unique_ptr<visa_interface> visa) : dso1000a()
{
    this->connect(std::move(visa));
    return;
}

dso1000a::~dso1000a() 
{
    m_comm->write(":KEY:LOCK DIS\n");
    if (this->connected())
        this->disconnect();
    return;
}

void dso1000a::connect(std::unique_ptr<ld_interface> comm)
{
    if ( this->connected() ) {
        string err = this->get_info() + " : device is already connected";
        throw device_error(err);
        return;
    }

    Interface_type type = comm->type();
    if (type == USBTMC) {
        // Convert to usbtmc interface
        unique_ptr<usbtmc_interface> usbtmc(
            dynamic_cast<usbtmc_interface*>(comm.release()));
        
        // USB initialization
        usbtmc->claim_interface(0);
        usbtmc->set_endpoint_out(0);
        usbtmc->set_endpoint_in(1);

        // Everything seems to be in order
        m_comm = std::move(usbtmc);
    } else if (type == VISA) {
        // Convert to visa interface
        unique_ptr<visa_interface> visa(
            dynamic_cast<visa_interface*>(comm.release()));
        m_comm = std::move(visa);
    } else {
        string err = this->get_info() + " : interface is not supported";
        throw device_error(err); 
    } 

    this->init(); 
    return;
}

void dso1000a::disconnect()
{
    m_comm.reset();
    return;
}

void dso1000a::enable_channel(unsigned channel, bool enable) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":DISP ";
    if (enable) msg << "1\n";
    else msg << "0\n";
    m_comm->write(msg.str());
    return;
}

bool dso1000a::channel_enabled(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":DISP?";
    string resp = m_comm->query(msg.str());
    return (stoi(resp) == 1) ? true : false;
}

void dso1000a::set_atten(unsigned channel, double att) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":PROB " << att << "X\n";
    m_comm->write(msg.str());
    return;
}

double dso1000a::get_atten(unsigned channel) {
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":PROB?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dso1000a::set_vert_base(unsigned channel, double volts_per_div) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":SCAL " << volts_per_div << "\n";
    m_comm->write(msg.str());
    return;
}

double dso1000a::get_vert_base(unsigned channel) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":SCAL?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dso1000a::set_vert_offs(unsigned channel, double offset_v)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":OFFS " << offset_v << "\n";
    m_comm->write(msg.str());
    return;
}

double dso1000a::get_vert_offs(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":OFFS?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dso1000a::set_horz_base(double sec_per_div) 
{
    stringstream msg("");
    msg << ":TIM:SCAL " << sec_per_div << "\n";
    m_comm->write(msg.str());
    return;
}

double dso1000a::get_horz_base() 
{
    string msg = m_comm->query(":TIM:SCAL?\n");
    return stof(msg);
}

void dso1000a::set_horz_offs(double offset_s)
{
    stringstream msg("");
    msg << ":TIM:OFFS " << offset_s << "\n";
    m_comm->write(msg.str());
    return;
}

double dso1000a::get_horz_offs()
{
    string msg = m_comm->query(":TIM:OFFS?\n");
    return stof(msg);
}

void dso1000a::set_meas(unsigned ch, meas_item meas)
{
    this->check_channel(ch);
    // Single source measurement?
    switch (meas) {
        case VMAX: 
        case VMIN:
        case VPP:
        case VTOP:
        case VBASE:
        case VAMP:
        case VAVG:
        case VRMS:
        case OVERSHOOT:
        case PRESHOOT:
        case FREQ:
        case RISETIME:
        case FALLTIME:
        case POS_WIDTH:
        case NEG_WIDTH:
        case POS_DUTY:
        case NEG_DUTY:
        break;

        default:
        fprintf(stderr, "Invalid single source measurement (%i)\n", meas);
        abort();
    }
    stringstream msg("");
    msg << ":MEAS:" << this->meas_to_str(meas) << " CHAN" << ch << "\n";
    m_comm->write(msg.str());
    return;
}

double dso1000a::get_meas(unsigned ch, meas_item meas)
{
    this->check_channel(ch);
    // Single source measurement?
    switch (meas) {
        case VMAX:
        case VMIN:
        case VPP:
        case VTOP:
        case VBASE:
        case VAMP:
        case VAVG:
        case VRMS:
        case OVERSHOOT:
        case PRESHOOT:
        case FREQ:
        case RISETIME:
        case FALLTIME:
        case POS_WIDTH:
        case NEG_WIDTH:
        case POS_DUTY:
        case NEG_DUTY:
        break;

        default:
        fprintf(stderr, "Invalid single source measurement (%i)\n", meas);
        abort();
    }
    stringstream msg("");
    msg << ":MEAS:" << this->meas_to_str(meas) << "? CHAN" << ch << "\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dso1000a::set_meas(unsigned ch1, unsigned ch2, meas_item meas)
{
    this->check_channel(ch1);
    this->check_channel(ch2);
    // Dual source measurement?
    switch (meas) {
        case POS_DELAY:
        case NEG_DELAY:
        case POS_PHASE:
        case NEG_PHASE:
        break;

        default:
        fprintf(stderr, "Invalid dual source measurement (%i)\n", meas);
        abort();
    }
    stringstream msg("");
    msg << ":MEAS:" << this->meas_to_str(meas) << " CHAN" << ch1;
    msg << ",CHAN" << ch2 << "\n";
    m_comm->write(msg.str());
    return;
}

double dso1000a::get_meas(unsigned ch1, unsigned ch2, meas_item meas)
{
    this->check_channel(ch1);
    this->check_channel(ch2);
    // Dual source measurement?
    switch (meas) {
        case POS_DELAY:
        case NEG_DELAY:
        case POS_PHASE:
        case NEG_PHASE:
        break;

        default:
        fprintf(stderr, "Invalid dual source measurement (%i)\n", meas);
        abort();
    }
    stringstream msg("");
    msg << ":MEAS:" << this->meas_to_str(meas) << "? CHAN" << ch1;
    msg << ",CHAN" << ch2 << "\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void dso1000a::clear_meas()
{
    m_comm->write(":MEAS:CLE\n");
    return;
}

void dso1000a::run() 
{
    m_comm->write(":RUN\n");
    usleep(100e3);
    return;
}

void dso1000a::stop() 
{
    m_comm->write(":STOP\n");
    usleep(100e3);
    return;
}

void dso1000a::single_shot() 
{
    m_comm->write(":SINGLE\n");
    usleep(100e3);
    return;
}

void dso1000a::set_trigger_type(trig_type trig) 
{
    stringstream msg("");
    msg << ":TRIG:MODE EDGE\n";
    m_comm->write(msg.str().c_str());

    msg.str("");
    msg << ":TRIG:EDGE:SLOP ";
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
    m_comm->write(msg.str().c_str());

    return;
}

void dso1000a::set_trigger_level(double level) 
{
    stringstream msg("");
    msg << ":TRIG:EDGE:LEV " << level << "\n";
    m_comm->write(msg.str().c_str());
    return;
}

void dso1000a::set_trigger_source(unsigned channel) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":TRIG:EDGE:SOUR CHAN" << channel << "\n";
    m_comm->write(msg.str().c_str());
    return;
}


bool dso1000a::triggered() 
{
    string status = m_comm->query(":TRIG:STAT?\n");
    if ( status.find("T'D") != string::npos )
        return true;
    return false;
}

bool dso1000a::stopped() 
{
    string status = m_comm->query(":TRIG:STAT?\n");
    if ( status.find("STOP") != string::npos )
        return true;
    return false;
}

void dso1000a::read_sample_data(unsigned channel, vector<double> &horz_data, 
    vector<double> &vert_data) 
{
    // Switch channel
    this->check_channel(channel);
    m_comm->write(":WAV:SOUR CHAN" + to_string(channel) + "\n");

    // Clear vectors
    horz_data.clear();
    vert_data.clear();

    // Get waveform preamble
    string data = m_comm->query(":WAV:PRE?\n");
    vector<string> preamble = split(data, ",", 10);
    if (preamble.size() != 10) {
        debug_print("Received wrong preamble size (%lu): '%s'\n",
            preamble.size(), data.c_str());
        throw device_error("Received incomplete preamble.", -1);
    }

    // Extract data from preamble
    int npts = stoi( preamble.at(2) );
    double xincr = stof( preamble.at(4) );
    double xorg = stof( preamble.at(5) );
    int xref = stoi( preamble.at(6) );
    double yinc = stof( preamble.at(7) );
    double yorg = stof( preamble.at(8) );
    int yref = stoi( preamble.at(9) );

    debug_print("pts = %i\n", npts);
    debug_print("xincr = %e\n", xincr);
    debug_print("xorg = %e\n", xorg);
    debug_print("xref = %i\n", xref);
    debug_print("yinc = %e\n", yinc);
    debug_print("yorg = %e\n", yorg);
    debug_print("yref = %i\n", yref);

    vector<uint8_t> mem_data = this->read_mem_data(); 
    debug_print("Total points read from memory: %lu\n", mem_data.size());

    // Convert byte data using preamble
    double xval, yval;
    for (size_t i = 0; i < mem_data.size(); i++) {
        // For calculations see manual p.24
        xval = i*xincr + xorg; 
        yval = (yref - mem_data.at(i)) * yinc - yorg;
        vert_data.push_back(xval);
        horz_data.push_back(yval);
    }

    return;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void dso1000a::init() 
{
    m_comm->write("*CLS\n");
    usleep(100e3);  // DSO needs some time...
    m_dev_name = m_comm->query("*IDN?\n");

    // Set waveform format
    m_comm->write(":WAV:FORM BYTE\n");
    m_comm->write(":WAV:POIN:MODE MAX\n");
    return;
}

void dso1000a::check_channel(unsigned channel) 
{
    if ( (channel == 0) || (channel > this->get_n_channels()) ) {
        fprintf(stderr, "Invalid channel %i\n", channel);
        abort();
    }
    return;
}

vector<uint8_t> dso1000a::read_mem_data() 
{
    // Read data block
    string data = m_comm->query(":WAV:DATA?\n");
    // Extract header
    size_t len = 0;
    string header = data.substr(0, 11);
    sscanf(header.c_str(), "#9%9zd", &len);
    debug_print("len = %zu\n", len);

    // Read the waveform
    while (data.size() < len)
        data.append( m_comm->read() );

    vector<uint8_t> ret;
    for (size_t i = 0; i < len; i++)
        ret.push_back( (uint8_t)data.at(i + header.size()) );

    return ret;   
}

}