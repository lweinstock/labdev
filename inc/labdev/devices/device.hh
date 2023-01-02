#ifndef DEVICE_HH
#define DEVICE_HH

#include <labdev/interface.hh>

namespace labdev {

    /*
     *  Abstract class for basic devices
     */

    class device {
    public:
        device() : m_comm(nullptr) {};
        device(const device&) = delete;
        virtual ~device() { this->disconnect(); }

        // Does a communication interface exist and is it connected?
        bool connected() const { return (m_comm ? m_comm->good() : false); }

        void disconnect();

        // Returns human readable information string about the interface
        std::string get_info() const;

    protected:
        interface* m_comm;
    };
}

#endif