#ifndef DPO5000B_H
#define DPO5000B_H

#include <labdev/exceptions.hh>
#include <labdev/devices/scpi_device.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/visa_interface.hh>
#include <labdev/usbtmc_interface.hh>

// Edge trigger settings
#define DPO5000B_EDGE_RISE  0x00
#define DPO5000B_EDGE_FALL  0x01
#define DPO5000B_EDGE_BOTH  0x02

namespace labdev {

    /*
    *      Tektronix DPO5000B series oscilloscope
    */

    class dpo5000b : public scpi_device {
    public:
        dpo5000b();
        dpo5000b(visa_interface* visa);
        dpo5000b(tcpip_interface* tcpip);
        dpo5000b(usbtmc_interface* usbtmc);
        ~dpo5000b();

        static constexpr uint16_t DPO5204B_VID = 0x0699;
        static constexpr uint16_t DPO5204B_PID = 0x0503;

        enum trigger_edge : uint16_t {RISE = 0x00, FALL = 0x01, BOTH = 0x02};

        void enable_channel(unsigned channel, bool enable = true);

        void start_acquisition();
        void stop_acquisition();

        void set_edge_trigger(unsigned channel, double level, uint8_t egde = RISE);
        bool triggered();

        void set_sample_rate(int samples_per_sec);
        int get_sample_rate();

        // Set length of sample to be read from memory
        void set_sample_len(int rec_len);
        int get_sample_len();
        // Set sample to match the data shown on screen (between the cursors)
        void sample_screen();

        // Read sample data from internal memory
        void read_sample_data(unsigned channel, std::vector<double> &horz_data,
            std::vector<double> &vert_data);

        void set_vert_base(unsigned channel, double volts_per_div);
        double get_vert_base(unsigned channel);

        void set_horz_base(double sec_per_div);
        double get_horz_base();

    private:
        double m_xincr, m_xzero, m_xoff, m_ymult, m_yzero, m_yoff;
        int m_nbyte, m_nbits, m_npts;

        void init();
        void check_channel(unsigned channel);
    };
}

#endif