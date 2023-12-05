#ifndef LD_SERIAL_INTERFACE_HH
#define LD_SERIAL_INTERFACE_HH

#include <labdev/ld_interface.hh>

namespace labdev{

class serial_interface : public ld_interface {
public:
    serial_interface() : ld_interface(), m_baud(9600), m_nbits(8), m_sbits(1), 
        m_par_en(false), m_par_even(false) {};
    ~serial_interface() {};

    // Set baud rate for serial interface
    virtual void set_baud(unsigned baud) { m_baud = baud; }
    virtual unsigned get_baud() const { return m_baud; }

    // Set number of data bits per packet
    virtual void set_nbits(unsigned nbits) { m_nbits = nbits; }
    virtual unsigned get_nbits() const { return m_nbits; }

    // Send 1 or 2 stop bits
    virtual void set_stop_bits(unsigned stop_bits) { m_sbits = stop_bits; }
    virtual unsigned get_stop_bits() const { return m_sbits; }

    // Enable and set parity
    virtual void set_parity(bool en = true, bool even = true) 
        { m_par_en = en; m_par_even = even; }
    virtual bool get_parity() const { return m_par_en; }
    virtual bool parity_even() const { return m_par_even; }

    // Apply changed settings
    virtual void apply_settings() = 0;

    // En-/disable hardware flow control
    virtual void enable_rts_cts() = 0;
    virtual void enable_dtr_dsr() = 0;
    virtual void disable_hw_flow_ctrl() = 0;

    // Data Terminal Ready (DTR) for manual flow control
    virtual void set_dtr() = 0;
    virtual void clear_dtr() = 0;

    // Request To Send (RTS) for manual flow control
    virtual void set_rts() = 0;
    virtual void clear_rts() = 0;

    Interface_type type() const final { return rs232; }

protected:
    unsigned m_baud, m_nbits, m_sbits ;
    bool m_par_en, m_par_even, m_update_settings;
};

}

#endif
