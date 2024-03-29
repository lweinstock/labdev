#ifndef HMP4000_H
#define HMP4000_H

#include <labdev/exceptions.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/serial_interface.hh>
#include <labdev/devices/scpi_device.hh>

namespace labdev {

    /*
     *      Rhode und Schwarz HMP4000 series linear power supply
     */

    class hmp4000 : public scpi_device {
    public:
        hmp4000();
        hmp4000(tcpip_interface* tcpip);
        hmp4000(serial_interface* ser);
        ~hmp4000();

        // En-/disable channel for output switching
        void enable_channel(int channel, bool ena = true);
        bool channel_enabled(int channel);

        // Switches the outputs of all enabled channels on/off
        void enable_outputs(bool ena = true);

        // Set/get output voltage/current for given channel
        void set_voltage(int channel, double volts);
        double get_voltage(int channel);
        void set_current(int channel, double amps);
        double get_current(int channel);

        // Measure actual output voltage/current value
        double measure_voltage(int channel);
        double measure_current(int channel);

        // Over voltage protection settings
        void set_ovp(int channel, double volts);
        void ovp_reset(int channel);
        bool ovp_tripped(int channel);

    private:
        int cur_ch;

        void init();

        // Select seperate channel "instrument"
        void select_channel(int ch);

        // Activate channel for output switching
        void activate(bool ena);

    };

}

#endif