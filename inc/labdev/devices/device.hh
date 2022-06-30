#ifndef DEVICE_HH
#define DEVICE_HH

#include <labdev/interface.hh>

namespace labdev {

    /*
     *  Interface identifiers for device constructors
     */

    struct serial_config {
        serial_config() : 
            path(""), baud(9600), nbits(8), stop_bits(1), 
            par_en(false), par_even(false) {};
        serial_config(std::string path, unsigned baud = 9600, 
            unsigned nbits = 8, bool par_en = false, bool par_even = false, 
            unsigned stop_bits = 1) :
            path(path), baud(baud), nbits(nbits), stop_bits(stop_bits), 
            par_en(par_en), par_even(par_even) {};
        std::string path;
        unsigned baud, nbits, stop_bits;
        bool par_en, par_even;
    };

    struct ip_address { 
        ip_address() : ip(""), port(0) {};
        ip_address(std::string ip_addr, unsigned port = 0) : 
            ip(ip_addr), port(port) {};
        std::string ip;
        unsigned port;
    };

    struct visa_identifier { 
        visa_identifier() : str("") {};
        visa_identifier(std::string visa_id) : str(visa_id) {};
        std::string str; 
    }; 

    struct usb_id {
        usb_id(uint16_t vendor_id, uint16_t product_id, std::string ser_no="") : 
            serno(ser_no), vid(vendor_id), pid(product_id), bus(0x00), 
            port(0x00) {};
        usb_id(uint8_t bus_no = 0x00, uint8_t port_no = 0x00) : serno(""), 
            vid(0x0000), pid(0x0000), bus(bus_no), port(port_no) {};
        std::string serno;
        uint16_t vid, pid;
        uint8_t bus, port;
    };

    /*
     *  Abstract base class for devices
     */

    class device {
    public:
        device() : m_comm(nullptr) {};
        device(const device&) = delete;
        virtual ~device();

        // Does a communication interface exist and is it connected?
        bool good();
        
        // Close the communication interface
        virtual void close();

        // Returns pointer to current interface
        interface* get_interface();

    protected:
        interface* m_comm;
    };
}

#endif