#ifndef DSO5000P_H
#define DSO5000P_H

#include <labdev/exceptions.hh>
#include <labdev/usb_interface.hh>
#include <labdev/devices/oscilloscope.hh>
#include <vector>

namespace labdev {

    /*
     *      Hantek DSO5000P series digital storage oszilloscope
     */

    class dso5000p : public oscilloscope {
    public:
        dso5000p();
        dso5000p(usb_interface* usb);
        ~dso5000p();

        // USB definitions
        static constexpr uint16_t VID = 0x049F;
        static constexpr uint16_t PID = 0x505A;

        // Turn channel on/off
        void enable_channel(unsigned channel, bool enable = true);

        // Attenuation settings
        void set_atten(unsigned channel, double att);
        double get_atten(unsigned channel);

        // Vertical settings
        void set_vert_base(unsigned channel, double volts_per_div);
        double get_vert_base(unsigned channel);

        // Horizontal settings
        void set_horz_base(double sec_per_div);
        double get_horz_base();

        // Acquisition settings
        void start_acquisition();
        void stop_acquisition();
        void single_shot();

        // Trigger settings
        void set_trigger_type(trigger_type trig);
        void set_trigger_level(double level);
        void set_trigger_source(unsigned channel);

        // Returns true if trigger conditions have been met
        bool triggered();
        // Returns true if data acquisition has stopped
        bool stopped();

        // Read sample data
        void read_sample_data(unsigned channel,  
            std::vector<double> &horz_data, std::vector<double> &vert_data);

        /*

        void beep();

        void lock_panel();
        void unlock_panel();

        void start_acquisition();
        void stop_acquisition();

        // Returns true if DSO is triggered
        bool triggered();

        // Set probe attenuation to 1, 10, 100, or 1000
        void set_probe_atten(int channel, int att);

        // Set vertical resolution in volts/div
        void set_vert_base(int channel, double volts_per_div);

        // Set horizontal resoluton in sec/dic
        void set_horz_base(double seconds_per_div);

        // Set edge triggering with given threshold and edge type
        // (rising, falling, or both)
        void set_edge_trig(int channel, double thresh,
            uint8_t edge = RISING_EDGE);

        // Reads sample waveform from DSO
        void read_sample_data(int channel, std::vector<double>& horz_data,
            std::vector<double>& vert_data);

        // Read file from DSO and save to destination (mainly for debugging...)
        void read_file(std::string src_path, std::string dst_path);

        */

        // Apply settings; DSO might require several seconds to apply settings
        // (depending on changes)
        void read_dso_settings();
        void apply_dso_settings();

    private:
        static const size_t MAX_BUF_SIZE = 1024*1024;   // 1MB buffer
        static const unsigned SLEEP_US = 100e3;     // 1ms sleep

        // Protocol definitions
        enum MSG : uint8_t {
            MSG_NORMAL = 0x53,
            MSG_DEBUG = 0x43
        };

        enum CMD : uint8_t {
            ECHO        = 0x00,
            READ_CFG    = 0x01,
            READ_SAMPLE = 0x02,
            READ_FILE   = 0x10,
            WRITE_CFG   = 0x11,
            DAQ         = 0x12,
            BUZZER      = 0x44,
            RESP_FLG    = 0x80
        };

        enum SUBCMD : uint8_t {
            STASTO  = 0x00,
            LOCK    = 0x01
        };

        // Trigger settings
        enum TRIG : uint8_t {
            RISING_EDGE     = 0x00,     // rising edge
            FALLING_EDGE    = 0x01,     // falling edge
            BOTH_EDGES      = 0x02,     // both rising and falling edges
        };

        // Trigger states
        enum TRIG_STATE : uint8_t {
            STOP        = 0x00,
            READY       = 0x01,
            AUTO        = 0x02,
            TRIGGERED   = 0x03,
            SCAN        = 0x04,
            ASTOP       = 0x05,
            ARMED       = 0x06
        };

        // Settings indizes (according to /protocol.inf)
        enum settings : int {
            VERT_CH1_DISP            = 0,
            VERT_CH1_VB              = 1,
            VERT_CH1_COUP            = 2,
            VERT_CH1_20MHZ           = 3,
            VERT_CH1_FINE            = 4,
            VERT_CH1_PROBE           = 5,
            VERT_CH1_RPHASE          = 6,
            VERT_CH1_CNT_FINE        = 7,
            VERT_CH1_POS             = 8,
            VERT_CH2_DISP            = 10,
            VERT_CH2_VB              = 11,
            VERT_CH2_COUP            = 12,
            VERT_CH2_20MHZ           = 13,
            VERT_CH2_FINE            = 14,
            VERT_CH2_PROBE           = 15,
            VERT_CH2_RPHASE          = 16,
            VERT_CH2_CNT_FINE        = 17,
            VERT_CH2_POS             = 18,
            TRIG_STATE               = 20,
            TRIG_TYPE                = 21,
            TRIG_SRC                 = 22,
            TRIG_MODE                = 23,
            TRIG_COUP                = 24,
            TRIG_VPOS                = 25,
            TRIG_FREQUENCY           = 27,
            TRIG_HOLDTIME_MIN        = 35,
            TRIG_HOLDTIME_MAX        = 43,
            TRIG_HOLDTIME            = 51,
            TRIG_EDGE_SLOPE          = 59,
            TRIG_VIDEO_NEG           = 60,
            TRIG_VIDEO_PAL           = 61,
            TRIG_VIDEO_SYN           = 62,
            TRIG_VIDEO_LINE          = 63,
            TRIG_PULSE_NEG           = 65,
            TRIG_PULSE_WHEN          = 66,
            TRIG_PULSE_TIME          = 67,
            TRIG_SLOPE_SET           = 75,
            TRIG_SLOPE_WIN           = 76,
            TRIG_SLOPE_WHEN          = 77,
            TRIG_SLOPE_V1            = 78,
            TRIG_SLOPE_V2            = 80,
            TRIG_SLOPE_TIME          = 82,
            TRIG_SWAP_CH1_TYPE       = 90,
            TRIG_SWAP_CH1_MODE       = 91,
            TRIG_SWAP_CH1_COUP       = 92,
            TRIG_SWAP_CH1_EDGE_SLOPE = 93,
            TRIG_SWAP_CH1_VIDEO_NEG  = 94,
            TRIG_SWAP_CH1_VIDEO_PAL  = 95,
            TRIG_SWAP_CH1_VIDEO_SYN  = 96,
            TRIG_SWAP_CH1_VIDEO_LINE = 97,
            TRIG_SWAP_CH1_PULSE_NEG  = 99,
            TRIG_SWAP_CH1_PULSE_WHEN = 100,
            TRIG_SWAP_CH1_PULSE_TIME = 101,
            TRIG_SWAP_CH1_SLOPE_SET  = 102,
            TRIG_SWAP_CH1_SLOPE_WIN  = 103,
            TRIG_SWAP_CH1_SLOPE_WHEN = 104,
            TRIG_SWAP_CH1_SLOPE_V1   = 105,
            TRIG_SWAP_CH1_SLOPE_V2   = 107,
            TRIG_SWAP_CH1_SLOPE_TIME = 109,
            TRIG_SWAP_CH2_TYPE       = 117,
            TRIG_SWAP_CH2_MODE       = 118,
            TRIG_SWAP_CH2_COUP       = 119,
            TRIG_SWAP_CH2_EDGE_SLOPE = 120,
            TRIG_SWAP_CH2_VIDEO_NEG  = 121,
            TRIG_SWAP_CH2_VIDEO_PAL  = 122,
            TRIG_SWAP_CH2_VIDEO_SYN  = 123,
            TRIG_SWAP_CH2_VIDEO_LINE = 124,
            TRIG_SWAP_CH2_PULSE_NEG  = 126,
            TRIG_SWAP_CH2_PULSE_WHEN = 127,
            TRIG_SWAP_CH2_PULSE_TIME = 128,
            TRIG_SWAP_CH2_SLOPE_SET  = 136,
            TRIG_SWAP_CH2_SLOPE_WIN  = 137,
            TRIG_SWAP_CH2_SLOPE_WHEN = 138,
            TRIG_SWAP_CH2_SLOPE_V1   = 139,
            TRIG_SWAP_CH2_SLOPE_V2   = 141,
            TRIG_SWAP_CH2_SLOPE_TIME = 143,
            TRIG_OVERTIME_NEG        = 151,
            TRIG_OVERTIME_TIME       = 152,
            HORIZ_TB                 = 160,
            HORIZ_WIN_TB             = 161,
            HORIZ_WIN_STATE          = 162,
            HORIZ_TRIGTIME           = 163,
            MATH_DISP                = 171,
            MATH_MODE                = 172,
            MATH_FFT_SRC             = 173,
            MATH_FFT_WIN             = 174,
            MATH_FFT_FACTOR          = 175,
            MATH_FFT_DB              = 176,
            DISPLAY_MODE             = 177,
            DISPLAY_PERSIST          = 178,
            DISPLAY_FORMAT           = 179,
            DISPLAY_CONTRAST         = 180,
            DISPLAY_MAXCONTRAST      = 181,
            DISPLAY_GRID_KIND        = 182,
            DISPLAY_GRID_BRIGHT      = 183,
            DISPLAY_MAXGRID_BRIGHT   = 184,
            ACQURIE_MODE             = 185,
            ACQURIE_AVG_CNT          = 186,
            ACQURIE_TYPE             = 187,
            ACQURIE_STORE_DEPTH      = 188,
            MEASURE_ITEM1_SRC        = 189,
            MEASURE_ITEM1            = 190,
            MEASURE_ITEM2_SRC        = 191,
            MEASURE_ITEM2            = 192,
            MEASURE_ITEM3_SRC        = 193,
            MEASURE_ITEM3            = 194,
            MEASURE_ITEM4_SRC        = 195,
            MEASURE_ITEM4            = 196,
            MEASURE_ITEM5_SRC        = 197,
            MEASURE_ITEM5            = 198,
            MEASURE_ITEM6_SRC        = 199,
            MEASURE_ITEM6            = 200,
            MEASURE_ITEM7_SRC        = 201,
            MEASURE_ITEM7            = 202,
            MEASURE_ITEM8_SRC        = 203,
            MEASURE_ITEM8            = 204,
            CONTROL_TYPE             = 205,
            CONTROL_MENUID           = 206,
            CONTROL_DISP_MENU        = 207,
            len
        };

        usb_interface* comm;    // Has only one interface: USB!

        // DSO vertical and horizontal settings
        uint8_t m_dso_settings[settings::len];
        double m_vert_vb[2], m_horiz_tb;
        int m_vert_probe[2];
        int64_t m_horiz_trigtime;
        uint16_t m_vert_pos[2];

        // Check if channel is valid
        void check_channel(unsigned channel);

        // Convert hex settings to values required for calculations
        double get_volts_per_div(uint8_t vb);
        double get_timebase(uint8_t tb);
        int get_probe_attenuation(uint8_t att);

        // Flushes the input buffer until read times out
        void flush_buffer();

        // Creates packet according to protocol, returns length of packet
        int create_pkt(uint8_t mark, uint8_t cmd, const uint8_t* data,
            size_t len, uint8_t* packet);

        // Extracts data from packet according to protocol, returns length of payload
        int extract_pkt(const uint8_t* packet, int pkt_len, uint8_t &mark,
            uint8_t &cmd, uint8_t* data);

        // Formats message according totocol and send data using bulk transfer
        void send_pkg(uint8_t mark, uint8_t cmd, const uint8_t* payload, int len);

        // Reads response, checks marker and command, and fills data into payload
        // returns length of payload
        int read_pkg(uint8_t mark, uint8_t cmd, uint8_t* payload,
            int timeout_ms = 1000);
    };
}

#endif