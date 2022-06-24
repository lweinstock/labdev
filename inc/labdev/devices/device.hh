#ifndef DEVICE_HH
#define DEVICE_HH

#include <labdev/interface.hh>

namespace labdev {

    /*
     *  Interface identifiers for device constructors
     */

    struct ip_address { 
        ip_address() : str("") {};
        ip_address(std::string ip_addr) : str(ip_addr) {};
        std::string str; 
    };

    struct visa_identifier { 
        visa_identifier() : str("") {};
        visa_identifier(std::string visa_id) : str(visa_id) {};
        std::string str; 
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