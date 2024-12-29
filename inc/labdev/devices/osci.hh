#ifndef LD_OSCI_HH
#define LD_OSCI_HH

#include <cstring>
#include <string>
#include <vector>

#include <labdev/devices/ld_device.hh>

namespace labdev{

/*
 *  Abstract base class for all oscis
 */
class osci : public ld_device {
public:
    osci(unsigned n_ch, std::string name = "?") : ld_device(name), m_n_ch(n_ch) {};
    virtual ~osci() {};

    // Returns maximum number of channels
    const unsigned get_n_channels() const { return m_n_ch; }

    // Turn channel on/off
    virtual void enable_channel(unsigned channel, bool enable = true) = 0;
    void disable_channel(unsigned channel) { enable_channel(channel, false); }
    virtual bool channel_enabled(unsigned channel) = 0;

    // Attenuation settings
    virtual void set_atten(unsigned channel, double att) = 0;
    virtual double get_atten(unsigned channel) = 0;

    // Vertical settings
    virtual void set_vert_base(unsigned channel, double volts_per_div) = 0;
    virtual double get_vert_base(unsigned channel) = 0;
    virtual void set_vert_offs(unsigned channel, double offset_v) = 0;
    virtual double get_vert_offs(unsigned channel) = 0;

    // Horizontal settings
    virtual void set_horz_base(double sec_per_div) = 0;
    virtual double get_horz_base() = 0;
    virtual void set_horz_offs(double offset_s) = 0;
    virtual double get_horz_offs() = 0;

    // Single and dual source measurements
    enum meas_item : unsigned {VMAX = 0, VMIN, VPP, VTOP, VBASE, VAMP, 
        VAVG, VRMS, OVERSHOOT, PRESHOOT, FREQ, RISETIME, FALLTIME, POS_WIDTH,
        NEG_WIDTH, POS_DUTY, NEG_DUTY, POS_DELAY, NEG_DELAY, POS_PHASE, 
        NEG_PHASE};
    virtual void set_meas(unsigned ch, meas_item meas) = 0;
    virtual double get_meas(unsigned ch, meas_item meas) = 0;
    virtual void set_meas(unsigned ch1, unsigned ch2, meas_item meas) = 0;
    virtual double get_meas(unsigned ch1, unsigned ch2, meas_item meas) = 0;
    virtual void clear_meas() = 0;

    // Acquisition settings
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual void single_shot() = 0;

    // Trigger settings
    enum trig_type : unsigned {RISE = 0, FALL, BOTH};
    virtual void set_trigger_type(trig_type trig) = 0;
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
    // Number of channels
    const unsigned m_n_ch;
};

}

#endif