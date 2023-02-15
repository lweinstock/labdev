#include <pybind11/pybind11.h>

#include <labdev/interface.hh>
#include <labdev/serial_interface.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/usb_interface.hh>
#include <labdev/visa_interface.hh>

#include <labdev/devices/device.hh>
#include <labdev/devices/oscilloscope.hh>
#include <labdev/devices/jenny-science/xenax_xvi_75v8.hh>
#include <labdev/devices/baumer/om70_l.hh>
#include <labdev/devices/musashi/ml-808gx.hh>

namespace py = pybind11;

using labdev::serial_config;
using labdev::ip_address;
using labdev::usb_config;
using labdev::visa_identifier;

using labdev::device;
using labdev::xenax_xvi_75v8;
using labdev::om70_l;
using labdev::ml_808gx;

PYBIND11_MODULE(pylabdev, m) {
    m.doc() = "Library to communicate with lab devices.";

    /*   I N T E R F A C E   C O N F I G S   */

    py::class_<ip_address>(m, "ip_address")
        .def(py::init<>(),
            "Create empty IP address struct")
        .def(py::init<std::string, unsigned>(),
            py::arg("ip"),
            py::arg("port"),
            "Create IP address struct with given ip and port"
        )
        .def_readwrite("ip", &ip_address::ip, 
            "IPv4 of target device")
        .def_readwrite("port", &ip_address::port, 
            "Port of target device")
    ;

    py::class_<serial_config>(m, "serial_config")
        .def(py::init<>(),
            "Create empty serial config")
        .def(py::init<std::string, unsigned, unsigned, bool, bool, unsigned>(),
            py::arg("dev_file"),
            py::arg("baud") = 9600,
            py::arg("nbits") = 8,
            py::arg("par_ena") = false,
            py::arg("par_even") = false,
            py::arg("stop_bits") = 1,
            "Create serial config with given arguments (default: BAUD 9600 8N1)"
        )
        .def_readwrite("dev_file", &serial_config::dev_file, 
            "Path to device file of target device")
        .def_readwrite("baud", &serial_config::baud,
            "Serial baudrate in bits per second")
        .def_readwrite("nbits", &serial_config::nbits,
            "Number of bits per transmission")
        .def_readwrite("par_ena", &serial_config::par_ena,
            "En-/disable parity bit for transmission")
        .def_readwrite("par_even", &serial_config::par_even,
            "Parity bit is even or oddå")
        .def_readwrite("stop_bits", &serial_config::stop_bits,
            "Number of stop bits per transmission")
    ;

    /*   D E V I C E S   */

    // Device base class
    py::class_<device>(m, "device")
        .def(py::init())
        .def("connected", &device::connected,
            "Returns true if device is ready for communication")
        .def("disconnect", &device::disconnect, 
            "Closes the communication interface")
        .def("reconnect", &device::reconnect,
            "Re-establishes the current communication interface")
        .def("get_info", &device::get_info,
            "Returns human readable information string about the interface")
    ;

    // Xenax XVI motor controller
    py::class_<xenax_xvi_75v8, device> xenax_xvi(m, "xenax_xvi");
        xenax_xvi.def_property_readonly_static("dflt_port", 
            [](py::object) { return xenax_xvi_75v8::PORT; })
        .def(py::init<>())
        .def(py::init<ip_address&>())
        .def(py::init<serial_config&>())
        .def("connect", 
            static_cast<void (xenax_xvi_75v8::*)(ip_address&)>(&xenax_xvi_75v8::connect),
            "Connect to given ip address")
        .def("connect", 
            static_cast<void (xenax_xvi_75v8::*)(serial_config&)>(&xenax_xvi_75v8::connect),
            "Connect to given serial device")
        .def("__repr__", 
            [](const xenax_xvi_75v8 &self) {
                return self.get_info();
            })
        .def("power_on", 
            &xenax_xvi_75v8::power_on, 
            "Turn on motor power", 
            py::arg("enable") = true)
        .def("power_off", 
            &xenax_xvi_75v8::power_off, 
            "Turn off motor power")
        .def("reference_axis", 
            &xenax_xvi_75v8::reference_axis, 
            "Start referencing axis position")
        .def("is_referenced", 
            &xenax_xvi_75v8::is_referenced, 
            "Returns true if axis is referenced")
        .def("disable_smu", 
            &xenax_xvi_75v8::disable_smu, 
            "Disable safety motion unit")
        .def("move_position", 
            &xenax_xvi_75v8::move_position, 
            "Non-blocking movement to absolute position in incs",
            py::arg("pos"))
        .def("goto_position", 
            &xenax_xvi_75v8::goto_position, 
            "Blocking movement to absolute position in incs",
            py::arg("pos"),
            py::arg("interval_ms") = 500,
            py::arg("timeout_ms") = 10000)
        .def("get_position", 
            &xenax_xvi_75v8::get_position, 
            "Returns absolute position in incs")
        .def("in_motion", 
            &xenax_xvi_75v8::in_motion, 
            "Returns true if axis is moving")
        .def("in_position", 
            &xenax_xvi_75v8::in_position, 
            "Returns true if axis has reached its final position")
        .def("stop_motion", 
            &xenax_xvi_75v8::stop_motion, 
            "Immediatly stops all movement")
        .def("set_speed", 
            &xenax_xvi_75v8::set_speed, 
            "Set movement speed in incs/sec",
            py::arg("inc_per_sec"))
        .def("get_speed", 
            &xenax_xvi_75v8::get_speed, 
            "Returns current movement speed in incs/sec")
        .def("set_acceleration", 
            &xenax_xvi_75v8::set_acceleration, 
            "Set movement acceleration in incs/sec2",
            py::arg("inc_per_sec2"))
        .def("get_acceleration", 
            &xenax_xvi_75v8::get_acceleration, 
            "Returns current movement acceleration in incs/sec2")
        .def("set_s_curve", 
            &xenax_xvi_75v8::set_s_curve, 
            "Set acceleration curve in %",
            py::arg("percent"))
        .def("get_s_curve", 
            &xenax_xvi_75v8::get_s_curve, 
            "Returns current acceleration curve in %")
        .def("force_calibration",
            &xenax_xvi_75v8::force_calibration, 
            "Perform force calibration for force measurements", 
            py::arg("distance") = 1000)
        .def("get_motor_current",
            &xenax_xvi_75v8::get_motor_current, 
            "Returns the motor current in mA")
        .def("get_force_constant",
            &xenax_xvi_75v8::get_force_constant, 
            "Returns the force constant in N/mA")
        .def("get_motor_force",
            &xenax_xvi_75v8::get_motor_force, 
            "Returns the motor force in N")
        .def("set_force_limit",
            &xenax_xvi_75v8::set_force_limit, 
            "Set maximum force of axis in N",
            py::arg("flim_N"))
        .def("get_force_limit",
            &xenax_xvi_75v8::get_force_limit, 
            "Returns maximum force of axis in N")
        .def("set_output_type",
            &xenax_xvi_75v8::set_output_type, 
            "Sets the output type (sink/source/hiZ) of GPIO pins",
            py::arg("mask"))
        .def("set_output_state",
            &xenax_xvi_75v8::set_output_state, 
            "Sets the output state (high/low) of GPIO pins",
            py::arg("gpio"))
        .def("get_output_state",
            &xenax_xvi_75v8::get_output_state, 
            "Returns GPIO output state")
        .def("get_input_state",
            &xenax_xvi_75v8::get_input_state, 
            "Returns GPIO input state")
        .def("reset_motor_type",
            &xenax_xvi_75v8::reset_motor_type, 
            "Required when motors are changed")
        .def("force_limit_reached", 
            &xenax_xvi_75v8::force_limit_reached, 
            "Returns true if force limit was reached")
        .def("get_error",
            &xenax_xvi_75v8::get_error,
            "Returns current error number")
        .def("get_strerror",
            &xenax_xvi_75v8::get_strerror,
            "Returns current error string")
    ;

    // Set Output Type (SOT) enums for Xenax Xvi 75v8
    py::enum_<xenax_xvi_75v8::SOT>(xenax_xvi, "SOT")
        .value("SOT10", xenax_xvi_75v8::SOT::SOT10)
        .value("SOT11", xenax_xvi_75v8::SOT::SOT11)
        .value("SOT20", xenax_xvi_75v8::SOT::SOT20)
        .value("SOT21", xenax_xvi_75v8::SOT::SOT21)
        .value("SOT30", xenax_xvi_75v8::SOT::SOT30)
        .value("SOT31", xenax_xvi_75v8::SOT::SOT31)
        .value("SOT40", xenax_xvi_75v8::SOT::SOT40)
        .value("SOT41", xenax_xvi_75v8::SOT::SOT41)
        .value("SOT50", xenax_xvi_75v8::SOT::SOT50)
        .value("SOT51", xenax_xvi_75v8::SOT::SOT51)
        .value("SOT60", xenax_xvi_75v8::SOT::SOT60)
        .value("SOT61", xenax_xvi_75v8::SOT::SOT61)
        .value("SOT70", xenax_xvi_75v8::SOT::SOT70)
        .value("SOT71", xenax_xvi_75v8::SOT::SOT71)
        .value("SOT80", xenax_xvi_75v8::SOT::SOT80)
        .value("SOT81", xenax_xvi_75v8::SOT::SOT81)
    ;

    // Set Output Activity (SOA) enums for Xenax Xvi 75v8
    py::enum_<xenax_xvi_75v8::SOA>(xenax_xvi, "SOA")
        .value("SOA1", xenax_xvi_75v8::SOA::SOA1)
        .value("SOA2", xenax_xvi_75v8::SOA::SOA2)
        .value("SOA3", xenax_xvi_75v8::SOA::SOA3)
        .value("SOA4", xenax_xvi_75v8::SOA::SOA4)
        .value("SOA5", xenax_xvi_75v8::SOA::SOA5)
        .value("SOA6", xenax_xvi_75v8::SOA::SOA6)
        .value("SOA7", xenax_xvi_75v8::SOA::SOA7)
        .value("SOA8", xenax_xvi_75v8::SOA::SOA8)
    ;

    // Baumer laser distance sensor OM70
    py::class_<om70_l, device>(m, "om70_l")
        .def_property_readonly_static("dflt_port", 
            [](py::object) { return om70_l::PORT; })
        .def(py::init<>())
        .def(py::init<ip_address&>())
        .def("connect", 
            static_cast<void (om70_l::*)(ip_address&)>(&om70_l::connect),
            "Connect to given ip address")
        .def("__repr__", 
            [](const om70_l &self) {
                return self.get_info();
            })
        .def("get_distance", 
            static_cast<float (om70_l::*)()>(&om70_l::get_distance), 
            "Returns distance in mm")
    ;

    // Musashi time-pressure dispenser ML-808 GX
    py::class_<ml_808gx, device>(m, "ml_808gx")
        .def(py::init())
        .def(py::init<serial_config&>())
        .def("connect", 
            static_cast<void (ml_808gx::*)(serial_config&)>(&ml_808gx::connect),
            "Connect to given serial device")
        .def("__repr__", 
            [](const ml_808gx &self) {
                return "ML-808gx dispenser unit, "
                + self.get_info();
            })
        .def("dispense", 
            &ml_808gx::dispense, 
            "Dispense glue according to current mode (manual/timed) and "
            "channel/recipe")
        .def("select_channel", 
            &ml_808gx::select_channel, 
            "Select channel/recipe")
        .def("manual_mode", 
            &ml_808gx::manual_mode, 
            "Stop dispensing after receiving a second dispence command")
        .def("timed_mode", 
            &ml_808gx::timed_mode, 
            "Stop dispensing after time defined by recipe")
        .def("set_channel_params", 
            &ml_808gx::set_channel_params, 
            "Set parameters for current channel; pressure in 100 Pa, duration "
            "in ms, on/off delay in 0.1ms", 
            py::arg("pressure"), 
            py::arg("dur"), 
            py::arg("on_delay"), 
            py::arg("off_delay"))
        .def("get_channel_params",
            static_cast<std::tuple<unsigned, unsigned, unsigned, unsigned> 
                (ml_808gx::*)()>(&ml_808gx::get_channel_params),
            "Returns parameters of current channel; pressure in 100 Pa, duration "
            "in ms, on/off delay in 0.1ms")
    ;
}