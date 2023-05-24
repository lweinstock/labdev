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

xenax_xvi::xenax_xvi(const serial_config ser): xenax_xvi() 
{
    // TODO: check serial setup: 115200 8N1 (manual p. 28)
    m_comm = std::make_shared<serial_interface>(ser);
    this->init();
    return;
}

xenax_xvi::xenax_xvi(const ip_address ip): xenax_xvi() 
{
    // Default port 10001
    if (ip.port != xenax_xvi::PORT) {
        fprintf(stderr, "XENAX Xvi 75v8 only supports port %u.\n",
            xenax_xvi::PORT);
        abort();
    }
    m_comm = std::make_shared<tcpip_interface>(ip);
    this->init();
    return;
}

void xenax_xvi::reference_axis() 
{
    debug_print("%s\n", "Referencing axis...");
    this->query_cmd("REF");
    // Wait until referencing is complete (max. 10s)
    this->wait_status_set(IN_MOTION);
    this->wait_status_set(IN_POSITION | REF);
    this->wait_status_clr(IN_MOTION);
    return;
}

void xenax_xvi::reference_axis(bool pos_dir) 
{
    debug_print("Changing reference direction to %s\n", 
        pos_dir ? "positive" : "negative");
    if (pos_dir)
        this->query_cmd("DRHR0");
    else 
        this->query_cmd("DRHR1");
    this->reference_axis();
    return;
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

void xenax_xvi::set_speed(unsigned inc_per_sec) 
{
    this->query_cmd("SP" + to_string(inc_per_sec));
    return;
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

void xenax_xvi::force_calibration(unsigned len) 
{
    debug_print("%s\n", "Performing force calibration...");
    this->query_cmd("FC" + to_string(len));
    this->wait_status_set(IN_MOTION, 200);
    this->wait_status_clr(FORCE_CALIBRATION_ACTIVE, 500);
    this->get_force_constant();
    return;
}

float xenax_xvi::get_force_constant() 
{
    string resp = this->query_cmd("FCM?");
    m_force_const = 1e-6 * stoi(resp);
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
    int flim_10mA = stoi( this->query_cmd("LIF?") );
    return float(10.*flim_10mA*m_force_const);
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

unsigned xenax_xvi::get_error(std::string &strerror) 
{
    // Toggle error pending
    if (m_error_pending)
        m_error_pending = false;
    m_error = stoi(this->query_cmd("TE"));
    strerror = this->query_cmd("TES");
    return m_error;
}

/*
 *      P R I V A T E   M E T H O D S
 */

xenax_xvi::xenax_xvi()
    : device("XENAX Xvi 75v8"), m_force_const(0), m_error(0),
      m_output_type(0x5555), m_output_activity(0xFF), m_error_pending(false)
{
    return;
}

void xenax_xvi::init() 
{
    // Clear input buffer
    this->flush_buffer();
    // Disable asynchronous status updates
    this->query_cmd("EVT0");
    // Get force constant for force calculations
    this->get_force_constant();
    return;
}

void xenax_xvi::flush_buffer() 
{
    debug_print("%s\n", "flushing read buffer...");
    while (true) {
        try { m_comm->read(200); }
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
    m_comm->write(cmd + "\n");
    size_t pos;

    // Read until an EOM delimiter was received and store in buffer
    // (see application note 'TCP_IP_KOMMUNIKATION.pdf' p. 1)
    m_input_buffer.append( m_comm->read_until(">", pos, timeout_ms) );

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
