#include <pybind11/pybind11.h>

#include <labdev/interface.hh>
#include <labdev/tcpip_interface.hh>

namespace py = pybind11;
using labdev::interface;
using labdev::tcpip_interface;

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
            py::arg("timeout_ms") = interface::s_dflt_timeout_ms)
        .def("query", &interface::query, 
            "write string and read response", 
            py::arg("msg"),
            py::arg("timeout_ms") = interface::s_dflt_timeout_ms)
    ;

    // TCP/IP interface
    py::class_<tcpip_interface, interface>(m, "tcpip_interface")
        .def(py::init())
        .def(py::init<const std::string&, unsigned>(),
            py::arg("ip_addr"), 
            py::arg("port") = 0)
        .def("write_raw", &tcpip_interface::write_raw, 
            "write characters",
            py::arg("data"), 
            py::arg("len"))
        .def("read_raw", &tcpip_interface::read_raw, 
            "read characters",
            py::arg("data"), 
            py::arg("max_len"), 
            py::arg("timeout_ms") = interface::s_dflt_timeout_ms)
        .def("get_info", &tcpip_interface::get_info, 
            "returns ip and port")
        .def("connected", &tcpip_interface::connected, 
            "returns true if socket is ready for io")
        .def("close", &tcpip_interface::close,
            "closes tcp/ip socket")
    ;
}