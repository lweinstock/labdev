#ifndef LD_USBTMC_INTERFACE_H
#define LD_USBTMC_INTERFACE_H

#include <labdev/usb_interface.hh>

namespace labdev{

class usbtmc_interface : public usb_interface {
public:
    usbtmc_interface() : usb_interface(), m_cur_tag(0x01) {};
    usbtmc_interface(uint16_t vid, uint16_t pid) 
        : usb_interface(vid, pid), m_cur_tag(0x01) {};
    usbtmc_interface(uint8_t bus, uint8_t port) 
        : usb_interface(bus, port), m_cur_tag(0x01) {};
    virtual ~usbtmc_interface() {};

    // USBTMC protocol definitions
    enum bRequest : uint16_t {
        INITIATE_ABORT_BULK_OUT     = 0x01,
        CHECK_ABORT_BULK_OUT_STATUS = 0x02,
        INITIATE_ABORT_BULK_IN      = 0x03,
        CHECK_ABORT_BULK_IN_STATUS  = 0x04,
        INITIATE_CLEAR              = 0x05,
        CHECK_CLEAR_STATUS          = 0x06,
        GET_CAPABILITIES            = 0x07,
        INDICATOR_PULSE             = 0x40
    };

    enum MsgID : uint16_t {
        DEV_DEP_MSG_OUT             = 0x01,
        REQUEST_DEV_DEP_MSG_IN      = 0x02,
        DEV_DEP_MSG_IN              = 0x02,
        VENDOR_SPECIFIC_OUT         = 0x7E,
        REQUEST_VENDOR_SPECIFIC_IN  = 0x7F,
        VENDOR_SPECIFIC_IN          = 0x7F
    };

    enum bmTransferAttributes : uint16_t {
        EOM = 0x01,
        TERM_CHAR = 0x02
    };

    int write_raw(const uint8_t* data, size_t len) override;
    int read_raw(uint8_t* data, size_t max_len, 
        unsigned timeout_ms = s_dflt_timeout_ms) override;

    Interface_type type() const override { return usbtmc; }

    // USBTMC device dependant data transfer
    int write_dev_dep_msg(const uint8_t* msg, size_t len,
        uint8_t transfer_attr = EOM);
    int read_dev_dep_msg(uint8_t* data, size_t max_len,
        int timeout_ms = s_dflt_timeout_ms, uint8_t transfer_attr = TERM_CHAR, 
        uint8_t term_char = '\n');

    // USBTMC vendor specific data transfer
    int write_vendor_specific(std::string msg);
    std::string read_vendor_specific(int timeout_ms = s_dflt_timeout_ms);

    // USBTMC clear Bulk-IN/OUT buffers
    void clear_buffer();

private:

    static constexpr unsigned s_header_len = 12;
    static constexpr uint8_t LIBUSB_SUBCLASS_TMC = 0x03;

    uint8_t m_cur_tag, m_term_char;

    // Creates a USBTMC header
    void create_usbtmc_header(uint8_t* header, uint8_t message_id,
    uint8_t transfer_attr, uint32_t transfer_size, uint8_t term_char = 0x00);

    // Extract data from header, check bTag fields, returns transfer length
    int check_usbtmc_header(uint8_t* message, uint8_t message_id);

};

}

#endif