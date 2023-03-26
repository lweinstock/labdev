#ifndef FY6900_HH
#define FY6900_HH

#include <labdev/exceptions.hh>
#include <labdev/serial_interface.hh>
#include <labdev/devices/fgen.hh>

namespace labdev {

    /*
    *      FeelTech FY6900 series function generator
    */

    class fy6900 : public fgen {
    public:
        fy6900();
        fy6900(serial_config &ser);
        ~fy6900() {};

        void connect(serial_config &ser);

        static constexpr unsigned BAUD = 115200;

        /* Definition of generic function generator methods */

        // Turn channel on/off
        void enable_channel(unsigned channel, bool ena = true);
        // Get output state of channel (true = on, false = off)
        bool get_state(unsigned channel);

        // Signal waveform 
        void set_waveform(unsigned channel, fgen::waveform wvfm);
        fgen::waveform get_waveform(unsigned channel);

        // Signal frequency 
        void set_freq(unsigned channel, float freq_hz);
        float get_freq(unsigned channel);

        // Signal duty cycle
        void set_duty_cycle(unsigned channel, float dcl);
        float get_duty_cycle(unsigned channel);

        // Signal phase in degree
        void set_phase(unsigned channel, float phase_deg);
        float get_phase(unsigned channel);

        // Signal amplitude in V
        void set_ampl(unsigned channel, float ampl_v);
        float get_ampl(unsigned channel);

        // Signal offset in V
        void set_offset(unsigned channel, float offset_v);
        float get_offset(unsigned channel);

    private:
        // End of message character (0x0a)
        static constexpr const char EOM = '\n';
        // Timeout between read/write (10ms)
        static const unsigned SLEEP_US = 100e3;

        // Send command and read response
        std::string query_cmd(std::string cmd);
        
        // Convert number to wave type and vice versa
        unsigned wvfm_to_int(fgen::waveform wvfm);
        fgen::waveform int_to_wvfm(unsigned iwvfm);
    };
}

#endif