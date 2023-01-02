#ifndef LD_OSCILLOSCOPE_HH
#define LD_OSCILLOSCOPE_HH

#include <cstring>
#include <string>
#include <vector>

#include <labdev/devices/device.hh>

namespace labdev{

    /*
     *  Abstract base class for all oscilloscopes
     */

    class oscilloscope : public virtual device {
    public:
        oscilloscope(unsigned n_ch) : device(), m_n_ch(n_ch) {};
        virtual ~oscilloscope() {};

        // Generic trigger type definitions
        enum trigger_type : uint16_t {
            RISE    = 0x00,
            FALL    = 0x01,
            BOTH    = 0x02
        };

        // Returns maximum number of channels
        const int get_n_channels() const { return m_n_ch; }

        // Turn channel on/off
        virtual void enable_channel(unsigned channel, bool enable = true) = 0;
        void disable_channel(unsigned channel) { enable_channel(channel, false); }

        // Attenuation settings
        virtual void set_atten(unsigned channel, double att) = 0;
        virtual double get_atten(unsigned channel) = 0;

        // Vertical settings
        virtual void set_vert_base(unsigned channel, double volts_per_div) = 0;
        virtual double get_vert_base(unsigned channel) = 0;

        // Horizontal settings
        virtual void set_horz_base(double sec_per_div) = 0;
        virtual double get_horz_base() = 0;

        // Acquisition settings
        virtual void start_acquisition() = 0;
        virtual void stop_acquisition() = 0;
        virtual void single_shot() = 0;

        // Trigger settings
        virtual void set_trigger_type(trigger_type trig) = 0;
        virtual void set_trigger_level(double level) = 0;
        virtual void set_trigger_source(unsigned channel) = 0;

        // Returns true if trigger conditions have been met
        virtual bool triggered() = 0;
        // Returns true if data acquisition has stopped
        virtual bool stopped() = 0;

        // Read sample data
        virtual void read_sample_data(unsigned channel,  
            std::vector<double> &horz_data, std::vector<double> &vert_data) = 0;

    private:
        const unsigned m_n_ch;
    };
}

#endif