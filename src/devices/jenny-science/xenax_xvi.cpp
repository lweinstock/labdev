#include <labdev/devices/jenny-science/xenax_xvi.hh>

#include <unistd.h>
#include <sys/time.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>

#include <labdev/tcpip_interface.hh>
#include <labdev/serial_interface.hh>
#include <labdev/exceptions.hh>
#include <labdev/utils/utils.hh>
#include <labdev/ld_debug.hh>

using namespace std;

namespace labdev {

constexpr unsigned xenax_xvi::PORT;

xenax_xvi::xenax_xvi()
    : ld_device("XENAX Xvi 75v8"), m_force_const(0), m_error(0),
      m_output_type(0x5555), m_output_activity(0xFF), m_error_pending(false)
{
    return;
}

xenax_xvi::xenax_xvi(serial_interface* ser) : xenax_xvi()
{
    this->connect(ser);
    return;
}

xenax_xvi::xenax_xvi(tcpip_interface* tcpip) : xenax_xvi()
{
    this->connect(tcpip);
    return;
}

xenax_xvi::~xenax_xvi()
{
    if (this->connected())
        this->disconnect();
    return;
}

void xenax_xvi::connect(serial_interface* ser)
{
    // Check and assign interface
    this->set_comm(ser);

    // Check for correct serial setup => 115200 8N1 (manual p. 28)
    if (ser->get_baud() != 115200) {
        fprintf(stderr, "Invalid baud rate %u BAUD; 115200 8N1 required\n", 
            ser->get_baud());
        abort();
    }
    if (ser->get_nbits() != 8) {
        fprintf(stderr, "Invalid number of bits %u; 115200 8N1 required\n", 
            ser->get_nbits());
        abort();
    }
    if (ser->get_parity() != false) {
        fprintf(stderr, "Invalid parity; 115200 8N1 required\n");
        abort();
    }
    if (ser->get_stop_bits() != 1) {
        fprintf(stderr, "Invalid number of stop bits %u; 115200 8N1 required\n", 
            ser->get_stop_bits());
        abort();
    }

    this->init();
    return;
}

void xenax_xvi::connect(tcpip_interface* tcpip)
{
    // Check and assign interface
    this->set_comm(tcpip);

    // Check port => 10001
    if (tcpip->get_port() != xenax_xvi::PORT) {
        fprintf(stderr, "Invalid port %u (port 10001 required).\n", 
            tcpip->get_port());
        abort();
    }
    this->init();
    return;
}

void xenax_xvi::disconnect()
{
    this->stop_motion();
    this->reset_comm();
    return;
}

void xenax_xvi::reference_axis() 
{
    debug_print("%s\n", "Referencing axis...");
    this->query_cmd("REF");
    return;
}

void xenax_xvi::set_reference_dir(ref_dir dir)
{
    debug_print("Changing reference direction to %u\n", dir);
    this->query_cmd("DRHR" + to_string(dir));
    return;
}

xenax_xvi::ref_dir xenax_xvi::get_reference_dir()
{
    bool success = false;
    unsigned tmp = convert_to<unsigned>(this->query_cmd("DRHR?"), success);
    if (!success)
        throw bad_protocol("Failed to get reference direction");
    ref_dir dir = static_cast<ref_dir>(tmp);
    debug_print("Current reference direction %u\n", dir);
    return dir;
}

int xenax_xvi::get_position() { 
    bool success = false;
    unsigned ret = convert_to<unsigned>(this->query_cmd("TP"), success);
    if (!success)
        throw bad_protocol("Failed to get current position");
    return ret;
}


bool xenax_xvi::force_limit_reached() 
{
    return (this->get_status_register() & I_FORCE_LIMIT_REACHED);
}

void xenax_xvi::stop_motion() 
{
    this->query_cmd("SM");
    this->wait_status_clr(IN_MOTION, 200);
    return;
}

bool xenax_xvi::in_motion() 
{
    bool ret;
    try {
        ret = this->get_status_register() & IN_MOTION;
    } catch (const device_error& ex) {
        if (ex.error_number() == 03)    // In motion (refer manual p. 34)
            return true;
        throw;
    }
    return ret;
}

bool xenax_xvi::in_position() 
{
    bool ret;
    try {
        ret = this->get_status_register() & IN_POSITION;
    } catch (const device_error& ex) {
        if (ex.error_number() == 03)    // In motion (refer manual p. 34)
            return false;
        throw;
    }
    return ret;
}

bool xenax_xvi::error_pending()
{
    // Reading the status register also updates m_error_pending
    this->get_status_register();
    return m_error_pending;
}

bool xenax_xvi::is_referenced() 
{
    return (this->get_status_register() & REF);
}

bool xenax_xvi::gantry_init() 
{
    return (this->get_status_register() & END_OF_GANTRY_INIT);
}

void xenax_xvi::set_speed(unsigned inc_per_sec) 
{
    this->query_cmd("SP" + to_string(inc_per_sec));
    return;
}

unsigned xenax_xvi::get_speed() 
{
    bool success = false;
    unsigned ret = convert_to<unsigned>(this->query_cmd("SP?"), success);
    if (!success)
        throw bad_protocol("Failed to get current speed");
    return ret; 
}

void xenax_xvi::set_acceleration(unsigned inc_per_sec2)
{
    this->query_cmd("AC" + to_string(inc_per_sec2));
    return;
}

unsigned xenax_xvi::get_acceleration() 
{ 
    bool success = false;
    unsigned ret = convert_to<unsigned>(this->query_cmd("AC?"), success);
    if (!success)
        throw bad_protocol("Failed to get current acceleration");
    return ret; 
}

void xenax_xvi::set_s_curve(unsigned percent) 
{
    if (percent > 100) {
        printf("Invalid S-curve percentage value %i\n", percent);
        abort();
    }
    this->query_cmd("SCRV" + to_string(percent));
    return;
}

unsigned xenax_xvi::get_s_curve() 
{ 
    bool success = false;
    unsigned ret = convert_to<unsigned>(this->query_cmd("SCRV?"), success);
    if (!success)
        throw bad_protocol("Failed to get S-curve value");
    return ret;  
}


void xenax_xvi::force_calibration(unsigned len) 
{
    debug_print("%s\n", "Performing force calibration...");
    this->query_cmd("FC" + to_string(len));
    this->wait_status_set(IN_MOTION, 200);
    this->wait_status_clr(FORCE_CALIBRATION_ACTIVE, 500);
    this->get_force_constant();
    return;
}

int xenax_xvi::get_motor_current() 
{ 
    bool success = false;
    unsigned ret = convert_to<unsigned>(this->query_cmd("TMC"), success);
    if (!success)
        throw bad_protocol("Failed to get S-curve value");
    return ret;
}


float xenax_xvi::get_force_constant() 
{
    bool success = false;
    m_force_const = 1e-6 * convert_to<int>(this->query_cmd("FCM?"), success);
    if (!success)
        throw bad_protocol("Failed to get force constant");
    debug_print("force constant = %f N/mA\n", m_force_const);
    return m_force_const;
}

float xenax_xvi::get_motor_force() 
{
    if ( !(this->get_status_register() & FORCE_CALIBRATION_ACTIVE) )
        throw device_error("No force calibration active, "
            "run force_calibration(len) first\n", -1);
    return m_force_const*this->get_motor_current();
}

void xenax_xvi::set_force_limit(float fmax_N) 
{
    int flim_10mA = int(0.1*fmax_N/m_force_const);
    this->query_cmd("LIF" + to_string(flim_10mA));
    return;
}

float xenax_xvi::get_force_limit() 
{
    bool success = false;
    int flim_10mA = convert_to<int>(this->query_cmd("LIF?"), success);
    if (!success)
        throw bad_protocol("Failed to get force limit");
    float flim = 10.*flim_10mA*m_force_const;
    debug_print("force limit = %.3f N\n", flim);
    return flim;
}

void xenax_xvi::set_output_type(unsigned output_no, output_type type) 
{
    if ( (output_no > 8) || (output_no < 1) ){
        fprintf(stderr, "GPIO output number has to be between 1 and 8.\n");
        abort();
    }
    // Clear bits for given gpio number
    m_output_type &= ~(0b11 << 2*(output_no-1));
    // Set bits corresponding to state
    m_output_type |= (type << 2*(output_no-1));
    this->set_output_type_reg(m_output_type);
    return;
}

void xenax_xvi::set_limits(unsigned left, unsigned right)
{
    this->query_cmd("LL" + to_string(left));
    this->query_cmd("LR" + to_string(right));
    return;
}

unsigned xenax_xvi::get_limit_left() 
{ 
    bool success = false;
    unsigned ret = convert_to<unsigned>(this->query_cmd("LL?"), success);
    if (!success)
        throw bad_protocol("Failed to get left limit");
    return ret;
}
unsigned xenax_xvi::get_limit_right() 
{ 
    bool success = false;
    unsigned ret = convert_to<unsigned>(this->query_cmd("LR?"), success);
    if (!success)
        throw bad_protocol("Failed to get right limit");
    return ret;
}

void xenax_xvi::set_output_activity(unsigned output_no, bool active_hi) 
{
    if ( (output_no > 8) || (output_no < 1) ){
        fprintf(stderr, "GPIO output number has to be between 1 and 8.\n");
        abort();
    }
    // Clear bits
    m_output_activity &= ~(0b1 << (output_no-1));
    // Set bits
    if (active_hi)
        m_output_activity |= (0b1 << (output_no-1));
    this->set_output_state_reg(m_output_activity);
    return;
}

void xenax_xvi::set_output(unsigned output_no, bool high)
{
    if ( (output_no > 8) || (output_no < 1) ){
        fprintf(stderr, "GPIO output number has to be between 1 and 8.\n");
        abort();
    }
    if (high)
        this->query_cmd("SO" + to_string(output_no));
    else 
        this->query_cmd("CO" + to_string(output_no));
    return;
}

bool xenax_xvi::get_output(unsigned output_no)
{
    if ( (output_no > 8) || (output_no < 1) ){
        fprintf(stderr, "GPIO output number has to be between 1 and 8.\n");
        abort();
    }
    uint8_t output_reg = this->get_output_state_reg();
    // Bits are left-aligned; output 1 = bit 7, output 8 = bit 0
    return ( (1 << (8 - output_no)) & output_reg );
}

bool xenax_xvi::get_input(unsigned input_no)
{
    if ( (input_no > 16) || (input_no < 1) ){
        fprintf(stderr, "GPIO input number has to be between 1 and 16.\n");
        abort();
    }
    uint16_t input_reg = this->get_input_state_reg();
    return ( (1 << (input_no - 1)) & input_reg );
}

void xenax_xvi::set_sid(std::string sid)
{
    if (sid.size() > 16) {
        fprintf(stderr, "SID '%s' exceeds max. length of 16 characters.\n",
            sid.c_str());
        abort();
    }
    this->query_cmd("SID" + sid);
    return;
}

void xenax_xvi::set_card_identifier(unsigned ci)
{
    if (ci > 4) {
        fprintf(stderr, "Invalid card identifier %u (valid 0 - 4)\n", ci);
        abort();
    }
    debug_print("Changing card identifier to %i\n", ci);
    this->query_cmd("CI" + to_string(ci));
    return;
}

unsigned xenax_xvi::get_card_identifier()
{
    bool success = false;
    unsigned ci = convert_to<unsigned>(this->query_cmd("CI?", success));
    if (!success)
        throw bad_protocol("Failed to get card identifier");
    debug_print("Read card identifier %i\n", ci);
    return ci;
}

void xenax_xvi::set_gantry_slave_id(unsigned gsid)
{
    debug_print("Changing grantry slave identifier to %i\n", gsid);
    this->query_cmd("GSID" + to_string(gsid));
    return;
}

unsigned xenax_xvi::get_gantry_slave_id()
{
    bool success = false;
    unsigned gsid = convert_to<unsigned>(this->query_cmd("GSID?"), success);
    if (!success)
        throw bad_protocol("Failed to get gantry slave identifier");
    debug_print("Read card identifier %i\n", gsid);
    return gsid;
}

void xenax_xvi::set_gantry_master_slave_offs(int gmso)
{
    debug_print("Setting gantry master slave offset to %i\n", gmso);
    this->query_cmd("PGMSO" + to_string(gmso));
    return;
}

int xenax_xvi::get_gantry_master_slave_offs()
{
    bool success = false;
    unsigned gmso = convert_to<unsigned>(this->query_cmd("PGMSO?"), success);
    if (!success)
        throw bad_protocol("Failed to get gantry master slave offset");
    debug_print("Read gantry master slave offset %i\n", gmso);
    return gmso;
}

int xenax_xvi::detected_gantry_master_slave_offs()
{
    bool success = false;
    unsigned dgmso = convert_to<unsigned>(this->query_cmd("DGMSO?"), success);
    if (!success)
        throw bad_protocol("Failed to get detected gantry master slave offset");
    debug_print("Read detected gantry master slave offset %i\n", dgmso);
    return dgmso;
}

unsigned xenax_xvi::get_error(std::string &strerror) 
{
    // Toggle error pending
    if (m_error_pending)
        m_error_pending = false;
    bool success = false;
    m_error = convert_to<int>(this->query_cmd("TE"), success);
    if (!success)
        throw bad_protocol("Failed to read error queue");
    strerror = this->query_cmd("TES");
    return m_error;
}

void xenax_xvi::set_payload(unsigned payload_g)
{
    debug_print("Setting payload to %u\n", payload_g);
    this->query_cmd("ML" + to_string(payload_g));
    return;
}

unsigned xenax_xvi::get_payload()
{
    bool success = false;
    unsigned payload = convert_to<int>(this->query_cmd("ML?"), success);
    if (!success)
        throw bad_protocol("Failed to read payload");
    debug_print("Read payload %u\n", payload);
    return payload;
}

void xenax_xvi::set_gain_pos(unsigned gain_pos)
{
    debug_print("Setting GAIN POS to %u\n", gain_pos);
    this->query_cmd("BWP" + to_string(gain_pos));
    return;
}

unsigned xenax_xvi::get_gain_pos()
{
    bool success = false;
    unsigned gain_pos = convert_to<int>(this->query_cmd("BWP?"), success);
    if (!success)
        throw bad_protocol("Failed to read GAIN POS");
    debug_print("Read GAIN POS %u\n", gain_pos);
    return gain_pos;
}

void xenax_xvi::set_gain_cur(unsigned gain_cur)
{
    debug_print("Setting GAIN CUR to %u\n", gain_cur);
    this->query_cmd("BWC" + to_string(gain_cur));
}

unsigned xenax_xvi::get_gain_cur()
{
    bool success = false;
    unsigned gain_cur = convert_to<int>(this->query_cmd("BWC?"), success);
    if (!success)
        throw bad_protocol("Failed to read GAIN POS");
    debug_print("Read GAIN CUR %u\n", gain_cur);
    return gain_cur;
}

void xenax_xvi::set_max_deviation(unsigned max_dev)
{
    debug_print("Setting Deviation POS ACT to %u\n", max_dev);
    this->query_cmd("DP" + to_string(max_dev));
    return;
}

unsigned xenax_xvi::get_max_deviation()
{
    bool success = false;
    unsigned max_dev = convert_to<int>(this->query_cmd("DP?"), success);
    if (!success)
        throw bad_protocol("Failed to read GAIN POS");
    debug_print("Read Deviation POS ACT %u\n", max_dev);
    return max_dev;
}

void xenax_xvi::set_target_deviation(unsigned tar_dev)
{
    debug_print("Setting Deviation TARGET to %u\n", tar_dev);
    this->query_cmd("DTP" + to_string(tar_dev));
    return;
}

unsigned xenax_xvi::get_target_deviation()
{
    bool success = false;
    unsigned tar_dev = convert_to<int>(this->query_cmd("DTP?"), success);
    if (!success)
        throw bad_protocol("Failed to read Deviation TARGET");
    debug_print("Read Deviation TARGET %u\n", tar_dev);
    return tar_dev;
}

tuple<unsigned,string> xenax_xvi::get_error()
{
    string error_str;
    unsigned error_no = this->get_error(error_str);
    return make_tuple<unsigned,string>(std::move(error_no), std::move(error_str));
}

/*
 *      P R I V A T E   M E T H O D S
 */

void xenax_xvi::init() 
{
    // Clear input buffer
    this->flush_buffer();
    // Disable asynchronous status updates
    this->query_cmd("EVT0");
    // Get force constant for force calculations
    this->get_force_constant();
    // Get servo identifier
    string servo_id = this->query_cmd("SID?");
    if (!servo_id.empty())
        m_dev_name += "," + servo_id;
    return;
}

void xenax_xvi::flush_buffer() 
{
    debug_print("%s\n", "flushing read buffer...");
    while (true) {
        try { get_comm()->read(200); }
        catch (const timeout &ex) { break; }
    }
    m_input_buffer.clear();
    debug_print("%s\n", "buffer flushed");
    return;
}

uint32_t xenax_xvi::get_status_register() 
{
    string resp = this->query_cmd("TPSR");
    uint32_t status = stoi(resp, 0 , 16);

    debug_print("status register = 0x%08X\n", status);

    // Check error, warning, and info bit => update error pending
    m_error_pending = (status & (ERROR | WARNING | INFO));

    return status;
}

string xenax_xvi::query_cmd(string cmd, unsigned timeout_ms) 
{
    // Send command and CR
    get_comm()->write(cmd + "\n");
    size_t pos;

    // Read until an EOM delimiter was received and store in buffer
    // (see application note 'TCP_IP_KOMMUNIKATION.pdf' p. 1)
    m_input_buffer.append( get_comm()->read_until(">", pos, timeout_ms) );

    // Split response into parameters and remove it from the buffer
    string resp = m_input_buffer.substr(0, pos);
    vector<string> par_list = split(resp, "\r\n", 10);
    m_input_buffer.erase(0, pos+1);

    #ifdef LD_DEBUG
    for (size_t i = 0; i < par_list.size(); i++) {
        debug_print("param (%zu/%zu): '%s'\n", i+1, par_list.size(),
            par_list.at(i).c_str());
    }
    #endif

    // First parameter should be cmd echo
    if ( par_list.at(0).find(cmd) == string::npos )
        throw bad_protocol("No command echo for '" + cmd + "' received");
    // Second parameter is the payload
    if (par_list.size() == 2) {
        if ( par_list.at(1).find("?") != string::npos)
            throw bad_protocol("Invalid command '" + cmd + "'\n");
        if ( par_list.at(1).find("#") != string::npos) {
            // Handle hash error message (refer manual p.34)
            string hash_str = par_list.at(1).substr(1);
            int hash_no = stoi(hash_str);
            switch (hash_no) {
                case 01:
                throw device_error("Cannot execute command, error in error "
                "queue (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 03:
                throw device_error("Cannot execute command, currently moving"
                " (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 05:
                throw device_error("Cannot execute command, program is active"
                " (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 13:
                throw device_error("Cannot execute command, emergency exit "
                "EE1 is pending (" + par_list.at(1) + ")\n", hash_no);

                case 14:
                throw device_error("Cannot execute command, emergency exit "
                "EE is pending (" + par_list.at(1) + ")\n", hash_no);

                case 15:
                throw device_error("Cannot execute command, force calibration "
                "is active (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 27:
                throw device_error("Cannot execute command, I Force Drift "
                "Compensation is active (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 34:
                throw device_error("Cannot execute command, rotation reference "
                "is active (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 36:
                throw device_error("Cannot execute command, gantry reference "
                "is active (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 38:
                throw device_error("Cannot execute command, reference is "
                "active (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 40:
                throw device_error("Cannot execute command, command not "
                "permitted (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 47:
                throw device_error("Cannot execute command, fault reaction "
                "active (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 49:
                throw device_error("Cannot execute command, no JSC motor "
                "connected (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 65:
                throw device_error("Cannot execute command, parameter out of "
                "value range (" + par_list.at(1) + ")\n", hash_no);
                break;

                case 66:
                throw device_error("Cannot execute command, 5s timeout occured "
                "(" + par_list.at(1) + ")\n", hash_no);
                break;

                default:
                throw device_error("Cannot execute command, unknown hash (" 
                + par_list.at(1) + ")\n", hash_no);
                break;
            }
        }

        return par_list.at(1);
    }

    return "";
}

void xenax_xvi::wait_status_set(uint32_t status, unsigned interval_ms,
unsigned timeout_ms) 
{
    // Setup timeout
    struct timeval tsta, tsto;
    gettimeofday(&tsta, NULL);
    float tdiff;
    uint32_t sreg = this->get_status_register();

    // Wait until all masked bits in status are set to '1'
    while ( (sreg & status) != status ) {
        usleep(interval_ms*1000);

        // Check for timeout
        gettimeofday(&tsto, NULL);
        tdiff = (tsto.tv_sec - tsta.tv_sec) * 1000.;
        if (tdiff > timeout_ms) {
            stringstream msg;
            msg << "Timeout on SREG bits to set" << endl;
            msg << "Expected 0x" << uppercase << setfill('0') << setw(8) 
                << hex << status << "to set" << endl; 
            msg << "Current  0x" << uppercase << setfill('0') << setw(8) 
                << hex << sreg << endl; 
            throw timeout(msg.str());
        }
        sreg = this->get_status_register();
    }

    return;
}

void xenax_xvi::wait_status_clr(uint32_t status, unsigned interval_ms,
unsigned timeout_ms) 
{
    // Setup timeout
    struct timeval tsta, tsto;
    gettimeofday(&tsta, NULL);
    float tdiff;
    uint32_t sreg = this->get_status_register();

    // Wait until all masked bits in status are cleared to '0'
    while ( ( sreg & status) ) {
        usleep(interval_ms*1000);

        // Check for timeout
        gettimeofday(&tsto, NULL);
        tdiff = (tsto.tv_sec - tsta.tv_sec) * 1000.;
        if (tdiff > timeout_ms) {
            stringstream msg;
            msg << "Timeout on SREG bits to clear" << endl;
            msg << "Expected 0x" << uppercase << setfill('0') << setw(8) 
                << hex << status  << "to clear" << endl; 
            msg << "Current  0x" << uppercase << setfill('0') << setw(8) 
                << hex << sreg << endl; 
            throw timeout(msg.str());
        }
        sreg = this->get_status_register();
    }

    return;
}

void xenax_xvi::set_output_type_reg(uint16_t mask) 
{
    debug_print("Setting output type to 0x%04X\n", mask);
    this->query_cmd("SOT" + to_string(mask));
    return;
}

void xenax_xvi::set_output_state_reg(uint8_t mask) 
{
    debug_print("Setting output state to 0x%02X\n", mask);
    this->query_cmd("SOA" + to_string(mask));
    return;
}

}
