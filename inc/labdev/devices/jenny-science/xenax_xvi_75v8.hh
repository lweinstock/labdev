#ifndef XENAX_XVI_75V8_HH
#define XENAX_XVI_75V8_HH

#include <labdev/tcpip_interface.hh>
#include <labdev/serial_interface.hh>
#include <labdev/exceptions.hh>

#include <vector>

namespace labdev {

    class xenax_xvi_error : public exception {
    public:
        xenax_xvi_error(const std::string& msg, int err = 0):
            exception(msg, err) {};
        virtual ~xenax_xvi_error() {};
    };

    class xenax_xvi_warning : public exception {
    public:
        xenax_xvi_warning(const std::string& msg, int err = 0):
            exception(msg, err) {};
        virtual ~xenax_xvi_warning() {};
    };

    class xenax_xvi_info : public exception {
    public:
        xenax_xvi_info(const std::string& msg, int err = 0):
            exception(msg, err) {};
        virtual ~xenax_xvi_info() {};
    };

    class xenax_xvi_75v8 {
    public:
        xenax_xvi_75v8();
        xenax_xvi_75v8(tcpip_interface* tcpip);
        xenax_xvi_75v8(serial_interface* serial);

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

        // GPIO Set Output Type (SOT) definitions (manual p. 26)
        enum SOT : uint16_t {
            SOT00 = (1 << 0),
            SOT01 = (1 << 1),
            SOT10 = (1 << 2),
            SOT11 = (1 << 3),
            SOT20 = (1 << 4),
            SOT21 = (1 << 5),
            SOT30 = (1 << 6),
            SOT31 = (1 << 7),
            SOT40 = (1 << 8),
            SOT41 = (1 << 9),
            SOT50 = (1 << 10),
            SOT51 = (1 << 11),
            SOT60 = (1 << 12),
            SOT61 = (1 << 13),
            SOT70 = (1 << 14),
            SOT71 = (1 << 15)
        };

        // GPIO Set Output Activity (SOA) definitions (manual p. 26)
        enum SOA : uint8_t {
            SOA0 = (1 << 0),
            SOA1 = (1 << 1),
            SOA2 = (1 << 2),
            SOA3 = (1 << 3),
            SOA4 = (1 << 4),
            SOA5 = (1 << 5),
            SOA6 = (1 << 6),
            SOA7 = (1 << 7)
        };

        // En-/disable power of Xenax motor controller
        void set_power(bool enable);
        void power_on()  { set_power(true); }
        void power_off() { set_power(false); }
        void power_continue();

        // Referencing for absolute position measurements
        void reference_axis();
        bool is_referenced();

        // Go to absolute position in micro meter (non blocking & blocking)
        void move_position(int pos);
        void goto_position(int pos, unsigned interval_ms = 1000,
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
        void force_calibration(unsigned len);
        // Motor current and force information
        int get_motor_current();        // [mA]
        float get_force_constant();     // [N/mA]
        float get_motor_force();        // [N]
        // Set force limit (manual p. 54)
        void set_force_limit(float fmax_N);
        float get_force_limit();        // [N]
        bool force_limit_reached();

        // Programmable Logic Controller (PLC) GPIO settings (manual p. 51ff)
        void set_output_type(uint16_t mask);
        void set_output_state(uint8_t state);
        uint8_t get_output_state();
        uint16_t get_input_state();

        // Read the Process Status Register (PSR)
        uint32_t get_status_register();

        // Disable motion blocked by unconfigured Safety Motion Unit (SMU)
        void disable_SMU() { this->query_command("DMBUS"); }

        // Get error information
        unsigned get_error() { return m_error; }
        std::string get_strerror() { return m_strerror; }

        // Motor type reset (in response to error 59)
        void reset_motor_type() { this->query_command("RESM"); }

        std::string query_command(std::string cmd);

    private:
        interface* comm;
        std::string m_input_buffer, m_strerror;
        bool m_is_refrenced, m_in_motion, m_in_position, m_f_limit_reached;
        float m_force_const;   // I->F conversion factor [N/mA]
        int m_error;

        void init();
        void flush_buffer();

        // Read PSR and update members
        void update_status();

        // Wait until status bits are set
        void wait_status_set(uint32_t status, unsigned interval_ms = 500,
            unsigned timeout_ms = 10000);

        // Wait until status bits are cleared
        void wait_status_clr(uint32_t status, unsigned interval_ms = 500,
            unsigned timeout_ms = 10000);

        void read_error_queue();

    };
}

#endif