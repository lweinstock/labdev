#ifndef LD_ML_808_GX_HH
#define LD_ML_808_GX_HH

#include <labdev/serial_interface.hh>

namespace labdev {
    class ml_808gx {
    public:
        ml_808gx(serial_interface* ser);
        ~ml_808gx();

        // Dispense glue using parameters from current channel
        void dispense();

        // Select dispensing parameters from channel/recipe
        void select_channel(unsigned ch);

        // Set parameters for channel recipe (see manual p. 61):
        //  - pressure in 100 Pa
        //  - duratio in ms
        //  - on/off delay in 0.1ms
        void set_channel_params(unsigned ch, unsigned pressure, unsigned dur, 
            unsigned on_delay, unsigned off_delay);
        void get_channel_params(unsigned &ch, unsigned &pressure, unsigned &dur, 
            unsigned &on_delay, unsigned &off_delay);

        // Manual and timed despense modes (see manual p. 16)
        void manual_mode();
        void timed_mode();

        // Vacuum interval feature (see manual p. 35)
        void set_vacuum(bool ena);
        void enable_vacuum() { this->set_vacuum(false); }
        void disable_vacuum() { this->set_vacuum(true); }
        void set_vacuum_interval(unsigned on_ms, unsigned off_ms);

        // Send data host -> device (see manual p. 56, 58ff)
        // cmd has to be size 4 (FIXME)
        void download_command(std::string cmd, std::string data = "");

        // Query data device -> host (see manual p. 57, 67ff)
        int upload_command(std::string cmd, std::string &data);

    private:
        serial_interface* comm;
        void init();

        // Protocol definitions (see manual p. 57)
        const static uint8_t STX;   // Start of text (ASCII)
        const static uint8_t ETX;   // End of text (ASCII)
        const static uint8_t EOT;   // End of transmission
        const static uint8_t ENQ;   // Start enquiry
        const static uint8_t ACK;   // Acknowledge
        const static char A0[];  // Success
        const static char A2[];  // Error

    };

}

#endif