#ifndef LD_SERIAL_INTERFACE_HH
#define LD_SERIAL_INTERFACE_HH

#include <labdev/ld_iface.hh>

namespace labdev{

/*! \brief Abstract base class for serial communication interfaces.
 *
 *  Another layer of abstraction used for serial communication interfaces 
 *  (e.g. RS232, RS422, RS485, UART, etc.). Derived classes must implement
 *  baud rate, number of data and stop bits per message, parity settings and
 *  so on. This layer of abstraction can be used to implement adapters
 *  (e.g. an ethernet-to-serial converter).
 *  \sa labdev::eth_to_ser
 */

class serial_iface : public ld_iface {
public:
    serial_iface() : ld_iface(), m_baud(9600), m_nbits(8), m_sbits(1), 
        m_par_en(false), m_par_even(false) {};
    ~serial_iface() {};

    //! Set baud rate for serial interface.
    virtual void set_baud(unsigned baud) { m_baud = baud; }
    //! Get baud rate for serial interface.
    virtual unsigned get_baud() const { return m_baud; }

    //! Set number of data bits per packet
    virtual void set_nbits(unsigned nbits) { m_nbits = nbits; }
    //! Get number of data bits per packet
    virtual unsigned get_nbits() const { return m_nbits; }

    //! Send 1 or 2 stop bits
    virtual void set_stop_bits(unsigned stop_bits) { m_sbits = stop_bits; }
    //! Returns the number of stop bits
    virtual unsigned get_stop_bits() const { return m_sbits; }

    /*! \brief Enable and set parity
     *  \param en True/false = use/don't use parity bit.
     *  \param even True/false = use even/odd parity.
     */
    virtual void set_parity(bool en = true, bool even = true) 
        { m_par_en = en; m_par_even = even; }
    //! Returns true if parity is used.
    virtual bool get_parity() const { return m_par_en; }
    //! Returns true if even parity is used.
    virtual bool parity_even() const { return m_par_even; }

    //! Apply changed settings.
    virtual void apply_settings() = 0;

    //! \brief Enable hardware flow control; use Request To Send (RTS) and 
    //! Clear To Send (CTS) signals for flow control.
    virtual void enable_rts_cts() = 0;
    //! \brief Enable hardware flow control; use Data Terminal Ready (DTR) and 
    //! Data Set Ready (DSR) signals for flow control.
    virtual void enable_dtr_dsr() = 0;
    //! Disable hardware flow control.
    virtual void disable_hw_flow_ctrl() = 0;

    //! Set Data Terminal Ready (DTR) for manual flow control.
    virtual void set_dtr() = 0;
    //! Clear Data Terminal Ready (DTR) for manual flow control.
    virtual void clear_dtr() = 0;

    //! Set Request To Send (RTS) for manual flow control
    virtual void set_rts() = 0;
    //! Clear Request To Send (RTS) for manual flow control
    virtual void clear_rts() = 0;

    Interface_type type() const noexcept { return SERIAL; }

protected:
    unsigned m_baud, m_nbits, m_sbits ;
    bool m_par_en, m_par_even, m_update_settings;
};

}

#endif