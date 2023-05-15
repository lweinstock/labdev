#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <tuple>

#include <labdev/ld_interface.hh>
#include <labdev/serial_interface.hh>
#include <labdev/tcpip_interface.hh>
#include <labdev/usb_interface.hh>
#include <labdev/visa_interface.hh>

#include <labdev/devices/device.hh>
#include <labdev/devices/osci.hh>
#include <labdev/devices/jenny-science/xenax_xvi.hh>
#include <labdev/devices/baumer/om70_l.hh>
#include <labdev/devices/musashi/ml-808gx.hh>

namespace py = pybind11;

using std::string;
using std::tuple;

using labdev::serial_config;
using labdev::ip_address;
using labdev::usb_config;
using labdev::visa_identifier;

using labdev::device;
using labdev::xenax_xvi;
using labdev::om70_l;
using labdev::ml_808gx;

PYBIND11_MODULE(pylabdev, m) {
    m.doc() = "Library to communicate with lab devices.";

    /*   I N T E R F A C E   C O N F I G S   */

    py::class_<ip_address>(m, "ip_address")
        .def(py::init<>(),
            "Create empty IP address struct")
        .def(py::init<string, unsigned>(),
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
        .def(py::init<string, unsigned, unsigned, bool, bool, unsigned>(),
            py::arg("path"),
            py::arg("baud") = 9600,
            py::arg("nbits") = 8,
            py::arg("par_ena") = false,
            py::arg("par_even") = false,
            py::arg("stop_bits") = 1,
            "Create serial config with given arguments (default: BAUD 9600 8N1)"
        )
        .def_readwrite("path", &serial_config::path, 
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
        .def("get_info", &device::get_info,
            "Returns human readable information string about the interface")
    ;

    // Xenax XVI motor controller
    py::class_<xenax_xvi, device> xenax_xvi(m, "xenax_xvi");
        xenax_xvi.def_property_readonly_static("dflt_port", 
            [](py::object) { return xenax_xvi::PORT; })
        .def(py::init<const ip_address>())
        .def(py::init<const serial_config>())
        .def("__repr__", 
            [](const class xenax_xvi &self) {
                return self.get_info();
            })
        .def("power_on", 
            &xenax_xvi::power_on, 
            "Turn on motor power", 
            py::arg("enable") = true)
        .def("power_off", 
            &xenax_xvi::power_off, 
            "Turn off motor power")
        .def("reference_axis", 
            static_cast<void (xenax_xvi::*)()>(&xenax_xvi::reference_axis), 
            "Start referencing axis position")
        .def("reference_axis", 
            static_cast<void (xenax_xvi::*)(bool pos_dir)>(&xenax_xvi::reference_axis), 
            "Start referencing axis position in specified direction "
            "(true = positive, false = negative)")
        .def("is_referenced", 
            &xenax_xvi::is_referenced, 
            "Returns true if axis is referenced")
        .def("disable_smu", 
            &xenax_xvi::disable_smu, 
            "Disable safety motion unit")
        .def("move_position", 
            &xenax_xvi::move_position, 
            "Non-blocking movement to absolute position in incs",
            py::arg("pos"))
        .def("goto_position", 
            &xenax_xvi::goto_position, 
            "Blocking movement to absolute position in incs",
            py::arg("pos"),
            py::arg("interval_ms") = 500,
            py::arg("timeout_ms") = 10000)
        .def("get_position", 
            &xenax_xvi::get_position, 
            "Returns absolute position in incs")
        .def("in_motion", 
            &xenax_xvi::in_motion, 
            "Returns true if axis is moving")
        .def("in_position", 
            &xenax_xvi::in_position, 
            "Returns true if axis has reached its final position")
        .def("stop_motion", 
            &xenax_xvi::stop_motion, 
            "Immediatly stops all movement")
        .def("set_speed", 
            &xenax_xvi::set_speed, 
            "Set movement speed in incs/sec",
            py::arg("inc_per_sec"))
        .def("get_speed", 
            &xenax_xvi::get_speed, 
            "Returns current movement speed in incs/sec")
        .def("set_acceleration", 
            &xenax_xvi::set_acceleration, 
            "Set movement acceleration in incs/sec2",
            py::arg("inc_per_sec2"))
        .def("get_acceleration", 
            &xenax_xvi::get_acceleration, 
            "Returns current movement acceleration in incs/sec2")
        .def("set_s_curve", 
            &xenax_xvi::set_s_curve, 
            "Set acceleration curve in %",
            py::arg("percent"))
        .def("get_s_curve", 
            &xenax_xvi::get_s_curve, 
            "Returns current acceleration curve in %")
        .def("force_calibration",
            &xenax_xvi::force_calibration, 
            "Perform force calibration for force measurements", 
            py::arg("distance") = 1000)
        .def("get_motor_current",
            &xenax_xvi::get_motor_current, 
            "Returns the motor current in mA")
        .def("get_force_constant",
            &xenax_xvi::get_force_constant, 
            "Returns the force constant in N/mA")
        .def("get_motor_force",
            &xenax_xvi::get_motor_force, 
            "Returns the motor force in N")
        .def("set_force_limit",
            &xenax_xvi::set_force_limit, 
            "Set maximum force of axis in N",
            py::arg("flim_N"))
        .def("get_force_limit",
            &xenax_xvi::get_force_limit, 
            "Returns maximum force of axis in N")
        .def("reset_motor_type",
            &xenax_xvi::reset_motor_type, 
            "Required when motors are changed")
        .def("force_limit_reached", 
            &xenax_xvi::force_limit_reached, 
            "Returns true if force limit was reached")
        .def("get_error",
            &xenax_xvi::get_error,
            "Returns current error number")
        .def("get_strerror",
            &xenax_xvi::get_strerror,
            "Returns current error string")
        .def("set_output_type",
            &xenax_xvi::set_output_type,
            "Set output type for given PLC output",
            py::arg("output number"),
            py::arg("output type"))
        .def("set_output_activity",
            &xenax_xvi::set_output_activity,
            "Set output activity for given PLC output",
            py::arg("output number"),
            py::arg("output activity"))
        .def("set_output",
            &xenax_xvi::set_output,
            "Set output state for given PLC output",
            py::arg("output number"),
            py::arg("output state"))
    ;

    // Output type enums for Xenax Xvi 75v8
    py::enum_<xenax_xvi::output_type>(xenax_xvi, "output_type")
        .value("sink", xenax_xvi::output_type::SINK)
        .value("source", xenax_xvi::output_type::SOURCE)
        .value("sink_source", xenax_xvi::output_type::SINK_SOURCE)
    ;

    // Baumer laser distance sensor OM70
    py::class_<om70_l, device>(m, "om70_l")
        .def_property_readonly_static("dflt_port", 
            [](py::object) { return om70_l::PORT; })
        .def(py::init<const ip_address>())
        .def("__repr__", 
            [](const om70_l &self) {
                return self.get_info();
            })
        .def("enable_laser",
            &om70_l::enable_laser,
            "Enables the laser",
            py::arg("ena") = true)
        .def("disable_laser",
            &om70_l::disable_laser,
            "Disables the laser")
        .def("get_measurement", 
            &om70_l::get_measurement, 
            "Returns distance in mm, updates quality, sample rate, exposure "
            "reserve, and delay readings")
        .def("get_quality",
            &om70_l::get_quality,
            "Returns 0 (good), 1 (medium), 2 (bad)")
        .def("get_sample_rate",
            &om70_l::get_sample_rate,
            "Returns sample rate in Hz")
        .def("get_exposure",
            &om70_l::get_exposure,
            "Returns arb. unit, higher ist better")
        .def("get_measurement_mem", 
            &om70_l::get_measurement_mem, 
            "Returns distance in mm, updates quality, sample rate, exposure "
            "reserve, and delay readings")
        .def("get_quality_mem",
            &om70_l::get_quality_mem,
            "Returns 0 (good), 1 (medium), 2 (bad)")
        .def("get_sample_rate_mem",
            &om70_l::get_sample_rate_mem,
            "Returns sample rate in Hz")
        .def("get_exposure_mem",
            &om70_l::get_exposure_mem,
            "Returns arb. unit, higher ist better")
    ;

    // Musashi time-pressure dispenser ML-808 GX
    py::class_<ml_808gx, device>(m, "ml_808gx")
        .def(py::init<const serial_config>())
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
            static_cast<tuple<unsigned, unsigned, unsigned, unsigned> 
                (ml_808gx::*)()>(&ml_808gx::get_channel_params),
            "Returns parameters of current channel; pressure in 100 Pa, duration "
            "in ms, on/off delay in 0.1ms")
        .def("set_pressure",
            &ml_808gx::set_pressure,
            "Set pressure for current channel in units of 100 Pa",
            py::arg("pressure"))
        .def("set_duration",
            &ml_808gx::set_duration,
            "Set duration for current channel in ms",
            py::arg("duration"))
        .def("set_delays",
            &ml_808gx::set_delays,
            "Set on and off delays in units of 0.1 ms",
            py::arg("on_delay"),
            py::arg("off_delay"))
        .def("get_pressure",
            &ml_808gx::get_pressure,
            "Returns the pressure of current channel in units of 100 Pa")
        .def("get_duration",
            &ml_808gx::get_duration,
            "Returns the duration of current channel in ms")
        .def("get_delays",
            static_cast<tuple<unsigned, unsigned> 
                (ml_808gx::*)()> (&ml_808gx::get_delays),
            "Returns the on and off delay in units of 0.1 ms")
    ;
}
