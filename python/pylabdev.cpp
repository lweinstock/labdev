#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <tuple>

#include <labdev/ld_interface.hh>
#include <labdev/tcpip_interface.hh>

#include <labdev/devices/ld_device.hh>
#include <labdev/devices/jenny-science/xenax_xvi.hh>
#include <labdev/devices/baumer/om70_l.hh>
#include <labdev/devices/musashi/ml_808gx.hh>

namespace py = pybind11;

using std::string;
using std::tuple;

using labdev::ld_interface;
using labdev::tcpip_interface;

using labdev::ld_device;
using labdev::xenax_xvi;
using labdev::om70_l;
using labdev::ml_808gx;

PYBIND11_MODULE(pylabdev, m) {
    m.doc() = "Library to communicate with lab devices.";

    /*   I N T E R F A C E S   */

    // Interface base class
    py::class_<ld_interface>(m, "ld_interface")
        .def("write",
            &ld_interface::write,
            "Write string",
            py::arg("msg"))
        .def("read",
            &ld_interface::read,
            "Read string",
            py::arg("timeout_ms") = 2000)
        .def("query",
            &ld_interface::query,
            "Send a query and return the resulting string",
            py::arg("msg"),
            py::arg("timeout_ms") = 2000)
        .def("good",
            &ld_interface::good,
            "Returns true if the interface is operable")
    ;

    // TCP/IP interface
    py::class_<tcpip_interface, ld_interface>(m, "tcpip_interface")
        .def(py::init<>(),
            "Create unconnected, empty TCP/IP interface")
        .def(py::init<string, unsigned>(),
            py::arg("ip_addr"),
            py::arg("port"),
            "Create TCP/IP interface to specified IP address and port")
        .def("open",
            static_cast<void (tcpip_interface::*)()>(&tcpip_interface::open),
            "Open TCP/IP socket with stored IP address and port")
        .def("open",
            static_cast<void (tcpip_interface::*)(std::string, unsigned)>
                (&tcpip_interface::open),
            "Open TCP/IP socket with specified IP address and port")
        .def("close",
            &tcpip_interface::close,
            "Close current TCP/IP socket")
        .def("write_raw",
            &tcpip_interface::write_raw,
            "Write array of bytes",
            py::arg("data"),
            py::arg("len"))
        .def("read_raw",
            &tcpip_interface::read_raw,
            "Read array of bytes",
            py::arg("data"),
            py::arg("max_len"),
            py::arg("timeout_ms") = 2000)
        .def("set_ip",
            &tcpip_interface::set_ip,
            "Store specified IP address")
        .def("get_ip",
            &tcpip_interface::get_ip,
            "Returns stored IP address")
        .def("set_port",
            &tcpip_interface::set_port,
            "Store specified port")
        .def("get_port",
            &tcpip_interface::get_port,
            "Returns stored port")
        .def("set_timeout",
            &tcpip_interface::set_timeout,
            "Set the read/write timeout in ms")
    ;

    /*   D E V I C E S   */

    // Device base class
    py::class_<ld_device>(m, "ld_device")
        .def("get_info", &ld_device::get_info,
            "Returns human readable information string about the interface")
    ;

    // Xenax XVI motor controller

    py::class_<xenax_xvi, ld_device> xenax_xvi(m, "xenax_xvi");
        xenax_xvi.def_property_readonly_static("dflt_port", 
            [](py::object) { return xenax_xvi::PORT; })
        .def(py::init<tcpip_interface*>(),
            py::arg("tcpip"))
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
            &xenax_xvi::reference_axis, 
            "Start referencing axis position")
        .def("set_reference_dir", 
            &xenax_xvi::set_reference_dir, 
            "Set direction of reference movement")
        .def("get_reference_dir", 
            &xenax_xvi::get_reference_dir, 
            "Returns the direction of reference movement")
        .def("is_referenced", 
            &xenax_xvi::reference_completed, 
            "Returns true if axis is referenced")
        .def("disable_smu", 
            &xenax_xvi::disable_smu, 
            "Disable safety motion unit")
        .def("move_position", 
            &xenax_xvi::move_position, 
            "Non-blocking movement to absolute position in incs",
            py::arg("pos"))
        .def("get_position", 
            &xenax_xvi::get_position, 
            "Returns absolute position in incs")
        .def("motion_completed", 
            &xenax_xvi::motion_completed, 
            "Returns true if axis has completed previous motion")
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
            static_cast<std::tuple<unsigned,std::string> (xenax_xvi::*)()>
                (&xenax_xvi::get_error),
            "Returns current error number")
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
        .def("set_sid",
            &xenax_xvi::set_sid,
            "Set custom servo identifier",
            py::arg("sid"))
        .def("reset_sid",
            &xenax_xvi::reset_sid,
            "Reset servo identifier")
        .def("get_sid",
            &xenax_xvi::get_sid,
            "Read servo identifier")
    ;

    // Output type enums
    py::enum_<xenax_xvi::output_type>(xenax_xvi, "output_type")
        .value("sink", xenax_xvi::output_type::SINK)
        .value("source", xenax_xvi::output_type::SOURCE)
        .value("sink_source", xenax_xvi::output_type::SINK_SOURCE)
    ;

    // Reference directions
    py::enum_<xenax_xvi::ref_dir>(xenax_xvi, "ref_dir")
        .value("ref_pos", xenax_xvi::ref_dir::REF_POS)
        .value("ref_neg", xenax_xvi::ref_dir::REF_NEG)
        .value("gantry_pos", xenax_xvi::ref_dir::GANTRY_POS)
        .value("gantry_neg", xenax_xvi::ref_dir::GANTRY_NEG)
        .value("gantry_pos_neg", xenax_xvi::ref_dir::GANTRY_POS_NEG)
        .value("gantry_neg_pos", xenax_xvi::ref_dir::GANTRY_NEG_POS)
    ;

    // Baumer laser distance sensor OM70
    py::class_<om70_l, ld_device>(m, "om70_l")
        .def_property_readonly_static("dflt_port", 
            [](py::object) { return om70_l::PORT; })
        .def(py::init<tcpip_interface*>())
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

 /*   
    // Musashi time-pressure dispenser ML-808 GX
    py::class_<ml_808gx, ld_device>(m, "ml_808gx")
        .def(py::init<tcpip_interface>())
        .def("__repr__", 
            [](const ml_808gx &self) {
                return "ML-808gx dispenser unit, "
                + self.get_info();
            })
        .def("dispense", 
            &ml_808gx::dispense, 
            "Dispense glue according to current mode (manual/timed) and "
            "channel/recipe")
        .def("set_channel", 
            &ml_808gx::set_channel, 
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
            static_cast<tuple<float, float, float, float> 
                (ml_808gx::*)()>(&ml_808gx::get_channel_params),
            "Returns parameters of current channel; pressure in kPa, duration "
            "in and on/off delay in ms")
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
            static_cast<tuple<float, float> 
                (ml_808gx::*)()> (&ml_808gx::get_delays),
            "Returns the on and off delay in ms")
    ;
*/

}
