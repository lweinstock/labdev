#ifndef FY6900_HH
#define FY6900_HH

#include <labdev/exceptions.hh>
#include <labdev/serial_interface.hh>

namespace labdev {

    /*
    *      Brief exception class for FY6900 function generator specific errors
    */

    class fy6900_exception : public exception {
    public:
        fy6900_exception(const std::string& msg, int err = 0):
            exception(msg, err) {};
        virtual ~fy6900_exception() {};
    };

    /*
    *      FeelTech FY6900 series function generator
    */

    class fy6900 {
    public:
        fy6900();
        fy6900(serial_interface* ser);
        ~fy6900();

        enum waveform : unsigned {
            SINE        = 0,
            SQUARE      = 1,
            RECTANGLE   = 2,
            TRAPEZOID   = 3,
            CMOS        = 4,
            PULSE       = 5,
            DC          = 6,
            TRIANGLE    = 7,
            POS_RAMP    = 8,
            NEG_RAMP    = 9
        };  // TODO: waveforms 10 to 99 (!!)

        void enable_ch(int ch, bool ena);
        bool get_status(int ch);

        void set_waveform(int ch, int wvfm);
        std::string get_waveform(int ch);

        void set_freq(int ch, float freq_hz);
        float get_freq(int ch);

        void set_duty_cycle(int ch, float dcl);
        float get_duty_cycle(int ch);

        void set_phase(int ch, float phase_deg);
        float get_phase(int ch);

        void set_ampl(int ch, float ampl_v);
        float get_ampl(int ch);

        void set_offset(int ch, float offset_v);
        float get_offset(int ch);

    private:
        interface* comm;

        // Send command and read response
        std::string query_cmd(std::string cmd);
        // Convert waveform number to string
        std::string wvfm_string(unsigned wvfm) const;

        // End of message character (0x0a)
        static constexpr const char EOM = '\n';
        // Timeout between read/write (10ms)
        static const unsigned SLEEP_US = 100e3;
    };
}

#endif