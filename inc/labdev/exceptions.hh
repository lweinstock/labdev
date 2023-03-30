#ifndef LD_EXCEPTION_HH
#define LD_EXCEPTION_HH

#include <exception>
#include <string>

namespace labdev {

class exception : public std::exception {
public:
    exception(const std::string& msg, int err = 0):
        std::exception(),
        m_err_message(msg),
        m_err_number(err) {
    }

    virtual ~exception() noexcept {};

    virtual const char* what() const noexcept override {
        return m_err_message.c_str();
    }

    int error_number() const noexcept { return m_err_number; }

private:
    std::string m_err_message;
    int m_err_number;
};

// Timeout condition
class timeout : public exception {
public:
    timeout(const std::string& msg, int err = 0):
        exception(msg, err) {};
    virtual ~timeout() {};
};

// Connection lost or cannot be established
class bad_connection : public exception {
public:
    bad_connection(const std::string& msg, int err = 0):
        exception(msg, err) {};
    virtual ~bad_connection() {};
};

// Generic IO error
class bad_io : public exception {
public:
    bad_io(const std::string& msg, int err = 0):
        exception(msg, err) {};
    virtual ~bad_io() {};
};

// Communication protocol violation
class bad_protocol : public exception {
public:
    bad_protocol(const std::string& msg, int err = 0):
        exception(msg, err) {};
    virtual ~bad_protocol() {};
};

// Device specific error condition
class device_error : public exception {
public:
    device_error(const std::string& msg, int err = 0):
        exception(msg, err) {};
    virtual ~device_error() {};
};

}

#endif
