#ifndef DG4000_H
#define DG4000_H

#include <labdev/tcpip_interface.hh>
#include <labdev/visa_interface.hh>
#include <labdev/usbtmc_interface.hh>
#include <labdev/devices/scpi_device.hh>

namespace labdev {

    /*
     *      Rigol DG4000 series function generator
     */

    class dg4000 : public scpi_device {
    public:
        dg4000();
        dg4000(tcpip_interface* tcpip);
        dg4000(visa_interface* visa);
        dg4000(usbtmc_interface* usbtmc);
        ~dg4000();

        static constexpr uint16_t DG4162_VID = 0x1AB1;
        static constexpr uint16_t DG4162_PID = 0x0641;

        // Waveform definitions
        enum waveform : unsigned {SINE, SQUARE, RAMP, PULSE, NOISE,
            ARBITRARY, HARMONIC, USER, size};
        static const std::string waveform_string[];

        // En-/disables output of given channel
        void enable_channel(unsigned channel, bool enable = true);

        void set_waveform(unsigned channel, unsigned wvfm);
        void set_freq(unsigned channel, double freq_hz);
        void set_duty_cycle(unsigned channel, double dcl);
        void set_phase(unsigned channel, double phase_deg);
        void set_ampl(unsigned channel, double ampl_v);
        void set_offset(unsigned channel, double offset_v);

        std::string get_waveform(int channel);
        double get_freq(unsigned channel);
        double get_duty_cycle(unsigned channel);
        double get_phase(unsigned channel);
        double get_ampl(unsigned channel);
        double get_offset(unsigned channel);

        // Returns when at least time_ms have passed after sending msg
        void write_at_least(std::string msg, unsigned time_ms);

    private:
        void init();

        // Aborts if channel is invalid
        void check_channel(unsigned channel);

    };

}

#endif