#include <labdev/modbus_iface.hh>
#include <labdev/exceptions.hh>

namespace labdev {

void modbus_iface::check_error_code(uint8_t error)
{
    switch (error) {
    case ERR1:
        throw bad_protocol("Function code not supported");
        break;
    case ERR2:
        throw bad_protocol("Starting address or last address not supported");
        break;
    case ERR3:
        throw bad_protocol("Quantity of registers not supported (range 1 - 125)");
        break;
    case ERR4:
        throw bad_protocol("No read access to registers");
        break;
    default:
        throw bad_protocol("Unknown error code " + std::to_string(error));
    }
    return; 
}

}