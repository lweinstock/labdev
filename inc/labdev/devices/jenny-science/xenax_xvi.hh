#ifndef XENAX_XVI_HH
#define XENAX_XVI_HH

#include <labdev/devices/device.hh>
#include <labdev/serial_interface.hh>
#include <labdev/tcpip_interface.hh>

namespace labdev {

class xenax_xvi : public device {
public:
    xenax_xvi();
    xenax_xvi(serial_config &ser);
    xenax_xvi(ip_address &ip);
    xenax_xvi(const xenax_xvi&) = delete;

    void connect(serial_config &ser);
    void connect(ip_address &ip);

    // XENAX default port 10001
    static constexpr unsigned PORT = 10001;

    // Process Status Register definition (manual p. 56)
    enum PSR : uint32_t {
        ERROR                     = (1 << 0),
        REF                       = (1 << 1),
        IN_MOTION                 = (1 << 2),
        IN_POSITION               = (1 << 3),
        END_OF_PROGRAM            = (1 << 4),
        IN_FORCE                  = (1 << 5),
        IN_SECTO                  = (1 << 6),
        FORCE_IN_SECTOR           = (1 << 7),
        INVERTER_VOLTAGE          = (1 << 8),
        END_OF_GANTRY_IN          = (1 << 9),
        NEGATIVE_LIMIT_SWITC      = (1 << 10),
        POSITIVE_LIMIT_SWITC      = (1 << 11),
        REMAIN_POWER_ON           = (1 << 12),
        POWER_OFF                 = (1 << 13),
        FORCE_CALIBRATION_ACTIVE  = (1 << 14),
        I_FORCE_LIMIT_REACHED     = (1 << 15),
        STO_PRIMED_HIT            = (1 << 16),
        SS1_PRIMED_HIT            = (1 << 17),
        SS2_PRIMED                = (1 << 18),
        SS2_HIT                   = (1 << 19),
        SLS_PRIMED                = (1 << 20),
        SLS_SPEED_HIT             = (1 << 21),
        SLS_POSITION_HIT          = (1 << 22),
        WARNING                   = (1 << 23),
        INFO                      = (1 << 24),
        PHASING_DONE              = (1 << 25),
        I_FORCE_DRIFT_COMP_ACTIVE = (1 << 26)
    };

    enum output_type : uint8_t {
        SINK = 0b00,
        SOURCE = 0b01,
        SINK_SOURCE = 0b10
    };

    enum output_activity : uint8_t {
        ACTIVE_LOW = 0b0,
        ACTIVE_HIGH = 0b1
    };

    enum io_state : uint8_t {
        LOW = 0b0,
        HIGH = 0b1
    };

    // En-/disable power of Xenax motor controller
    void power_on(bool enable = true);
    void power_off() { power_on(false); }
    void power_continue();

    // Referencing for absolute position measurements (see manual p.48)
    void reference_axis();
    void reference_axis(bool pos_dir);
    bool is_referenced();

    // Go to absolute position in micro meter (non blocking & blocking)
    void move_position(int pos);
    void goto_position(int pos, unsigned interval_ms = 500,
        unsigned timeout_ms = 10000);
    int get_position();
    bool in_motion();
    bool in_position();
    // Move in positive/negative direction with constant speed
    void jog_pos();
    void jog_neg();
    void stop_motion();

    // Set and get movement parameters
    void set_speed(unsigned inc_per_sec);
    unsigned get_speed();           // [inc/s]
    void set_acceleration(unsigned inc_per_sec2);
    unsigned get_acceleration();    // [inc/s2]
    void set_s_curve(unsigned percent);
    unsigned get_s_curve();         // [%]

    // Calibration for more precise force measurements (dF ~ 0.5 - 1.0 N)
    void force_calibration(unsigned distance);
    // Motor current and force information
    int get_motor_current();        // [mA]
    float get_force_constant();     // [N/mA]
    float get_motor_force();        // [N]
    // Set force limit (manual p. 54)
    void set_force_limit(float fmax_N);
    float get_force_limit();        // [N]
    bool force_limit_reached();

    // Programmable Logic Controller (PLC) GPIO settings (manual p. 51ff)
    void set_output_type(unsigned output_no, output_type type);
    void set_output_activity(unsigned output_no, output_activity act);
    void set_output(unsigned output_no, io_state state);
    io_state get_input(unsigned input_no);

    // Read the Process Status Register (PSR) & update status
    uint32_t get_status_register();

    // Disable motion blocked by unconfigured Safety Motion Unit (SMU)
    void disable_smu() { this->query_command("DMBUS"); }

    // Get error information 
    //   COMMENT: better catch exception and use ex.error_numer() and ex.what()
    //   but then again, when are these methods to be used? Better private??
    unsigned get_error() { return std::stoi(this->query_command("TE")); }
    std::string get_strerror() { return this->query_command("TES"); }

    // Motor type reset (in response to error 59)
    void reset_motor_type() { this->query_command("RESM", 10000); }

    std::string query_command(std::string cmd, unsigned timeout_ms = 1000);

private:
    std::string m_input_buffer, m_strerror;
    float m_force_const;   // I->F conversion factor [N/mA]
    int m_error;
    uint16_t m_output_type;
    uint8_t m_output_activity;

    // GPIO Set Output Type (SOT) definitions (manual p. 26)
    enum SOT : uint16_t {
        SOT10 = (1 << 0),
        SOT11 = (1 << 1),
        SOT20 = (1 << 2),
        SOT21 = (1 << 3),
        SOT30 = (1 << 4),
        SOT31 = (1 << 5),
        SOT40 = (1 << 6),
        SOT41 = (1 << 7),
        SOT50 = (1 << 8),
        SOT51 = (1 << 9),
        SOT60 = (1 << 10),
        SOT61 = (1 << 11),
        SOT70 = (1 << 12),
        SOT71 = (1 << 13),
        SOT80 = (1 << 14),
        SOT81 = (1 << 15)
    };

    // GPIO Set Output Activity (SOA) definitions (manual p. 26)
    enum SOA : uint8_t {
        SOA1 = (1 << 0),
        SOA2 = (1 << 1),
        SOA3 = (1 << 2),
        SOA4 = (1 << 3),
        SOA5 = (1 << 4),
        SOA6 = (1 << 5),
        SOA7 = (1 << 6),
        SOA8 = (1 << 7)
    };

    void init();
    void flush_buffer();

    // Wait until status bits are set
    void wait_status_set(uint32_t status, unsigned interval_ms = 500,
        unsigned timeout_ms = 10000);

    // Wait until status bits are cleared
    void wait_status_clr(uint32_t status, unsigned interval_ms = 500,
        unsigned timeout_ms = 10000);

    // GPIO register access
    void set_output_type_reg(uint16_t mask);
    void set_output_state_reg(uint8_t mask);
    uint8_t get_output_state_reg();
    uint16_t get_input_state_reg();

    void read_error_queue();

};

}

#endif
