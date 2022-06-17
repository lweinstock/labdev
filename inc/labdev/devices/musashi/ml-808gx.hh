#ifndef LD_ML_808_GX_HH
#define LD_ML_808_GX_HH

#include <labdev/serial_interface.hh>

namespace labdev {
    class ml_808gx {
    public:
        ml_808gx() {};
        ml_808gx(serial_interface* ser);
        ~ml_808gx();

        // Dispense glue using parameters from current channel
        void dispense();

        // Select dispensing parameters from channel/recipe
        void select_channel(unsigned ch);

        // Set parameters for current channel recipe (see manual p. 61):
        //  - pressure in 100 Pa
        //  - duratio in ms
        //  - on/off delay in 0.1ms
        void set_channel_params(unsigned pressure, unsigned dur, 
            unsigned on_delay, unsigned off_delay);
        void get_channel_params(unsigned& pressure, unsigned& dur, 
            unsigned& on_delay, unsigned& off_delay);

        // Manual and timed despense modes (see manual p. 16)
        void manual_mode();
        void timed_mode();

        // Vacuum interval feature (see manual p. 35)
        void enable_vacuum(bool ena = true);
        void disable_vacuum() { this->enable_vacuum(false); }
        void set_vacuum_interval(unsigned on_ms, unsigned off_ms);

        // Send formatted command (see manual p. 55)
        void send_command(std::string cmd, std::string data = "");

        // Send data host -> device (see manual p. 56, 58ff)
        void download_command(std::string cmd, std::string data = "");

        // Query data device -> host (see manual p. 57, 67ff)
        void upload_command(std::string cmd, std::string& payload);

    private:
        serial_interface* comm;
        void init();
        unsigned m_cur_ch;

        // Protocol definitions (see manual p. 57)
        const static std::string STX;   // Start of text (ASCII)
        const static std::string ETX;   // End of text (ASCII)
        const static std::string EOT;   // End of transmission (ASCII)
        const static std::string ENQ;   // Start enquiry (ASCII)
        const static std::string ACK;   // Acknowledge (ASCII)
        const static std::string A0;    // Success
        const static std::string A2;    // Error
        const static std::string CAN;   // No idea what this is

    };

}

#endif