#include <labdev/devices/rigol/ds1000z.hh>
#include <labdev/utils/utils.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <unistd.h>

using namespace std;

namespace labdev {

ds1000z::ds1000z()
    : osci(4, "Rigol,DS1000Z")
{
    return;
}

ds1000z::ds1000z(std::unique_ptr<tcpip_iface> tcpip) : ds1000z()
{
    this->connect(std::move(tcpip));
    return;
}

ds1000z::ds1000z(std::unique_ptr<usbtmc_iface> usbtmc) : ds1000z()
{
    this->connect(std::move(usbtmc));
    return;
}

ds1000z::ds1000z(std::unique_ptr<visa_iface> visa) : ds1000z()
{
    this->connect(std::move(visa));
    return;
}

ds1000z::~ds1000z() 
{
    if (this->connected())
        this->disconnect();
    return;
}

void ds1000z::connect(std::unique_ptr<ld_iface> comm)
{
    if ( this->connected() ) {
        string err = this->get_info() + " : device is already connected";
        throw device_error(err);
        return;
    }

    Interface_type type = comm->type();
    if (type == TCPIP) {
        // Convert to tcpip interface
        unique_ptr<tcpip_iface> tcpip(
            dynamic_cast<tcpip_iface*>(comm.release()));
        
        // Default port 5555
        if (tcpip->get_port() != ds1000z::PORT) {
            fprintf(stderr, "Rigol DS1000z only supports port %u.\n", ds1000z::PORT);
            abort();
        }

        // Everything seems to be in order
        m_comm = std::move(tcpip);
    } else if (type == USBTMC) {
        // Convert to usbtmc interface
        unique_ptr<usbtmc_iface> usbtmc(
            dynamic_cast<usbtmc_iface*>(comm.release()));

        // USB initialization
        usbtmc->claim_iface(0);
        usbtmc->set_endpoint_in(1);
        usbtmc->set_endpoint_out(2);

        // Everything seems to be in order
        m_comm = std::move(usbtmc);
    } else if (type == VISA) {
        // Convert to usbtmc interface
        unique_ptr<visa_iface> visa(
            dynamic_cast<visa_iface*>(comm.release()));

        m_comm = std::move(visa);
    } else {
        string err = this->get_info() + " : interface is not supported";
        throw device_error(err); 
    } 

    this->init();
    return;
}

void ds1000z::disconnect()
{
    m_comm.reset();
    return;
}

void ds1000z::enable_channel(unsigned channel, bool enable) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":DISP ";
    if (enable) msg << "1\n";
    else msg << "0\n";
    m_comm->write(msg.str());
    return;
}

bool ds1000z::channel_enabled(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":DISP?\n";
    string resp = m_comm->query(msg.str());
    return (stoi(resp) == 1) ? true : false;
}

void ds1000z::set_atten(unsigned channel, double att) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":PROB " << att << "\n";
    m_comm->write(msg.str());
    return;
}

double ds1000z::get_atten(unsigned channel) {
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":PROB?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void ds1000z::set_vert_base(unsigned channel, double volts_per_div) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":SCAL " << volts_per_div << "\n";
    m_comm->write(msg.str());
    return;
}

double ds1000z::get_vert_base(unsigned channel) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":SCAL?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void ds1000z::set_vert_offs(unsigned channel, double offset_v)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":OFFS " << offset_v << "\n";
    m_comm->write(msg.str());
    return;
}

double ds1000z::get_vert_offs(unsigned channel)
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":CHAN" << channel << ":OFFS?\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void ds1000z::set_horz_base(double sec_per_div) 
{
    stringstream msg("");
    msg << ":TIM:SCAL " << sec_per_div << "\n";
    m_comm->write(msg.str());
    return;
}

double ds1000z::get_horz_base() 
{
    string msg = m_comm->query(":TIM:SCAL?\n");
    return stof(msg);
}

void ds1000z::set_horz_offs(double offset_s)
{
    stringstream msg("");
    msg << ":TIM:OFFS " << offset_s << "\n";
    m_comm->write(msg.str());
    return;
}

double ds1000z::get_horz_offs()
{
    string msg = m_comm->query(":TIM:OFFS?\n");
    return stof(msg);
}

void ds1000z::set_meas(unsigned ch, meas_item meas)
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
    msg << ":MEAS:ITEM " << this->meas_to_str(meas) << ",CHAN" << ch << "\n";
    m_comm->write(msg.str());
    return;
}

double ds1000z::get_meas(unsigned ch, meas_item meas)
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
    msg << ":MEAS:ITEM? " << this->meas_to_str(meas) << ",CHAN" << ch << "\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void ds1000z::set_meas(unsigned ch1, unsigned ch2, meas_item meas)
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
    msg << ":MEAS:ITEM " << this->meas_to_str(meas) << ",CHAN" << ch1;
    msg << ",CHAN" << ch2 << "\n";
    m_comm->write(msg.str());
    return;
}

double ds1000z::get_meas(unsigned ch1, unsigned ch2, meas_item meas)
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
    msg << ":MEAS:ITEM? " << this->meas_to_str(meas) << ",CHAN" << ch1;
    msg << ",CHAN" << ch2 << "\n";
    string resp = m_comm->query(msg.str());
    return stof(resp);
}

void ds1000z::clear_meas()
{
    m_comm->write(":MEAS:CLE\n");
    return;
}

void ds1000z::run() 
{
    m_comm->write(":RUN\n");
    return;
}

void ds1000z::stop() 
{
    m_comm->write(":STOP\n");
    return;
}

void ds1000z::single_shot() 
{
    m_comm->write(":SING\n");
    return;
}

void ds1000z::set_trigger_type(trig_type trig) 
{
    stringstream msg("");
    msg << ":TRIG:MODE EDGE\n";
    m_comm->write(msg.str().c_str());

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
    m_comm->write(msg.str().c_str());

    return;
}

void ds1000z::set_trigger_level(double level) 
{
    stringstream msg("");
    msg << ":TRIG:EDG:LEV " << level << "\n";
    m_comm->write(msg.str().c_str());
    return;
}

void ds1000z::set_trigger_source(unsigned channel) 
{
    this->check_channel(channel);
    stringstream msg("");
    msg << ":TRIG:EDG:SOUR CHAN" << channel << "\n";
    m_comm->write(msg.str().c_str());
    return;
}


bool ds1000z::triggered() 
{
    string status = m_comm->query(":TRIG:STAT?\n");
    if ( status.find("TD") != string::npos )
        return true;
    return false;
}

bool ds1000z::stopped() 
{
    string status = m_comm->query(":TRIG:STAT?\n");
    if ( status.find("STOP") != string::npos )
        return true;
    return false;
}

void ds1000z::read_sample_data(unsigned channel, vector<double> &horz_data, 
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
    unsigned npts  = stoi( preamble.at(2) );
    double xincr = stof( preamble.at(4) );
    double xorg  = stof( preamble.at(5) );
    double xref  = stof( preamble.at(6) );
    double yinc  = stof( preamble.at(7) );
    int yorg  = stoi( preamble.at(8) );
    int yref  = stoi( preamble.at(9) );

    debug_print("pts = %i\n", npts);
    debug_print("xincr = %e\n", xincr);
    debug_print("xorg = %e\n", xorg);
    debug_print("xref = %e\n", xref);
    debug_print("yinc = %e\n", yinc);
    debug_print("yorg = %i\n", yorg);
    debug_print("yref = %i\n", yref);

    // Read waveform in chunks of 250kSa
    vector<uint8_t> mem_data, temp;
    unsigned start = 1, stop = 250000;
    while (mem_data.size() < npts) {
        this->set_mem_range(start, stop);
        temp = this->read_mem_data();
        start += temp.size();
        stop += temp.size();
        if (stop > npts) stop = npts;
        mem_data.insert(mem_data.end(), temp.begin(), temp.end());
        usleep(100e3);  // 100ms wait to avoid accessive polling
    }
    debug_print("Total points read from memory: %lu\n", mem_data.size());

    // Convert byte data using preamble
    double xval, yval;
    for (size_t i = 0; i < mem_data.size(); i++) {
        xval = i*xincr + xorg; 
        yval = (mem_data.at(i) - yref - yorg) *  yinc;
        vert_data.push_back(xval);
        horz_data.push_back(yval);
    }

    return;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void ds1000z::init() 
{
    m_comm->write("*CLS\n");
    usleep(100e3);
    m_dev_name = m_comm->query("*IDN?\n");

    // Set waveform format
    m_comm->write(":WAV:FORM BYTE\n");
    m_comm->write(":WAV:MODE MAX\n");
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

void ds1000z::set_mem_range(unsigned sta, unsigned sto)
{    
    // Set start and stop address
    m_comm->write(":WAV:STAR " + to_string(sta) + "\n");
    m_comm->write(":WAV:STOP " + to_string(sto) + "\n");
    return;
}

vector<uint8_t> ds1000z::read_mem_data() 
{
    // Read data block defined by set_mem_range
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