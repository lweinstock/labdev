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

xenax_xvi::xenax_xvi()
    : device("XENAX Xvi 75v8"), m_strerror(""), m_force_const(0), m_error(0),
      m_output_type(0x5555), m_output_activity(0xFF)
{
    return;
}

xenax_xvi::xenax_xvi(serial_config &ser): xenax_xvi() 
{
    this->connect(ser);
    this->init();
    return;
}

xenax_xvi::xenax_xvi(ip_address &ip): xenax_xvi() 
{
    this->connect(ip);
    this->init();
    return;
}

void xenax_xvi::connect(serial_config &ser) 
{
    if ( this->connected() ) {
        fprintf(stderr, "Device is already connected!\n");
        abort();
    }
    // Serial setup: 115200 8N1 (manual p. 28)
    ser.baud = 115200;
    ser.nbits = 8;
    ser.par_ena = false;
    ser.stop_bits = 1;
    m_comm = new serial_interface(ser);
    return;
}

void xenax_xvi::connect(ip_address &ip) 
{
    if ( this->connected() ) {
        fprintf(stderr, "Device is already connected!\n");
        abort();
    }
    // Default port 10001
    if (ip.port != xenax_xvi::PORT) {
        fprintf(stderr, "XENAX Xvi 75v8 only supports port %u.\n",
            xenax_xvi::PORT);
        abort();
    }
    m_comm = new tcpip_interface(ip);
    return;
}

void xenax_xvi::power_on(bool enable) 
{
    this->query_command( enable? "PW" : "PQ");
    return;
}

void xenax_xvi::power_continue() 
{
    this->query_command("PWC");
    return;
}

void xenax_xvi::reference_axis() 
{
    debug_print("%s\n", "Referencing axis...");
    this->query_command("REF");
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
        this->query_command("DRHR0");
    else 
        this->query_command("DRHR1");
    this->reference_axis();
    return;
}

bool xenax_xvi::is_referenced() 
{
    return (this->get_status_register() & REF);
}

bool xenax_xvi::force_limit_reached() 
{
    return (this->get_status_register() & I_FORCE_LIMIT_REACHED);
}

void xenax_xvi::move_position(int pos) 
{
    this->query_command("G" + to_string(pos));
    return;
}

void xenax_xvi::goto_position(int pos, unsigned interval_ms,
unsigned timeout_ms) 
{
    this->move_position(pos);

    /*    
    // First: check if the axis is moving
    int dx = abs(this->get_position() - pos);
    if (dx > 10)    // Perform first check, if distance is large enough!
        this->wait_status_set(IN_MOTION);
    */

    // Alternative: Wait for 100ms
    usleep(100e3);

    // Second: check if the axis reached the position
    this->wait_status_set(IN_POSITION, interval_ms, timeout_ms);
    // Third: check if the axis is not moving anymore
    this->wait_status_clr(IN_MOTION, interval_ms, timeout_ms);
    return;
}

int xenax_xvi::get_position() 
{
    return stoi(this->query_command("TP"));
}

bool xenax_xvi::in_motion() 
{
    return (this->get_status_register() & IN_MOTION);
}

bool xenax_xvi::in_position() 
{
    return (this->get_status_register() & IN_POSITION);
}

void xenax_xvi::jog_pos() 
{
    this->query_command("JP");
    return;
}

void xenax_xvi::jog_neg() 
{
    this->query_command("JN");
    return;
}

void xenax_xvi::stop_motion() 
{
    this->query_command("SM");
    this->wait_status_clr(IN_MOTION, 200);
    return;
}

void xenax_xvi::set_speed(unsigned inc_per_sec) 
{
    this->query_command("SP" + to_string(inc_per_sec));
    return;
}

unsigned xenax_xvi::get_speed() 
{
    return stoi( this-> query_command("SP?") );
}

void xenax_xvi::set_acceleration(unsigned inc_per_sec2) 
{
    this->query_command("AC" + to_string(inc_per_sec2));
    return;
}

unsigned xenax_xvi::get_acceleration() 
{
    return stoi( this-> query_command("AC?") );
}

void xenax_xvi::set_s_curve(unsigned percent) 
{
    if (percent > 100) {
        printf("Invalid S-curve percentage value %i\n", percent);
        abort();
    }
    this->query_command("SCRV" + to_string(percent));
    return;
}

unsigned xenax_xvi::get_s_curve() 
{
    return stoi( this-> query_command("SCRV?") );
}

void xenax_xvi::force_calibration(unsigned len) 
{
    debug_print("%s\n", "Performing force calibration...");
    this->query_command("FC" + to_string(len));
    this->wait_status_set(IN_MOTION, 200);
    this->wait_status_clr(FORCE_CALIBRATION_ACTIVE, 500);
    this->get_force_constant();
    return;
}

int xenax_xvi::get_motor_current() 
{
    return stoi( this-> query_command("TMC") );
}

float xenax_xvi::get_force_constant() 
{
    string resp = this->query_command("FCM?");
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
    this->query_command("LIF" + to_string(flim_10mA));
    return;
}

float xenax_xvi::get_force_limit() 
{
    int flim_10mA = stoi( this->query_command("LIF?") );
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

void xenax_xvi::set_output_activity(unsigned output_no, output_activity act) 
{
    if ( (output_no > 8) || (output_no < 1) ){
        fprintf(stderr, "GPIO output number has to be between 1 and 8.\n");
        abort();
    }
    // Clear bits
    m_output_activity &= ~(0b1 << (output_no-1));
    // Set bits
    m_output_activity |= (act << (output_no-1));
    this->set_output_state_reg(m_output_activity);
    return;
}

void xenax_xvi::set_output(unsigned output_no, io_state state)
{
    if ( (output_no > 8) || (output_no < 1) ){
        fprintf(stderr, "GPIO output number has to be between 1 and 8.\n");
        abort();
    }
    if (state == HIGH)
        this->query_command("SO" + to_string(output_no));
    else 
        this->query_command("CO" + to_string(output_no));
    return;
}

xenax_xvi::io_state xenax_xvi::get_input(unsigned input_no)
{
    if ( (input_no > 16) || (input_no < 1) ){
        fprintf(stderr, "GPIO input number has to be between 1 and 16.\n");
        abort();
    }
    uint16_t input_reg = this->get_input_state_reg();
    if ( (1 << (input_no-1)) & input_reg )
        return HIGH;
    return LOW;
}

uint32_t xenax_xvi::get_status_register() 
{
    string resp = this->query_command("TPSR");
    uint32_t status = stoi(resp, 0 , 16);

    debug_print("status register = 0x%08X\n", status);

    // Check error, warning, and info bit
    if ( status & (ERROR | WARNING | INFO) ) {
        m_error = this->get_error();
        m_strerror = this->get_strerror();
        if (status & ERROR)
            throw device_error(m_strerror, m_error);
        else if (status & WARNING)  // Does WARNING need an exception...?
            throw device_error(m_strerror, m_error);
        else if (status & INFO)     // Does INFO need an exception...?
            throw device_error(m_strerror, m_error);
    }

    return status;
}

string xenax_xvi::query_command(string cmd, unsigned timeout_ms) 
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
                "queue (" + par_list.at(1) + ")\n");
                break;

                case 03:
                throw device_error("Cannot execute command, currently moving"
                " (" + par_list.at(1) + ")\n");
                break;

                case 05:
                throw device_error("Cannot execute command, program is active"
                " (" + par_list.at(1) + ")\n");
                break;

                case 13:
                throw device_error("Cannot execute command, emergency exit "
                "EE1 is pending (" + par_list.at(1) + ")\n");

                case 14:
                throw device_error("Cannot execute command, emergency exit "
                "EE is pending (" + par_list.at(1) + ")\n");

                case 15:
                throw device_error("Cannot execute command, force calibration "
                "is active (" + par_list.at(1) + ")\n");
                break;

                case 27:
                throw device_error("Cannot execute command, I Force Drift "
                "Compensation is active (" + par_list.at(1) + ")\n");
                break;

                case 34:
                throw device_error("Cannot execute command, rotation reference "
                "is active (" + par_list.at(1) + ")\n");
                break;

                case 36:
                throw device_error("Cannot execute command, gantry reference "
                "is active (" + par_list.at(1) + ")\n");
                break;

                case 38:
                throw device_error("Cannot execute command, reference is "
                "active (" + par_list.at(1) + ")\n");
                break;

                case 40:
                throw device_error("Cannot execute command, command not "
                "permitted (" + par_list.at(1) + ")\n");
                break;

                case 47:
                throw device_error("Cannot execute command, fault reaction "
                "active (" + par_list.at(1) + ")\n");
                break;

                case 49:
                throw device_error("Cannot execute command, no JSC motor "
                "connected (" + par_list.at(1) + ")\n");
                break;

                case 65:
                throw device_error("Cannot execute command, parameter out of "
                "value range (" + par_list.at(1) + ")\n");
                break;

                case 66:
                throw device_error("Cannot execute command, 5s timeout occured "
                "(" + par_list.at(1) + ")\n");
                break;

                default:
                throw device_error("Cannot execute command, unknown hash (" 
                + par_list.at(1) + ")\n");
                break;
            }
        }

        return par_list.at(1);
    }

    return "";
}

/*
    *      P R I V A T E   M E T H O D S
    */

void xenax_xvi::init() 
{
    // Clear input buffer
    this->flush_buffer();
    // Disable asynchronous status updates
    this->query_command("EVT0");
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
    this->query_command("SOT" + to_string(mask));
    return;
}

void xenax_xvi::set_output_state_reg(uint8_t mask) 
{
    debug_print("Setting output state to 0x%02X\n", mask);
    this->query_command("SOA" + to_string(mask));
    return;
}

uint8_t xenax_xvi::get_output_state_reg() 
{
    return stoi(this->query_command("TO"));
}

uint16_t xenax_xvi::get_input_state_reg() 
{
    return stoi(this->query_command("TI"));
}

}
