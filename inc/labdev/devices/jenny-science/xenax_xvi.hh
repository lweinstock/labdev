#ifndef XENAX_XVI_HH
#define XENAX_XVI_HH

#include <labdev/devices/device.hh>
#include <labdev/serial_interface.hh>
#include <labdev/tcpip_interface.hh>

namespace labdev {

class xenax_xvi : public device {
public:
    xenax_xvi(const serial_config ser);
    xenax_xvi(const ip_address ip);

    // XENAX default port 10001
    static constexpr unsigned PORT = 10001;

    // En-/disable power of Xenax motor controller
    void power_on(bool enable = true) { this->query_cmd( enable? "PW" : "PQ"); }
    void power_off() { power_on(false); }
    void power_continue() { this->query_cmd("PWC"); }
    bool is_on() { return std::stoi(this->query_cmd("TS")); }

    // Referencing for absolute position measurements (see manual p.48)
    void reference_axis();
    void reference_axis(bool pos_dir);

    // Go to absolute position in micro meter
    void move_position(int pos) { this->query_cmd("G" + std::to_string(pos)); }
    int get_position() { return std::stoi(this->query_cmd("TP")); }
    // Move in positive/negative direction with constant speed
    void jog_pos() { this->query_cmd("JP"); }
    void jog_neg() { this->query_cmd("JN"); }
    void stop_motion();
    // Status queries
    bool in_motion();
    bool in_position();
    bool is_referenced();
    bool error_pending();

    // Set and get movement parameters;
    //   speed [inc/s], accel [inc/s2], s curve [%]
    void set_speed(unsigned inc_per_sec);
    unsigned get_speed() { return stoi(this->query_cmd("SP?")); }
    void set_acceleration(unsigned inc_per_sec2);
    unsigned get_acceleration() { return stoi(this->query_cmd("AC?")); }
    void set_s_curve(unsigned percent);
    unsigned get_s_curve() { return std::stoi(this->query_cmd("SCRV?")); }

    // Calibration for more precise force measurements (dF ~ 0.5 - 1.0 N)
    void force_calibration(unsigned distance);
    // Motor current and force information;
    //  current [mA], force const [N/mA], force [N]
    int get_motor_current() { return std::stoi(this->query_cmd("TMC")); }
    float get_force_constant();
    float get_motor_force();
    // Set force limit (manual p. 54)
    void set_force_limit(float fmax_N);
    float get_force_limit();
    bool force_limit_reached();

    // Set and get soft limits (left = min, right = max)
    void set_limits(unsigned left, unsigned right);
    unsigned get_limit_left() { return std::stoi(this->query_cmd("LL?")); }
    unsigned get_limit_right() { return std::stoi(this->query_cmd("LR?")); }

    enum output_type : uint8_t {
        SINK = 0b00,
        SOURCE = 0b01,
        SINK_SOURCE = 0b10
    };

    // Programmable Logic Controller (PLC) GPIO settings (manual p. 51ff)
    void set_output_type(unsigned output_no, output_type type);
    void set_output_activity(unsigned output_no, bool active_hi);
    void set_output(unsigned output_no, bool high);
    bool get_output(unsigned output_no);
    // Returns true when high, false when low
    bool get_input(unsigned input_no);

    // Motor type reset (in response to error 59)
    void reset_motor_type() { this->query_cmd("RESM", 10000); }
    // Disable motion blocked by unconfigured Safety Motion Unit (SMU)
    void disable_smu() { this->query_cmd("DMBUS"); }

    // Set custom servo identifier (max. 16 bytes)
    void set_sid(std::string sid);
    void reset_sid() { this->set_sid(""); }
    std::string get_sid() { return this->query_cmd("SID?"); }

    // Get error information (C- and python-style)
    unsigned get_error(std::string &strerror);
    std::tuple<unsigned,std::string> get_error();

private:
    // Private default ctor
    xenax_xvi();

    std::string m_input_buffer;
    float m_force_const;   // I->F conversion factor [N/mA]
    int m_error;
    uint16_t m_output_type;
    uint8_t m_output_activity;
    bool m_error_pending;

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

    void init();
    void flush_buffer();

    // General command query
    std::string query_cmd(std::string cmd, unsigned timeout_ms = 1000);

    // Read the Process Status Register (PSR) & update status
    uint32_t get_status_register();

    // Wait until status bits are set
    void wait_status_set(uint32_t status, unsigned interval_ms = 500,
        unsigned timeout_ms = 10000);

    // Wait until status bits are cleared
    void wait_status_clr(uint32_t status, unsigned interval_ms = 500,
        unsigned timeout_ms = 10000);

    // GPIO register access
    void set_output_type_reg(uint16_t mask);
    void set_output_state_reg(uint8_t mask);
    uint8_t get_output_state_reg() { return std::stoi(this->query_cmd("TO")); }
    uint16_t get_input_state_reg() { return std::stoi(this->query_cmd("TI")); }

    void read_error_queue();

};

}

#endif
