#include <labdev/devices/hantek/dso5000p.hh>
#include "ld_debug.hh"

#include <unistd.h>
#include <math.h>
#include <fstream>

namespace labdev {

    dso5000p::dso5000p () : oscilloscope(2), m_vert_vb{0,0},  m_horiz_tb(0), 
    m_vert_probe{0,0}, m_horiz_trigtime(0), m_vert_pos{0,0} {
        return;
    }

    dso5000p::dso5000p (usb_interface* usb) : dso5000p() {
        if (!usb) {
            fprintf(stderr, "invalid interface\n");
            abort();
        }
        if (!usb->connected()) {
            fprintf(stderr, "interface not connected\n");
            abort();
        }
        // USB setup
        usb->claim_interface(0);
        usb->set_endpoint_in(0x01);
        usb->set_endpoint_out(0x02);
        comm = usb;

        // General initialization
        this->flush_buffer();
        usleep(SLEEP_US);
        this->read_dso_settings();
        return;
    }

    dso5000p::~dso5000p () {
        return;
    };

    void dso5000p::enable_channel(unsigned channel, bool enable) {
        this->check_channel(channel);
        if (channel == 1) 
            m_dso_settings[VERT_CH1_DISP] = (enable ? 0x01 : 0x00);
        else 
            m_dso_settings[VERT_CH2_DISP] = (enable ? 0x01 : 0x00);
        this->apply_dso_settings();
        return;
    }

    void dso5000p::set_atten(unsigned channel, double att) {
        this->check_channel(channel);

        // Convert attenuation setting
        uint8_t atten;
        if      (att == 1)    atten = 0x00;
        else if (att == 10)   atten = 0x01;
        else if (att == 100)  atten = 0x02;
        else if (att == 1000) atten = 0x03;
        else {
            fprintf(stderr, "Invalid probe attenuation %f\n", att);
            abort();
        }

        if (channel == 1) 
            m_dso_settings[VERT_CH1_PROBE] = atten;
        else 
            m_dso_settings[VERT_CH2_PROBE] = atten;
        this->apply_dso_settings();
        return;
    };

    double dso5000p::get_atten(unsigned channel) {
        return 0;
    };

    void dso5000p::set_vert_base(unsigned channel, double volts_per_div) {
        return;
    };

    double dso5000p::get_vert_base(unsigned channel) {
        return 0;
    };

    void dso5000p::set_horz_base(double sec_per_div) {
        return;
    };

    double dso5000p::get_horz_base() {
        return 0;
    };

    void dso5000p::start_acquisition() {
        return;
    };

    void dso5000p::stop_acquisition() {
        return;
    };

    void dso5000p::single_shot() {
        return;
    };

    void dso5000p::set_trigger_type(trigger_type trig) {
        return;
    };

    void dso5000p::set_trigger_level(double level) {
        return;
    };

    void dso5000p::set_trigger_source(unsigned channel) {
        return;
    };

    bool dso5000p::triggered() {
        return false;
    };

    bool dso5000p::stopped() {
        return false;
    };

    void dso5000p::read_sample_data(unsigned channel,  
    std::vector<double> &horz_data, std::vector<double> &vert_data) {
        return;
    }
    
    /*

    void dso5000p::beep() {
        uint8_t payload[] = {0x01}, dummy[MAX_BUF_SIZE];
        debug_print("%s", "Sending beep\n");
        this->send_pkg(MSG_DEBUG, BUZZER, payload, 1);
        usleep(SLEEP_US);  // 100 ms delay -> TODO: replace!!
        this->read_pkg(MSG_DEBUG, BUZZER, dummy);
        return;
    }

    void dso5000p::lock_panel() {
        debug_print("%s", "Locking panel\n");
        uint8_t payload[] = {LOCK, 0x01}, dummy[MAX_BUF_SIZE];
        this->send_pkg(MSG_NORMAL, DAQ, payload, 2);
        usleep(SLEEP_US);  // 100 ms delay -> TODO: replace!!
        this->read_pkg(MSG_NORMAL, DAQ, dummy);
        return;
    }

    void dso5000p::unlock_panel() {
        debug_print("%s", "Unlocking panel\n");
        uint8_t payload[] = {LOCK, 0x00}, dummy[MAX_BUF_SIZE];
        this->send_pkg(MSG_NORMAL, DAQ, payload, 2);
        usleep(SLEEP_US);  // 100 ms delay -> TODO: replace!!
        this->read_pkg(MSG_NORMAL, DAQ, dummy);
        return;
    }

    void dso5000p::start_acquisition() {
        debug_print("%s", "Starting acquisition\n");
        uint8_t payload[] = {STASTO, 0x00}, dummy[MAX_BUF_SIZE];
        this->send_pkg(MSG_NORMAL, DAQ, payload, 2);
        usleep(SLEEP_US);  // 100 ms delay -> TODO: replace!!
        this->read_pkg(MSG_NORMAL, DAQ, dummy);
        return;
    }

    void dso5000p::stop_acquisition() {
        debug_print("%s", "Stopping acquisition\n");
        uint8_t payload[] = {STASTO, 0x01}, dummy[MAX_BUF_SIZE];
        this->send_pkg(MSG_NORMAL, DAQ, payload, 2);
        usleep(SLEEP_US);  // 100 ms delay -> TODO: replace!!
        this->read_pkg(MSG_NORMAL, DAQ, dummy);
        return;
    }

    bool dso5000p::triggered() {
        this->read_dso_settings();
        if (m_dso_settings[TRIG_STATE] == TRIGGERED)
            return true;
        else
            return false;
    }

    void dso5000p::set_probe_atten(int channel, int att) {
        uint8_t probe = 0x00;
        switch (att) {
            case 1: probe = 0x00; break;
            case 10: probe = 0x01; break;
            case 100: probe = 0x02; break;
            case 1000: probe = 0x03; break;
            default:
                fprintf(stderr, "Invalid probe attenuation %i\n", att);
                abort();
        }

        if (channel == 0)
            m_dso_settings[VERT_CH1_PROBE] = probe;
        else if (channel == 1)
            m_dso_settings[VERT_CH2_PROBE] = probe;
        else {
            fprintf(stderr, "Invalid channel %i\n", channel);
            abort();
        }
        m_vert_probe[channel] = att;
        return;
    }

    void dso5000p::set_vert_base(int channel, double volts_per_div) {
        uint8_t vb = 0x00;
        if (volts_per_div == 0.001) vb = 0x00;          // 1 mV/div
        else if (volts_per_div == 0.002) vb = 0x01;     // 2 mV/div
        else if (volts_per_div == 0.005) vb = 0x02;     // 5 mV/div
        else if (volts_per_div == 0.010) vb = 0x03;     // 10 mV/div
        else if (volts_per_div == 0.020) vb = 0x04;     // 20 mV/div
        else if (volts_per_div == 0.050) vb = 0x05;     // 50 mV/div
        else if (volts_per_div == 0.100) vb = 0x06;     // 100 mV/div
        else if (volts_per_div == 0.200) vb = 0x07;     // 200 mV/div
        else if (volts_per_div == 0.500) vb = 0x08;     // 500 mV/div
        else if (volts_per_div == 1.000) vb = 0x09;     // 1 V/div
        else if (volts_per_div == 2.000) vb = 0x0A;     // 2 V/div
        else if (volts_per_div == 5.000) vb = 0x0B;     // 5 V/div
        else {
            fprintf(stderr, "Invalid voltage base %f\n", volts_per_div);
            abort();
        }

        if (channel == 0)
            m_dso_settings[VERT_CH1_VB] = vb;
        else if (channel == 1)
            m_dso_settings[VERT_CH2_VB] = vb;
        else {
            fprintf(stderr, "Invalid channel %i\n", channel);
            abort();
        }
        m_vert_vb[channel] = volts_per_div;
        return;
    }

    void dso5000p::set_horz_base(double seconds_per_div) {
        uint8_t tb = 0x00;
        if      (seconds_per_div < 2e-9) tb = 0x00;  // 2 ns/div
        else if (seconds_per_div < 4e-9) tb = 0x01;  // 4 ns/div
        else if (seconds_per_div < 8e-9) tb = 0x02;  // 8 ns/div
        else if (seconds_per_div < 2e-8) tb = 0x03;  // 20 ns/div
        else if (seconds_per_div < 4e-8) tb = 0x04;  // 40 ns/div
        else if (seconds_per_div < 8e-8) tb = 0x05;  // 80 ns/div
        else if (seconds_per_div < 2e-7) tb = 0x06;  // 200 ns/div
        else if (seconds_per_div < 4e-7) tb = 0x07;  // 400 ns/div
        else if (seconds_per_div < 8e-7) tb = 0x08;  // 800 ns/div
        else if (seconds_per_div < 2e-6) tb = 0x09;  // 2 us/div
        else if (seconds_per_div < 4e-6) tb = 0x0A;  // 4 us/div
        else if (seconds_per_div < 8e-6) tb = 0x0B;  // 8 us/div
        else if (seconds_per_div < 2e-5) tb = 0x0C;  // 20 us/div
        else if (seconds_per_div < 4e-5) tb = 0x0D;  // 40 us/div
        else if (seconds_per_div < 8e-5) tb = 0x0E;  // 80 us/div
        else if (seconds_per_div < 2e-4) tb = 0x0F;  // 200 us/div
        else if (seconds_per_div < 4e-4) tb = 0x10;  // 400 us/div
        else if (seconds_per_div < 8e-4) tb = 0x11;  // 800 us/div
        else if (seconds_per_div < 2e-3) tb = 0x12;  // 2 ms/div
        else if (seconds_per_div < 4e-3) tb = 0x13;  // 4 ms/div
        else if (seconds_per_div < 8e-3) tb = 0x14;  // 8 ms/div
        else if (seconds_per_div < 2e-2) tb = 0x15;  // 20 ms/div
        else if (seconds_per_div < 4e-2) tb = 0x16;  // 40 ms/div
        else if (seconds_per_div < 8e-2) tb = 0x17;  // 80 ms/div
        else if (seconds_per_div < 2e-1) tb = 0x18;  // 200 ms/div
        else if (seconds_per_div < 4e-1) tb = 0x19;  // 400 ms/div
        else if (seconds_per_div < 8e-1) tb = 0x1A;  // 800 ms/div
        else if (seconds_per_div < 2.)   tb = 0x1B;  // 2 s/div
        else if (seconds_per_div < 4.)   tb = 0x1C;  // 4 s/div
        else if (seconds_per_div < 8.)   tb = 0x1D;  // 8 s/div
        else if (seconds_per_div < 20.)  tb = 0x1E;  // 20 s/div
        else if (seconds_per_div < 40.)  tb = 0x1F;  // 40 s/div
        else {
            fprintf(stderr, "Invalid timebase %f\n", seconds_per_div);
            abort();
        }
        m_dso_settings[HORIZ_TB] = tb;
        m_dso_settings[HORIZ_WIN_TB] = tb;
        m_horiz_tb = seconds_per_div;
        return;
    }

    void dso5000p::set_edge_trig(int channel, double thresh_volts, uint8_t edge) {
        if ( (channel != 0) && (channel != 1) ) {
            fprintf(stderr, "Invalid channel %i\n", channel);
            abort();
        }
        // Set source and edge
        m_dso_settings[TRIG_SRC] = channel;
        m_dso_settings[TRIG_EDGE_SLOPE] = edge;
        // Calculate threshhold
        uint16_t thresh = (uint16_t)(thresh_volts * 25./m_vert_vb[channel])
            + m_vert_pos[channel];
        m_dso_settings[TRIG_VPOS + 1] = 0xFF & (thresh >> 8);
        m_dso_settings[TRIG_VPOS] = 0xFF & thresh;
        return;
    }

    */

    void dso5000p::read_dso_settings() {
        debug_print("%s", "Reading DSO settings\n");
        uint8_t payload[] = {};
        this->send_pkg(MSG_NORMAL, READ_CFG, payload, 0);
        usleep(100e3);  // 100 ms delay -> TODO: replace!!
        this->read_pkg(MSG_NORMAL, READ_CFG, m_dso_settings);

        // Extract settings from struct
        m_vert_vb[0]      = this->get_volts_per_div(m_dso_settings[VERT_CH1_VB]);
        m_vert_vb[1]      = this->get_volts_per_div(m_dso_settings[VERT_CH2_VB]);
        m_vert_pos[0]     = (uint16_t)(m_dso_settings[VERT_CH1_POS+1] << 8) |
                          (uint16_t)m_dso_settings[VERT_CH1_POS];
        m_vert_pos[1]     = (uint16_t)(m_dso_settings[VERT_CH2_POS+1] << 8) |
                          (uint16_t)m_dso_settings[VERT_CH2_POS];
        m_vert_probe[0]   = this->get_probe_attenuation(m_dso_settings[VERT_CH1_PROBE]);
        m_vert_probe[1]   = this->get_probe_attenuation(m_dso_settings[VERT_CH2_PROBE]);
        m_horiz_tb        = this->get_timebase(m_dso_settings[HORIZ_TB]);
        m_horiz_trigtime  = ((uint64_t)m_dso_settings[HORIZ_TRIGTIME + 7] << 56) |
                            ((uint64_t)m_dso_settings[HORIZ_TRIGTIME + 6] << 48) |
                            ((uint64_t)m_dso_settings[HORIZ_TRIGTIME + 5] << 40) |
                            ((uint64_t)m_dso_settings[HORIZ_TRIGTIME + 4] << 32) |
                            ((uint64_t)m_dso_settings[HORIZ_TRIGTIME + 3] << 24) |
                            ((uint64_t)m_dso_settings[HORIZ_TRIGTIME + 2] << 16) |
                            ((uint64_t)m_dso_settings[HORIZ_TRIGTIME + 1] <<  8) |
                            ((uint64_t)m_dso_settings[HORIZ_TRIGTIME]);

        debug_print("vert_ch1_vb = %f V/div (0x%02X)\n", m_vert_vb[0],
            m_dso_settings[VERT_CH1_VB]);
        debug_print("vert_ch2_vb = %f V/div (0x%02X)\n", m_vert_vb[1],
            m_dso_settings[VERT_CH2_VB]);
        debug_print("vert_ch1_probe = %i (0x%02X)\n", m_vert_probe[0],
            m_dso_settings[VERT_CH1_PROBE]);
        debug_print("vert_ch2_probe = %i (0x%02X)\n", m_vert_probe[1],
            m_dso_settings[VERT_CH2_PROBE]);
        debug_print("horiz_tb = %e s/div (0x%02X)\n", m_horiz_tb,
            m_dso_settings[HORIZ_TB]);
        debug_print("horiz_trigtime = %lli (0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X)\n",
            m_horiz_trigtime,
            m_dso_settings[HORIZ_TRIGTIME + 7],
            m_dso_settings[HORIZ_TRIGTIME + 6],
            m_dso_settings[HORIZ_TRIGTIME + 5],
            m_dso_settings[HORIZ_TRIGTIME + 4],
            m_dso_settings[HORIZ_TRIGTIME + 3],
            m_dso_settings[HORIZ_TRIGTIME + 2],
            m_dso_settings[HORIZ_TRIGTIME + 1],
            m_dso_settings[HORIZ_TRIGTIME]);

        return;
    }

    void dso5000p::apply_dso_settings() {
        debug_print("%s", "Applying DSO settings\n");
        uint8_t dummy[MAX_BUF_SIZE] = {0x00};
        int ret = 0;

        this->send_pkg(MSG_NORMAL, WRITE_CFG, m_dso_settings, settings::len);

        while (ret == 0) {
            try {
                ret = this->read_pkg(MSG_NORMAL, WRITE_CFG, dummy, 100);
            } catch (timeout &ex) {
                debug_print("%s\n", "Caught timeout, resending...");
                this->send_pkg(MSG_NORMAL, WRITE_CFG, m_dso_settings, settings::len);
            }
        }

        /*
        this->send_pkg(MSG_NORMAL, WRITE_CFG, m_dso_settings, settings::len);
        usleep(SLEEP_US);  // 100 ms delay -> TODO: replace!!
        this->read_pkg(MSG_NORMAL, WRITE_CFG, dummy);
        */
        return;
    }

    /*

    void dso5000p::read_sample_data(int channel, std::vector<double> &horz_data, std::vector<double> &vert_data) {
        if ( (channel != 0) && (channel != 1) ) {
            fprintf(stderr, "Wrong channel number %i\n", channel);
            abort();
        }

        debug_print("%s", "Reading sample data\n");
        uint8_t payload[] = {0x01, (uint8_t)channel}, buf[MAX_BUF_SIZE];
        this->send_pkg(MSG_NORMAL, READ_SAMPLE, payload, sizeof(payload));
        int len = 1, sample_len = -1;
        double vert, horz;
        while (len > 0) {
            usleep(SLEEP_US);
            len = this->read_pkg(MSG_NORMAL, READ_SAMPLE, buf);
            // payload contains...
            if (buf[0] == 0x00) {           //... sample data length (3 bytes)
                sample_len = buf[1] | (buf[2]<<8) | (buf[3]<<16);
                debug_print("Total sample length = %i\n", sample_len);
            } else if (buf[0] == 0x01) {    //... actual sample data
                debug_print("Found %i bytes of sample data\n", len);
                if (sample_len == -1)
                    throw bad_protocol("Received sample data before sample length", buf[0]);
                for (int i = 2; i < len; i++) {
                    // 10.2 divisions vertical (-127 .. +127)
                    vert = m_vert_probe[channel] * m_vert_vb[channel] * 5.1 * ((int8_t)buf[i])/127.;
                    // 20 divisions horizontal
                    horz = m_horiz_tb * 20. * i/sample_len;
                    vert_data.push_back(vert);
                    horz_data.push_back(horz);
                }
            } else if (buf[0] == 0x02) {    //... channel number -> end of sample data; break!
                debug_print("%s", "Found sample data channel\n");
                break;
            } else if (buf[0] == 0x03) {    // Error or no data available (or scope in STOP mode)
                throw bad_protocol("No sample data available", buf[0]);
            }
        }

        return;
    }

    void dso5000p::read_file(std::string src_path, std::string dst_path) {
        debug_print("Reading file '%s' from DSO\n", src_path.c_str());

        uint8_t payload[src_path.size()+1];
        uint8_t buf[MAX_BUF_SIZE] = {'\0'}, len = 1;
        std::ofstream dst_file;

        // payload = sub cmd + path
        payload[0] = 0x00;
        strcat((char*)payload, src_path.c_str());
        this->send_pkg(MSG_NORMAL, READ_FILE, payload, src_path.size()+1);

        // Open destination...
        dst_file.open(dst_path.c_str());
        while (len > 0)  {
            usleep(100e3);  // 100 ms delay -> TODO: replace!!
            len = this->read_pkg(MSG_NORMAL, 0x10, buf);
            if (buf[0] == 0x01)         // file content
                dst_file << (const char*)buf;
            else if (buf[0] == 0x02)    // checksum (dont care)
                break;
        }
        dst_file.close();

        return;
    }

    */

    /*
     *      P R I V A T E   M E T H O D S
     */

    void dso5000p::check_channel(unsigned channel) {
        if ( (channel == 0) || (channel > this->get_n_channels()) ) {
            fprintf(stderr, "Invalid channel %i\n", channel);
            abort();
        }
        return;
    }

    double dso5000p::get_volts_per_div(uint8_t vb) {
        switch (vb) {
            case 0x00: return 0.001;    // 1 mV/div
            case 0x01: return 0.002;    // 2 mV/div
            case 0x02: return 0.005;    // 5 mV/div
            case 0x03: return 0.010;    // 10 mV/div
            case 0x04: return 0.020;    // 20 mV/div
            case 0x05: return 0.050;    // 50 mV/div
            case 0x06: return 0.100;    // 100 mV/div
            case 0x07: return 0.200;    // 200 mV/div
            case 0x08: return 0.500;    // 500 mV/div
            case 0x09: return 1.000;    // 1 V/div
            case 0x0A: return 2.000;    // 2 V/div
            case 0x0B: return 5.000;    // 5 V/div
            default:
                fprintf(stderr, "Invalid voltage base 0x%02X\n", vb);
                abort();
        }
        return .0;
    }

    double dso5000p::get_timebase(uint8_t tb) {
        switch (tb) {
            case 0x00: return 2e-9;     // 2 ns/div
            case 0x01: return 4e-9;     // 4 ns/div
            case 0x02: return 8e-9;     // 8 ns/div
            case 0x03: return 2e-8;     // 20 ns/div
            case 0x04: return 4e-8;     // 40 ns/div
            case 0x05: return 8e-8;     // 80 ns/div
            case 0x06: return 2e-7;     // 200 ns/div
            case 0x07: return 4e-7;     // 400 ns/div
            case 0x08: return 8e-7;     // 800 ns/div
            case 0x09: return 2e-6;     // 2 us/div
            case 0x0A: return 4e-6;     // 4 us/div
            case 0x0B: return 8e-6;     // 8 us/div
            case 0x0C: return 2e-5;     // 20 us/div
            case 0x0D: return 4e-5;     // 40 us/div
            case 0x0E: return 8e-5;     // 80 us/div
            case 0x0F: return 2e-4;     // 200 us/div
            case 0x10: return 4e-4;     // 400 us/div
            case 0x11: return 8e-4;     // 800 us/div
            case 0x12: return 2e-3;     // 2 ms/div
            case 0x13: return 4e-3;     // 4 ms/div
            case 0x14: return 8e-3;     // 8 ms/div
            case 0x15: return 2e-2;     // 20 ms/div
            case 0x16: return 4e-2;     // 40 ms/div
            case 0x17: return 8e-2;     // 80 ms/div
            case 0x18: return 2e-1;     // 200 ms/div
            case 0x19: return 4e-1;     // 400 ms/div
            case 0x1A: return 8e-1;     // 800 ms/div
            case 0x1B: return 2.;       // 2 s/div
            case 0x1C: return 4.;       // 4 s/div
            case 0x1D: return 8.;       // 8 s/div
            case 0x1E: return 20.;      // 20 s/div
            case 0x1F: return 40.;      // 40 s/div
            default:
                fprintf(stderr, "Invalid time base 0x%02X\n", tb);
                abort();
        }
        return .0;
    }

    int dso5000p::get_probe_attenuation(uint8_t att) {
        switch (att) {
            case 0x00: return 1;
            case 0x01: return 10;
            case 0x02: return 100;
            case 0x03: return 1000;
            default:
                fprintf(stderr, "Invalid probe attenuation 0x%02X\n", att);
                abort();
        }
        return 0;
    }

    int dso5000p::create_pkt(uint8_t mark, uint8_t cmd, const uint8_t* data,
    size_t len, uint8_t* packet) {
        // len = payload + marker + cmd + 2 bytes len + checksum
        int total_len = len + 5;
        debug_print("Creating packet (len = %i)\n", total_len);
        packet[0] = mark;
        packet[1] = (len+2) & 0xFF;
        packet[2] = ((len+2) >> 8) & 0xFF;
        packet[3] = cmd;
        for (int i = 0; i < len; i++)
            packet[i+4] = data[i];
        uint8_t checksum = 0x00;
        for (int i = 0; i < total_len-1; i++)
            checksum += packet[i];
        packet[total_len-1] = checksum;
        return total_len;
    }

    int dso5000p::extract_pkt(const uint8_t* packet, int pkt_len, uint8_t &mark,
    uint8_t &cmd, uint8_t* data) {
        debug_print("Extracting packet (len = %i)\n", pkt_len);
        mark = packet[0];
        cmd = packet[3];
        int length = 0xFFFF & (packet[1] | ((uint16_t)packet[2] << 8));
        // NOTE: the length includes payload, cmd, and checksum bytes
        // but not marker nor the two length bytes themselves
        for (int i = 0; i < length-2; i++)
            data[i] = packet[i+4];
        // Calculate checksum
        uint8_t checksum = 0x00;
        for (int i = 0; i < length+2; i++)
            checksum += packet[i];
        // Validate checksum and length
        if (checksum != packet[length+2]) {
            debug_print("Wrong checksum : expected 0x%02X, received 0x%02X\n",
                checksum, packet[length+2]);
            //return -1; // ignore for now...
        }
        if (length != pkt_len-3) {
            debug_print("Wrong length : expected %i, received %i\n", pkt_len-3,
                length);
            //return -1; // ignore for now...
        }

        return length-2;
    }

    void dso5000p::send_pkg(uint8_t mark, uint8_t cmd, const uint8_t* payload,
    int len) {
        uint8_t packet[MAX_BUF_SIZE];
        int pkt_len = this->create_pkt(mark, cmd, payload, len, packet);
        comm->write_bulk(packet, pkt_len);
        return;
    }

    int dso5000p::read_pkg(uint8_t mark, uint8_t cmd, uint8_t* payload,
    int timeout_ms) {
        uint8_t packet[MAX_BUF_SIZE];
        uint8_t marker = 0x00, command = 0x00;
        int pkt_len = comm->read_bulk(packet, MAX_BUF_SIZE, timeout_ms);
        int payload_len = this->extract_pkt(packet, pkt_len, marker, command,
            payload);

        // Check marker and command byte (response bit set?)
        if ( marker != mark ) {
            debug_print("Wrong marker : expected 0x%02X, received 0x%02X\n",
                mark, marker);
            //return -1; // ignore for now... (TODO)
        }
        if ( command != (cmd | RESP_FLG) ) {
            debug_print("Wrong command : expected 0x%02X, received 0x%02X\n",
                cmd | RESP_FLG, command);
            //return -1; // ignore for now... (TODO)
        }

        // Debugging printfs...
        #ifdef DSO_MSG_DEBUG
        printf("mark :\t0x%02X\ncmd :\t0x%02X\ndata :\r\t", mark, command);
        for (int i = 0; i < payload_len; i++)
            printf("0x%02X[%i]%s", payload[i], i, (i%10 == 9) ? "\n\t" : " ");
        printf("\n");
        #endif

        return payload_len;
    }

    void dso5000p::flush_buffer() {
        uint8_t buf[MAX_BUF_SIZE];
        // Read until nothing left to be read...
        try {
            while ( comm->read_bulk(buf, MAX_BUF_SIZE, 100) > 0) { }
        } catch (const timeout& ex) { }
        return;
    }

}