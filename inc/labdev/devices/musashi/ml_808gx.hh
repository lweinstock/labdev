#ifndef LD_ML_808_GX_HH
#define LD_ML_808_GX_HH

#include <labdev/devices/ld_device.hh>
#include <labdev/serial_interface.hh>

#include <tuple>

namespace labdev {

class ml_808gx : public ld_device {
public:
    ml_808gx() : ld_device("ML-808GX"), m_cur_ch(0) {};
    ml_808gx(serial_interface* ser);
    ~ml_808gx();

    void connect(serial_interface* ser);
    void disconnect() override;

    // Dispense glue using parameters from current channel
    void dispense();

    // Select dispensing parameters from channel/recipe
    void set_channel(unsigned ch);
    unsigned get_channel();

    // Set parameters for current channel recipe (see manual p. 61):
    void set_channel_params(float pressure_kPa, float dur_ms, float on_delay_ms, 
        float off_delay_ms);
    void get_channel_params(float& pressure_kPa, float& dur_ms, 
        float& on_delay_ms, float& off_delay_ms);
    // Python-style tuple return (pressure, duration, on/off delay)
    std::tuple<float, float, float, float> get_channel_params();

    // Setter and getter methods for single parameters of current channel
    void set_pressure(float pressure_kPa);
    float get_pressure();
    void set_duration(float dur_ms);
    float get_duration();
    void set_delays(float on_delay_ms, float off_delay_ms);
    void get_delays(float &on_delay_ms, float &off_delay_ms);
    std::tuple<float, float> get_delays();

    // Manual and timed despense modes (see manual p. 16)
    void manual_mode();
    void timed_mode();

    // Vacuum interval feature (see manual p. 35)
    void enable_vacuum(bool ena = true);
    void disable_vacuum() { this->enable_vacuum(false); }
    void set_vacuum_interval(unsigned on_ms, unsigned off_ms);


private:
    // Protocol definitions (see manual p. 57)
    const static std::string STX;   // Start of text (ASCII)
    const static std::string ETX;   // End of text (ASCII)
    const static std::string EOT;   // End of transmission (ASCII)
    const static std::string ENQ;   // Start enquiry (ASCII)
    const static std::string ACK;   // Acknowledge (ASCII)
    const static std::string A0;    // Success
    const static std::string A2;    // Error
    const static std::string CAN;   // No idea what this is

    void init();
    unsigned m_cur_ch;
    
    // Send formatted command (see manual p. 55)
    void send_command(std::string cmd, std::string data = "");

    // Send data host -> device (see manual p. 56, 58ff)
    void download_command(std::string cmd, std::string data = "");

    // Query data device -> host (see manual p. 57, 67ff)
    void upload_command(std::string cmd, std::string& payload);


};

}

#endif