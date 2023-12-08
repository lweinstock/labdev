#ifndef LD_ETH_TO_RS232_CPP
#define LD_ETH_TO_RS232_CPP

#include <labdev/eth_to_ser.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <string>
#include <sstream>
#include <unistd.h>

using namespace std;

namespace labdev {

eth_to_ser::eth_to_ser() 
    : serial_interface(), m_tcpip_cfg(), m_tcpip_ser(), m_ip_addr("0.0.0.0"), 
      m_port(0), m_flc(0)
{
    return;
}

eth_to_ser::eth_to_ser(std::string ip_addr, unsigned port, 
    unsigned baud, unsigned nbits, bool par_ena, bool par_even, 
    unsigned stop_bits) : eth_to_ser()
{
    this->open(ip_addr, port, baud, nbits, par_ena, par_even, stop_bits);
    return;
}

eth_to_ser::~eth_to_ser()
{
    return;
}

void eth_to_ser::open()
{
    this->open(m_ip_addr, m_port, m_baud, m_nbits, m_par_en, m_par_even, m_sbits);
    return;
}

void eth_to_ser::open(std::string ip_addr, unsigned port, unsigned baud, 
    unsigned nbits, bool par_ena, bool par_even, unsigned stop_bits)
{
    this->set_ip(ip_addr);
    this->set_port(port);
    // Config via http
    m_tcpip_cfg.open(m_ip_addr, HTTP_PORT);
    // Serial communication via "raw" tcpip
    m_tcpip_ser.open(m_ip_addr, m_port);
    this->set_baud(baud);
    this->set_nbits(nbits);
    this->set_parity(par_ena, par_even);
    this->set_stop_bits(stop_bits);
    this->disable_hw_flow_ctrl();
    this->apply_settings();

    m_good = m_tcpip_ser.good() && m_tcpip_ser.good();
    return;
}

void eth_to_ser::close()
{
    m_tcpip_ser.close();
    m_good = m_tcpip_ser.good();
    return;
}

int eth_to_ser::write_raw(const uint8_t* data, size_t len)
{
    if (m_update_settings) 
        this->apply_settings();
    return m_tcpip_ser.write_raw(data, len);
}

int eth_to_ser::read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms)
{
    if (m_update_settings) 
        this->apply_settings();
    return m_tcpip_ser.read_raw(data, max_len, timeout_ms);
}

std::string eth_to_ser::get_info() const
{
    // Format example: serial;192.168.1.100:5555;9600;8N1
    stringstream ret("");
    ret << "serial;" << m_ip_addr << ":" << m_port << ";" << m_baud << ";";
    ret << m_nbits << (m_par_en ? (m_par_even ? "E" : "O") : "N") << m_sbits;
    return ret.str();
}

void eth_to_ser::set_ip(std::string ip_addr)
{
    m_ip_addr = ip_addr;
    m_tcpip_cfg.set_ip(m_ip_addr);
    m_tcpip_ser.set_ip(m_ip_addr);
    debug_print("Set ip address to %s\n", m_ip_addr.c_str());
    return;
}

void eth_to_ser::set_port(unsigned port)
{
    m_port = port;
    m_tcpip_ser.set_port(m_port);
    debug_print("Set port to %u\n", m_port);
    return;
}

void eth_to_ser::set_baud(unsigned baud)
{
    m_baud = baud;
    debug_print("Set baudrate to %i\n", m_baud);
    return;
}


void eth_to_ser::set_nbits(unsigned nbits)
{
    debug_print("Set number of bits to %i\n", nbits);
    m_nbits = nbits;
    m_update_settings = true;
    return;
}

void eth_to_ser::set_stop_bits(unsigned stop_bits)
{
    debug_print("Set number of stop bits to %i\n", m_sbits);
    m_sbits = stop_bits;
    m_update_settings = true;
    return;
}

void eth_to_ser::set_parity(bool en, bool even)
{
    m_par_en = en;
    m_par_even = even;
    m_update_settings = true;
    debug_print("%s parity with %s parity\n",
        (m_par_en) ? "enabled" : "disabled", (m_par_even) ? "even" : "odd");
    return;
}

void eth_to_ser::apply_settings()
{
    debug_print("%s\n", "Applying settings to server");

    // http body
    string body("");
    body += "bdr=" + this->get_bdr(m_baud) + "&";
    body += "dtb=" + to_string(this->get_dtb(m_nbits)) + "&";
    body += "prt=" + to_string(this->get_prt(m_par_en, m_par_even)) + "&";
    body += "stb=" + to_string(this->get_stb(m_sbits)) + "&";
    body += "flc=" + to_string(m_flc) + "&";
    body += "rtp=&post=Submit";
    // http header
    string head = "POST /ok.html HTTP/1.1\r\n"
                  "Content-Length: " + to_string(body.size()) + "\r\n"
                  "\r\n";

    // TCP/IP server needs to restart
    m_tcpip_ser.close();

    m_tcpip_cfg.write(head + body);
    string ret = m_tcpip_cfg.read_until("</SCRIPT>");   // End of message
    if ( ret.find("OK") == string::npos )
        throw bad_protocol("Did not receive 'HTTP/1.1 200 OK'");
    m_tcpip_cfg.close();

    // Reconnect to server
    usleep(100e3);
    m_tcpip_ser.open();
    m_tcpip_cfg.open();

    m_update_settings = false;
    return;
}

void eth_to_ser::enable_rts_cts()
{
    debug_print("%s\n", "Enabled RTS/CTS flow control");
    m_flc = 1;
    return;
}

void eth_to_ser::enable_dtr_dsr()
{
    debug_print("%s\n", "Enabled DTR/DSR flow control");
    m_flc = 2;
    return;
}

void eth_to_ser::disable_hw_flow_ctrl()
{
    debug_print("%s\n", "Disabled hardware flow control");
    m_flc = 0;
    return;
}

void eth_to_ser::set_dtr()
{
    throw exception("Setting DTR is currently not supported by eth_to_ser");
    return;
}

void eth_to_ser::clear_dtr()
{
    throw exception("Clearing DTR is currently not supported by eth_to_ser");
    return;
}

void eth_to_ser::set_rts()
{
    throw exception("Setting RTS is currently not supported by eth_to_ser");
    return;
}

void eth_to_ser::clear_rts()
{
    throw exception("Clearing RTS is currently not supported by eth_to_ser");
    return;
}

/*
 *      P R I V A T E   M E T H O D S
 */

string eth_to_ser::get_bdr(unsigned baud)
{
    string bdr = "0";
    switch(baud) {
        case 1200:   bdr = "0";  break;
        case 2400:   bdr = "1";  break;
        case 4800:   bdr = "2";  break;
        case 7200:   bdr = "3";  break;
        case 9600:   bdr = "4";  break;
        case 14400:  bdr = "5";  break;
        case 19200:  bdr = "6";  break;
        case 28800:  bdr = "7";  break;
        case 38400:  bdr = "8";  break;
        case 57600:  bdr = "9";  break;
        case 76800:  bdr = "A"; break;
        case 115200: bdr = "B"; break;
        default:
        fprintf(stderr, "Baudrate %i is not supported\n", baud);
        abort();
    }
    return bdr;
}

unsigned eth_to_ser::get_dtb(unsigned nbits)
{
    unsigned dtb = 0;
    switch(nbits) {
        case 8:   dtb = 0; break;
        case 7:   dtb = 1; break;
        case 6:   dtb = 2; break;
        case 5:   dtb = 3; break;
        default:
        fprintf(stderr, "%i-bit format is not supported\n", nbits);
        abort();
    }
    return dtb;
}

unsigned eth_to_ser::get_prt(bool par_en, bool par_even)
{
    if (par_en && par_even)
        return 2;
    if (par_en)
        return 1;
    return 0;
}

unsigned eth_to_ser::get_stb(unsigned stop_bits)
{
    unsigned stb = 0;
    switch (stop_bits) {
        case 1: stb = 0; break;
        case 2: stb = 1; break;
        default:
        fprintf(stderr, "%i stop bits are not supported\n", stop_bits);
        abort();
    }
    return stb;
}

}

#endif