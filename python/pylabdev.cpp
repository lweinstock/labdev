#include <pybind11/pybind11.h>

#include <labdev/interface.hh>
#include <labdev/tcpip_interface.hh>

#include <labdev/devices/device.hh>
#include <labdev/devices/oscilloscope.hh>
#include <labdev/devices/jenny-science/xenax_xvi_75v8.hh>
#include <labdev/devices/baumer/om70_l.hh>
#include <labdev/devices/musashi/ml-808gx.hh>

namespace py = pybind11;

using labdev::interface;
using labdev::tcpip_interface;

using labdev::device;
using labdev::oscilloscope;
using labdev::xenax_xvi_75v8;
using labdev::om70_l;
using labdev::ml_808gx;

PYBIND11_MODULE(pylabdev, m) {
    m.doc() = "Library to communicate with lab devices.";

    /*   I N T E R F A C E S   */

    // Interface base class
    py::class_<interface>(m, "interface")
        .def("write", &interface::write, 
            "write string", 
            py::arg("msg"))
        .def("read", &interface::read, 
            "read string", 
            py::arg("timeout_ms") = 1000)
        .def("query", &interface::query, 
            "write string and read response", 
            py::arg("msg"),
            py::arg("timeout_ms") = 1000)
    ;

    // TCP/IP interface
    py::class_<tcpip_interface, interface>(m, "tcpip_interface")
        .def(py::init())
        .def(py::init<const std::string&, unsigned>(),
            py::arg("ip_addr"), 
            py::arg("port") = 0)
        .def("open", &tcpip_interface::open,
            "Open TCP/IP socket with given IP and port",
            py::arg("ip_addr"),
            py::arg("port"))
        .def("close", &tcpip_interface::close,
            "Close TCP/IP socket")
        .def("write_raw", &tcpip_interface::write_raw, 
            "write characters",
            py::arg("data"), 
            py::arg("len"))
        .def("read_raw", &tcpip_interface::read_raw, 
            "read characters",
            py::arg("data"), 
            py::arg("max_len"), 
            py::arg("timeout_ms") = 1000)
        .def("get_info", &tcpip_interface::get_info, 
            "returns ip and port")
        .def("connected", &tcpip_interface::connected, 
            "returns true if socket is ready for io")
    ;

    /*   D E V I C E S   */

    // Device base class
    py::class_<device>(m, "device")
        .def(py::init())
        .def("good", &device::good,
            "Returns true if device is ready for communication")
        .def("get_info", &device::get_info,
            "Returns human readable information string about the interface")
    ;

    // Oscilloscope base class
    py::class_<oscilloscope, device>(m, "oscilloscope");

    // Rigol DS1000Z
/*
    // Xenax XVI motor controller
    py::class_<xenax_xvi_75v8, device>(m, "xenax_xvi")
        .def(py::init<>())
        .def(py::init<tcpip_interface*>())
        //.def("open", 
        //    static_cast<void (xenax_xvi_75v8::*)(const ip_address &)> 
        //        (&xenax_xvi_75v8::open), 
        //    "Open device with given ip address")
        .def("__repr__", 
            [](const xenax_xvi_75v8 &self) {
                return "XENAX Xvi 75V8 servo motor controller, "
                + self.get_info();
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
    ;

    // Baumer laser distance sensor OM70
    py::class_<om70_l>(m, "om70_l")
        .def(py::init<>())
        .def(py::init<const ip_address &>())
        .def("open", 
            &om70_l::open, 
            "Open device with given ip address")
        .def("__repr__", 
            [](const om70_l &self) {
                return "OM70-l laser distance sensor, "
                + self.get_info();
            })
        .def("get_distance", 
            static_cast<float (om70_l::*)()>(&om70_l::get_distance), 
            "Returns distance in mm")
    ;

    // Musashi time-pressure dispenser ML-808 GX
    py::class_<ml_808gx>(m, "ml_808gx")
        .def(py::init())
        .def(py::init<const serial_config &>())
        .def("open", 
            &ml_808gx::open, 
            "Open device with given serial configuration")
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
    ;
*/
}