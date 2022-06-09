#ifndef UT61B_H
#define UT61B_H

#include <labdev/exceptions.hh>
#include <labdev/serial_interface.hh>

namespace labdev {

    /*
     *      Brief exception class for UT61B multimeter specific errors
     */

    class ut61b_exception : public exception {
    public:
        ut61b_exception(const std::string& msg, int err = 0):
            exception(msg, err) {};
        virtual ~ut61b_exception() {};
    };

    /*
     *      UNI-T UT61B series multimeter
     */

    class ut61b {
    public:
        ut61b();
        ut61b(serial_interface* ser);
        ~ut61b();

        // Get current value from screen
        double get_value();

        // Get current unit from screen
        std::string get_unit() { return m_unit; }

    private:
        serial_interface* m_comm;
        std::string m_unit;

        // UT61b end of message character
        static constexpr const char* EOM = "\r\n";

        enum prefix_flag : uint8_t {
            micro   = (1 << 7),
            milli   = (1 << 6),
            kilo    = (1 << 5),
            mega    = (1 << 4),
            none    = (1 << 0)
        };

        enum unit_flag : uint8_t {
            volt    = (1 << 7),
            amp     = (1 << 6),
            ohm     = (1 << 5),
            hertz   = (1 << 3),
            degf    = (1 << 1),
            degc    = (1 << 0)
        };


    };
}

#endif