#include <labdev/devices/jenny-science/xenax_xvi_75v8.hh>
#include <labdev/exceptions.hh>
#include <labdev/utils/utils.hh>
#include "ld_debug.hh"

#include <unistd.h>
#include <sys/time.h>
#include <cmath>

namespace labdev {

    xenax_xvi_75v8::xenax_xvi_75v8():
        comm(nullptr),
        m_strerror(""),
        m_is_refrenced(false),
        m_in_motion(false),
        m_in_position(false),
        m_f_limit_reached(false),
        m_force_const(0),
        m_error(0) {
        return;
    }

    xenax_xvi_75v8::xenax_xvi_75v8(tcpip_interface* tcpip): xenax_xvi_75v8() {
        if (!tcpip) {
            fprintf(stderr, "Invalid interface pointer\n");
            abort();
        }
        if (!tcpip->connected()) {
            fprintf(stderr, "Interface not connected\n");
            abort();
        }
        comm = tcpip;
        this->init();
        return;
    }

    xenax_xvi_75v8::xenax_xvi_75v8(serial_interface* serial): xenax_xvi_75v8() {
        if (!serial) {
            fprintf(stderr, "Invalid interface pointer\n");
            abort();
        }
        if (!serial->connected()) {
            fprintf(stderr, "Interface not connected\n");
            abort();
        }
        // Serial setup: 115200 8N1 (manual p. 28)
        serial->set_baud(115200);
        serial->set_nbits(8);
        serial->set_parity(false);
        serial->set_stop_bits(1);
        serial->apply_settings();

        comm = serial;
        this->init();
        return;
    }

    void xenax_xvi_75v8::set_power(bool enable) {
        this->query_command( enable? "PW" : "PQ");
        return;
    }

    void xenax_xvi_75v8::power_continue() {
        this->query_command("PWC");
        return;
    }

    void xenax_xvi_75v8::reference_axis() {
        this->query_command("REF");
        // Wait until referencing is complete (max. 10s)
        this->wait_status_set(IN_MOTION);
        this->wait_status_set(IN_POSITION | REF);
        this->wait_status_clr(IN_MOTION);
        return;
    }

    bool xenax_xvi_75v8::is_referenced() {
        this->update_status();
        return m_is_refrenced;
    }

    bool xenax_xvi_75v8::force_limit_reached() {
        this->update_status();
        return m_f_limit_reached;
    }

    void xenax_xvi_75v8::move_position(int pos) {
        this->query_command("G" + std::to_string(pos));
        return;
    }

    void xenax_xvi_75v8::goto_position(int pos, unsigned interval_ms,
    unsigned timeout_ms) {
        this->move_position(pos);
        // First: check if the axis is moving
        int dx = abs(this->get_position() - pos);
        if (dx > 10)    // Perform first check, if distance is large enough!
            this->wait_status_set(IN_MOTION);
        // Second: check if the axis reached the position
        this->wait_status_set(IN_POSITION, interval_ms, timeout_ms);
        // Third: check if the axis is not moving anymore
        this->wait_status_clr(IN_MOTION, interval_ms, timeout_ms);
        return;
    }

    int xenax_xvi_75v8::get_position() {
        return std::stoi(this->query_command("TP"));
    }

    bool xenax_xvi_75v8::in_motion() {
        this->update_status();
        return m_in_motion;
    }

    bool xenax_xvi_75v8::in_position() {
        this->update_status();
        return m_in_position;
    }

    void xenax_xvi_75v8::jog_pos() {
        this->query_command("JP");
        return;
    }

    void xenax_xvi_75v8::jog_neg() {
        this->query_command("JN");
        return;
    }

    void xenax_xvi_75v8::stop_motion() {
        this->query_command("SM");
        this->wait_status_clr(IN_MOTION, 200);
        return;
    }

    void xenax_xvi_75v8::set_speed(unsigned inc_per_sec) {
        this->query_command("SP" + std::to_string(inc_per_sec));
        return;
    }

    unsigned xenax_xvi_75v8::get_speed() {
        return std::stoi( this-> query_command("SP?") );
    }

    void xenax_xvi_75v8::set_acceleration(unsigned inc_per_sec2) {
        this->query_command("AC" + std::to_string(inc_per_sec2));
        return;
    }

    unsigned xenax_xvi_75v8::get_acceleration() {
        return std::stoi( this-> query_command("AC?") );
    }

    void xenax_xvi_75v8::set_s_curve(unsigned percent) {
        if (percent > 100) {
            printf("Invalid S-curve percentage value %i\n", percent);
            abort();
        }
        this->query_command("SCRV" + std::to_string(percent));
        return;
    }

    unsigned xenax_xvi_75v8::get_s_curve() {
        return std::stoi( this-> query_command("SCRV?") );
    }

    void xenax_xvi_75v8::force_calibration(unsigned len) {
        debug_print("%s\n", "Performing force calibration...");
        this->query_command("FC" + std::to_string(len));
        this->wait_status_set(IN_MOTION, 200);
        this->wait_status_clr(FORCE_CALIBRATION_ACTIVE, 500);
        return;
    }

    int xenax_xvi_75v8::get_motor_current() {
        return std::stoi( this-> query_command("TMC") );
    }

    float xenax_xvi_75v8::get_force_constant() {
        std::string resp = this->query_command("FCM?");
        m_force_const = 1e-6 * std::stoi(resp);
        debug_print("force constant = %f N/mA\n", m_force_const);
        return m_force_const;
    }

    float xenax_xvi_75v8::get_motor_force() {
        return m_force_const*this->get_motor_current();
    }

    void xenax_xvi_75v8::set_force_limit(float fmax_N) {
        int flim_10mA = int(0.1*fmax_N/m_force_const);
        this->query_command("LIF" + std::to_string(flim_10mA));
        return;
    }

    float xenax_xvi_75v8::get_force_limit() {
        int flim_10mA = std::stoi( this->query_command("LIF?") );
        return float(10.*flim_10mA*m_force_const);
    }

    void xenax_xvi_75v8::set_output_type(uint16_t mask) {
        this->query_command("SOT" + std::to_string(mask));
        return;
    }

    void xenax_xvi_75v8::set_output_state(uint8_t state) {
        this->query_command("SOA" + std::to_string(state));
        return;
    }

    uint8_t xenax_xvi_75v8::get_output_state() {
        return std::stoi(this->query_command("TO"));
    }

    uint16_t xenax_xvi_75v8::get_input_state() {
        return std::stoi(this->query_command("TI"));
    }

    uint32_t xenax_xvi_75v8::get_status_register() {
        std::string resp = this->query_command("TPSR");
        uint32_t status = std::stoi(resp, 0 , 16);
        return status;
    }

    std::string xenax_xvi_75v8::query_command(std::string cmd) {
        // Send command and CR
        comm->write(cmd + "\n");
        size_t pos;

        // Read until an EOM delimiter was received and store in buffer
        // (see applicatio note 'TCP_IP_KOMMUNIKATION.pdf' p. 1)
        m_input_buffer.append( comm->read_until(">", pos) );

        // Split response into parameters and remove it from the buffer
        std::string resp = m_input_buffer.substr(0, pos);
        std::vector<std::string> par_list = split(resp, "\r\n", 10);
        m_input_buffer.erase(0, pos+1);

        #ifdef LD_DEBUG
        for (size_t i = 0; i < par_list.size(); i++) {
            debug_print("param (%zu/%zu): '%s'\n", i+1, par_list.size(),
                par_list.at(i).c_str());
        }
        #endif

        // First parameter should be cmd echo
        if ( par_list.at(0).find(cmd) == std::string::npos )
            throw bad_protocol("No command echo for '" + cmd + "' received");
        // Second parameter is the payload
        if (par_list.size() == 2) {
            if ( par_list.at(1).find("?") != std::string::npos)
                throw bad_protocol("Invalid command '" + cmd + "'\n");
            if ( par_list.at(1).find("#") != std::string::npos)
                throw device_error("Cannot execute command, device busy ("
                    + par_list.at(1) + ")\n");

            return par_list.at(1);
        }

        return "";
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

    void xenax_xvi_75v8::init() {
        // Clear input buffer
        this->flush_buffer();
        // Disable asynchronous status updates
        this->query_command("EVT0");
        // Get force constant for force calculations
        this->get_force_constant();
        // Update status
        //this->update_status();
        return;
    }

    void xenax_xvi_75v8::flush_buffer() {
        debug_print("%s\n", "flushing read buffer...");
        while (true) {
            try { comm->read(200); }
            catch (const timeout &ex) { break; }
        }
        m_input_buffer.clear();
        debug_print("%s\n", "buffer flushed");
        return;
    }

    void xenax_xvi_75v8::read_error_queue() {
        m_strerror = this->query_command("TES");
        m_error = std::stoi( this->query_command("TE") );
        return;
    }

    void xenax_xvi_75v8::update_status() {
        uint32_t status = this->get_status_register();
        debug_print("status register = 0x%08X\n", status);

        // Check error, warning, and info bit
        if ( status & (ERROR | WARNING | INFO) ) {
            this->read_error_queue();
            if (status & ERROR)
                throw xenax_xvi_error(m_strerror, m_error);
            else if (status & WARNING)  // Does WARNING need an exception...?
                throw xenax_xvi_warning(m_strerror, m_error);
            //else if (status & INFO)   // Does INFO need an exception...?
            //    throw xenax_xvi_info(m_strerror, m_error);
        }

        // Update general status info
        m_is_refrenced = status & REF;
        m_in_motion = status & IN_MOTION;
        m_in_position = status & IN_POSITION;
        m_f_limit_reached = status & I_FORCE_LIMIT_REACHED;

        return;
    }

    void xenax_xvi_75v8::wait_status_set(uint32_t status, unsigned interval_ms,
    unsigned timeout_ms) {
        // Setup timeout
        struct timeval tsta, tsto;
        gettimeofday(&tsta, NULL);
        float tdiff;

        // Wait until all bits in status are set to '1'
        while ( (this->get_status_register() & status) != status ) {
            usleep(interval_ms*1000);

            // Check for timeout
            gettimeofday(&tsto, NULL);
            tdiff = (tsto.tv_sec - tsta.tv_sec) * 1000.;
            if (tdiff > timeout_ms) {
                char buf[100];
                sprintf(buf, "status 0x%08X not set (0x%08X)", status,
                    this->get_status_register());
                throw timeout(buf);
            }
        }

        return;
    }

    void xenax_xvi_75v8::wait_status_clr(uint32_t status, unsigned interval_ms,
    unsigned timeout_ms) {
        // Setup timeout
        struct timeval tsta, tsto;
        gettimeofday(&tsta, NULL);
        float tdiff;

        // Wait until all bits in status are cleared to '0'
        while ( (this->get_status_register() & status) ) {
            usleep(interval_ms*1000);

            // Check for timeout
            gettimeofday(&tsto, NULL);
            tdiff = (tsto.tv_sec - tsta.tv_sec) * 1000.;
            if (tdiff > timeout_ms) {
                char buf[100];
                sprintf(buf, "status 0x%08X not cleared (0x%08X)", status,
                    this->get_status_register());
                throw timeout(buf);
            }
        }

        return;
    }

}