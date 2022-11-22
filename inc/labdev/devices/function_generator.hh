#ifndef LD_FUN_GEN_HH
#define LD_FUN_GEN_HH

#include <cstring>
#include <string>
#include <vector>

#include <labdev/devices/device.hh>

namespace labdev{

    /*
     *  Abstract base class for all function generators
     */

    class function_generator : public virtual device {
    public:
        function_generator(unsigned n_ch) : m_n_ch(n_ch) {};
        virtual ~function_generator() {};

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
        const unsigned m_n_ch;
    };
}

#endif