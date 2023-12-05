#ifndef LD_LIBUSB_RAW_H
#define LD_LIBUSB_RAW_H

#include <labdev/ld_interface.hh>
#include <libusb.h>

namespace labdev{

class libusb_raw : public ld_interface {
public:
    libusb_raw();
    libusb_raw(uint16_t vid, uint16_t pid, std::string serno = "");
    virtual ~libusb_raw();

    int write_raw(const uint8_t* data, size_t len) override;
    int read_raw(uint8_t* data, size_t max_len, 
        unsigned timeout_ms = s_dflt_timeout_ms) override;

    void open() override;
    void open(uint16_t vid, uint16_t pid, std::string serno = "");
    void close() override;

    std::string get_info() const override;
    Interface_type type() const override { return usb; }

    // libusb-style data transfer to control endpoint (ep0)
    int write_control(uint8_t request_type, uint8_t request, uint16_t value,
        uint16_t index, const uint8_t* data, int len);
    int read_control(uint8_t request_type, uint8_t request, uint16_t value,
        uint16_t index, const uint8_t* data, int len);

    // libusb-style data transfer to bulk endpoints
    int write_bulk(const uint8_t* data, int len);
    int read_bulk(uint8_t* data, int max_len, 
        int timeout_ms = s_dflt_timeout_ms);

    // libusb-style data transfer to interrupt endpoints
    int write_interrupt(const uint8_t* data, int len);
    int read_interrupt(uint8_t* data, int max_len, 
        int timeout_ms = s_dflt_timeout_ms);

    // Set current I/O configuration
    void claim_interface(int int_no, int alt_setting = 0);
    void set_endpoint_in(unsigned ep_no);
    void set_endpoint_out(unsigned ep_no);

    // Clear endpoint buffers
    void clear();

    // Set/get vendor ID
    void set_vid(uint16_t vid) { m_vid = vid; }
    uint16_t get_vid() const { return m_vid; }
    // Set/get product ID
    void set_pid(uint16_t pid) { m_pid = pid; }
    uint16_t get_pid() const { return m_pid; }
    // Set/get serial number
    void set_serial(std::string serno) { m_serno = serno; }
    std::string get_serial() const { return m_serno; }

    // Get bus:port
    uint8_t get_bus() const { return m_bus; }
    uint8_t get_port() const { return m_port; }

    // Get information on the current interface
    uint8_t get_interface_class() const { return m_interface_class; }
    uint8_t get_interface_subclass() const { return m_interface_subclass; }
    uint8_t get_interface_protocol() const { return m_interface_protocol; }

protected:
    static const int s_no_interface = -1;
    static libusb_context* s_default_ctx;
    static int s_dev_count;
    libusb_device* m_usb_dev;
    libusb_device_handle* m_usb_handle;

    // Current device I/O information
    int m_cur_cfg, m_cur_alt_setting, m_cur_interface_no;
    uint8_t m_ep_in_addr, m_ep_out_addr;
    size_t m_max_pkt_size_in, m_max_pkt_size_out;

    // Device info
    uint16_t m_vid, m_pid;
    std::string m_serno;
    uint8_t m_bus, m_port;
    uint8_t m_dev_class, m_dev_subclass, m_dev_protocol;
    uint8_t m_interface_class, m_interface_subclass, m_interface_protocol;

    // Reads information from device descriptors
    void gather_device_information();

    // Reads information from interface descriptors
    void gather_interface_information();

    void check_and_throw(int status, const std::string& msg) const;
    void check_interface();
};

}

#endif
