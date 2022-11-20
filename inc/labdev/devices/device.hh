#ifndef DEVICE_HH
#define DEVICE_HH

#include <labdev/interface.hh>

namespace labdev {

    /*
     *  Abstract base class for devices
     */

    class device {
    public:
        device() : m_comm(nullptr) {};
        device(const device&) = delete;
        virtual ~device();

        // Does a communication interface exist and is it connected?
        bool good() const { return (m_comm ? m_comm->connected() : false); }
        
        // Close the communication interface
        virtual void close();

        // Returns human readable information string about the interface
        std::string get_info() const;

    protected:
        interface* m_comm;
    };
}

#endif