#ifndef LD_RTA4000_HH
#define LD_RTA4000_HH

#include <labdev/exceptions.hh>

#include <labdev/tcpip_interface.hh>
#include <labdev/visa_interface.hh>
#include <labdev/usbtmc_interface.hh>
#include <labdev/serial_interface.hh>

#include <labdev/devices/oscilloscope.hh>
#include <labdev/devices/scpi_device.hh>

namespace labdev{

    /*
     *      Rohde & Schwarz RTA4000 series oscilloscope
     */

    class rta4000 : public oscilloscope, public scpi_device {
    public:
        rta4000();
        rta4000(tcpip_interface* tcpip);
        rta4000(visa_interface* visa);
        rta4000(usbtmc_interface* usbtmc);
        rta4000(serial_interface* serial);
        ~rta4000() {};

        static constexpr uint16_t RTA4004_VID = 0x0AAD;
        static constexpr uint16_t RTA4004_PID = 0x01D6;

        /* Definition of generic oscilloscope functions */

        // Turn channel on/off
        void enable_channel(unsigned channel, bool enable = true) override;

        // Attenuation settings
        void set_atten(unsigned channel, double att) override;
        double get_atten(unsigned channel) override;

        // Vertical settings
        void set_vert_base(unsigned channel, double volts_per_div) override;
        double get_vert_base(unsigned channel) override;

        // Horizontal settings
        void set_horz_base(double sec_per_div) override;
        double get_horz_base() override;

        // Acquisition settings
        void start_acquisition() override;
        void stop_acquisition() override;
        void single_shot() override;

        // Trigger settings
        void set_trigger_type(trigger_type trig) override;
        void set_trigger_level(double level) override;
        void set_trigger_source(unsigned channel) override;

        // Returns true if trigger conditions have been met
        bool triggered() override;
        // Returns true if data acquisition has stopped
        bool stopped() override;

        // Read sample data
        void read_sample_data(unsigned channel, std::vector<double> &horz_data, 
            std::vector<double> &vert_data) override;

        /* Definition of RTA4000 series specific functions */

    private:
        void init();
        void check_channel(unsigned channel);

        unsigned m_npts;
        double m_xincr, m_xorg, m_xref, m_yinc;
        int m_yorg, m_yref;

    };
}

#endif