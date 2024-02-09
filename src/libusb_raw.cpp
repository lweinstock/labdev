#include <labdev/libusb_raw.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std;

namespace labdev {

libusb_context* libusb_raw::s_default_ctx = NULL;
int libusb_raw::s_dev_count = 0;


libusb_raw::libusb_raw(): m_usb_dev(NULL), m_usb_handle(NULL),
    m_cur_cfg(0), m_cur_alt_setting(0), m_cur_interface_no(-1),
    m_ep_in_addr(0), m_ep_out_addr(0), m_max_pkt_size_in(64), 
    m_max_pkt_size_out(64), m_vid(0x0000), m_pid(0x0000), m_serno(""), 
    m_bus(0xFF), m_port(0xFF), m_dev_class(0), m_dev_subclass(0), 
    m_dev_protocol(0), m_interface_class(0), m_interface_subclass(0), 
    m_interface_protocol(0) 
{
    return;
}

libusb_raw::libusb_raw(uint16_t vid, uint16_t pid, string serno) 
    : libusb_raw() 
{
    this->open(vid, pid, serno);
    return;
}

libusb_raw::~libusb_raw()
{
    this->close();
    return;
}

int libusb_raw::write_raw(const uint8_t* data, size_t len)
{
    return this->write_bulk(data, len);
}

int libusb_raw::read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms)
{
    return this->read_bulk(data, max_len, timeout_ms);
}

void libusb_raw::open()
{
    this->open(m_vid, m_pid, m_serno);
    return;
}

void libusb_raw::open(uint16_t vid, uint16_t pid, string serno)
{
    int stat;
    // If first device, start new libusb session
    if (s_dev_count == 0) {
        stat = libusb_init(&s_default_ctx);
        check_and_throw(stat, "libusb init failed");
        debug_print("new libusb session initialized (%i)\n", stat);
    }

    // Search for deivce with given VID & PID
    libusb_device** dev_list;
    int ndev = libusb_get_device_list(s_default_ctx, &dev_list);
    check_and_throw(ndev, "Failed to get device list");

    for (int idev = 0; idev < ndev; idev++) {
        // Get dev descriptor for vid, pid, bus, and port
        libusb_device* tmp_dev = dev_list[idev];
        libusb_device_descriptor desc;
        stat = libusb_get_device_descriptor(tmp_dev, &desc);
        check_and_throw(stat, "Failed to get device descriptor");

        debug_print("device %i: VID:PID=0x%04X:0x%04X\n", 
            idev, desc.idVendor, desc.idProduct);

        // Check for VID and PID
        if ( (desc.idVendor == vid) && (desc.idProduct == pid) ) {
            // Also check for serial number, if specified
            if ( !serno.empty() ) {
                libusb_device_handle* tmp_handle;
                stat = libusb_open(tmp_dev, &tmp_handle);
                check_and_throw(stat, "Failed to get usb handle");

                uint8_t serial_cstr[126];
                stat = libusb_get_string_descriptor_ascii(tmp_handle,
                    desc.iSerialNumber, serial_cstr, 126);
                check_and_throw(stat, "Failed to get string descriptor");
                debug_print("SerialNumber %s\n", serial_cstr);
                string serial_str( (char*)serial_cstr );

                // If serial number does not match, skip rest
                if (serno != serial_str) continue;
            }
            m_usb_dev = tmp_dev;
            m_vid = vid;
            m_pid = pid;
            m_serno = serno;
            m_bus = libusb_get_bus_number(m_usb_dev);
            m_port = libusb_get_port_number(m_usb_dev);
            break;
        }
    }

    // Get I/O handle
    if (m_usb_dev) {
        stat = libusb_open(m_usb_dev, &m_usb_handle);
        check_and_throw(stat, "Failed to get usb handle");
        this->gather_device_information();
    } else {
        fprintf(stderr, "Device ID 0x%04X:0x%04X not found\n", vid, pid);
        abort();
    }
    s_dev_count++;
    debug_print("Opened device, new device count = %i\n", s_dev_count);

    libusb_free_device_list(dev_list, 1);
    m_good = true;
    return;
}

void libusb_raw::close()
{
    // Release claimed interfaces and device
    if (m_cur_interface_no != s_no_interface)
        libusb_release_interface(m_usb_handle, m_cur_interface_no);
    if (m_usb_handle)
        libusb_close(m_usb_handle);
    m_cur_interface_no = s_no_interface;
    // Close context if last device released
    s_dev_count--;
    debug_print("Closed device, new device count = %i\n", s_dev_count);
    if (s_dev_count == 0) {
        debug_print("%s\n", "Last device closed, exiting libusb\n");
        libusb_exit(s_default_ctx);
    }

    m_good = false;
    return;
}

string libusb_raw::get_info() const
{
    stringstream info("");
    info << std::uppercase << setfill('0') << setw(4) << std::hex;
    info << "usb;0x" << m_vid << m_pid << m_serno;
    return info.str();
}

void libusb_raw::clear()
{
    libusb_clear_halt(m_usb_handle, m_ep_in_addr);
    libusb_clear_halt(m_usb_handle, m_ep_out_addr);
    return;
}

int libusb_raw::write_control(uint8_t request_type, uint8_t request,
    uint16_t value, uint16_t index, const uint8_t* data, int len) 
{
    this->check_interface();
    int nbytes = libusb_control_transfer(
        m_usb_handle,
        LIBUSB_ENDPOINT_OUT | request_type,
        request,
        value,
        index,
        (uint8_t*)data,
        len,
        1000);
    check_and_throw(nbytes, "Control transfer failed to send data");

    // Verbose debug print
    debug_print("Send %i bytes : ", nbytes);
    #ifdef LD_DEBUG
    for (int i = 0; i < nbytes; i++)
        printf("0x%02X ", data[i]);
    printf("\n");
    #endif

    return nbytes;
}

int libusb_raw::read_control(uint8_t request_type, uint8_t request, 
    uint16_t value, uint16_t index, const uint8_t* data, int len) 
{
    this->check_interface();
    int nbytes = libusb_control_transfer(
        m_usb_handle,
        LIBUSB_ENDPOINT_IN | request_type,
        request,
        value,
        index,
        (uint8_t*)data,
        len,
        1000);
    check_and_throw(nbytes, "Control transfer failed to read data");

    // Verbose debug print
    debug_print("read %i bytes : ", nbytes);
    #ifdef LD_DEBUG
    for (int i = 0; i < nbytes; i++)
        printf("0x%02X ", data[i]);
    printf("\n");
    #endif

    return nbytes;
}


int libusb_raw::write_bulk(const uint8_t* data, int len) 
{
    this->check_interface();
    int stat, nbytes = 0;
    size_t bytes_left = len;
    size_t bytes_written = 0;

    while ( bytes_left > 0 ) {
        // Packets can be max wMaxPacketSize
        stat = libusb_bulk_transfer(
            m_usb_handle,
            m_ep_out_addr,
            (uint8_t*)&data[bytes_written],
            min(bytes_left, m_max_pkt_size_out),
            &nbytes,
            1000);
        check_and_throw(stat, "Bulk transfer failed to send data");
        bytes_left -= nbytes;
        bytes_written += nbytes;
        debug_print_byte_data(data, nbytes, "Written %zu bytes: ", nbytes);
    }
    return nbytes;
}

int libusb_raw::read_bulk(uint8_t* data, int max_len, int timeout_ms) 
{
    this->check_interface();
    int nbytes = 0;
    int stat = libusb_bulk_transfer(
        m_usb_handle,
        m_ep_in_addr,
        data,
        max_len,
        &nbytes,
        timeout_ms);
    check_and_throw(stat, "Bulk transfer failed to read data");
    debug_print_byte_data(data, nbytes, "Read %zu bytes: ", nbytes);
    return nbytes;
}

int libusb_raw::write_interrupt(const uint8_t* data, int len) 
{
    // TODO
    return 0;
}

int libusb_raw::read_interrupt(uint8_t* data, int max_len,
    int timeout_ms) 
{
    // TODO
    return 0;
}

void libusb_raw::claim_interface(int interface_no, int alt_setting) 
{
    int stat;
    string msg("");
    // Release current interface
    if ( (interface_no != m_cur_interface_no) &&
            (m_cur_interface_no != s_no_interface) ) {
        stat = libusb_release_interface(m_usb_handle, m_cur_interface_no);
        msg = "Failed to release interface " + to_string(m_cur_interface_no);
        check_and_throw(stat, msg);
    }
    // Detach kernel drivers if active
    if ( libusb_kernel_driver_active(m_usb_handle, interface_no) ) {
        stat = libusb_detach_kernel_driver(m_usb_handle, interface_no);
        check_and_throw(stat, "Failed to detach kernel driver from interface");
    }
    // Claim new interface
    stat = libusb_claim_interface(m_usb_handle, interface_no);
    msg = "Failed to claim interface " + to_string(interface_no);
    check_and_throw(stat, msg);
    m_cur_interface_no = interface_no;
    debug_print("Successfully claimed interface %i\n", m_cur_interface_no);

    // Apply alternate settingsm if specified
    // (FYI: most devices dont use alt settings)
    if (alt_setting) {
        stat = libusb_set_interface_alt_setting(m_usb_handle,
            m_cur_interface_no, alt_setting);
        msg = "Failed to apply alternate settings " + to_string(alt_setting);
        check_and_throw(stat, msg);
        m_cur_alt_setting = alt_setting;
        debug_print("Applied alternate settings %i\n", m_cur_alt_setting);
    }

    this->gather_interface_information();
    return;
}

void libusb_raw::set_endpoint_in(unsigned ep_no) 
{
    this->check_interface();

    // Get endpoint descriptor
    libusb_config_descriptor* cfg_desc;
    const libusb_interface* usb_interface;
    const libusb_interface_descriptor* int_desc;
    const libusb_endpoint_descriptor* ep_desc;

    int stat = libusb_get_config_descriptor(m_usb_dev, m_cur_cfg, &cfg_desc);
    check_and_throw(stat, "Failed to get config descriptor");
    usb_interface = &cfg_desc->interface[m_cur_interface_no];
    int_desc = &usb_interface->altsetting[m_cur_alt_setting];

    unsigned n_eps = static_cast<unsigned>(int_desc->bNumEndpoints);
    if (ep_no < n_eps)
        ep_desc = &int_desc->endpoint[ep_no];
    else 
        throw bad_io("Invalid endpoint number", ep_no);
    // Get endpoint address
    uint8_t ep_addr = ep_desc->bEndpointAddress;
    if (ep_addr & LIBUSB_ENDPOINT_IN)
        m_ep_in_addr = ep_addr;
    else
        throw bad_io("Wrong endpoint direction", ep_no);
    // Get max packet size
    m_max_pkt_size_in = ep_desc->wMaxPacketSize;
    debug_print("Endpoint%u (IN):\taddr 0x%02X, wMaxPacketSize %lu\n", ep_no, 
        m_ep_in_addr, ep_desc->wMaxPacketSize);
    return;
}

void libusb_raw::set_endpoint_out(unsigned ep_no) 
{
    this->check_interface();

    // Get endpoint descriptor
    libusb_config_descriptor* cfg_desc;
    const libusb_interface* usb_interface;
    const libusb_interface_descriptor* int_desc;
    const libusb_endpoint_descriptor* ep_desc;

    int stat = libusb_get_config_descriptor(m_usb_dev, m_cur_cfg, &cfg_desc);
    check_and_throw(stat, "Failed to get config descriptor");
    usb_interface = &cfg_desc->interface[m_cur_interface_no];
    int_desc = &usb_interface->altsetting[m_cur_alt_setting];

    unsigned n_eps = static_cast<unsigned>(int_desc->bNumEndpoints);
    if (ep_no < n_eps)
        ep_desc = &int_desc->endpoint[ep_no];
    else 
        throw bad_io("Invalid endpoint number", ep_no);
    // Get endpoint address
    uint8_t ep_addr = ep_desc->bEndpointAddress;
    if ( !(ep_addr & LIBUSB_ENDPOINT_IN) )
        m_ep_out_addr = ep_addr;
    else 
        throw bad_io("Wrong endpoint direction", ep_no);
    // Get max packet size
    m_max_pkt_size_out = ep_desc->wMaxPacketSize;
    debug_print("Endpoint%u (OUT):\taddr 0x%02X, wMaxPacketSize %lu\n", ep_no, 
        m_ep_out_addr, m_max_pkt_size_out);
    return;
}

/*
 *      P R I V A T E   M E T H O D S
 */

void libusb_raw::gather_device_information() 
{
    if (!m_usb_dev)
        return;

    // Get bus address
    m_bus = libusb_get_bus_number(m_usb_dev);
    m_port = libusb_get_port_number(m_usb_dev);

    // Get device info
    libusb_device_descriptor desc;
    int stat = libusb_get_device_descriptor(m_usb_dev, &desc);
    check_and_throw(stat, "Failed to get device descriptor");
    m_vid = desc.idVendor;
    m_pid = desc.idProduct;
    m_dev_class = desc.bDeviceClass;
    m_dev_subclass = desc.bDeviceSubClass;
    m_dev_protocol = desc.bDeviceProtocol;
/*
    char strdesc_buf[100] = {'\0'};
    if (desc.iSerialNumber) {
        stat = libusb_get_string_descriptor_ascii(m_usb_handle,
            desc.iSerialNumber, (uint8_t*)strdesc_buf, 100);
        check_and_throw(stat, "Failed to get string descriptor");
        m_serno = strdesc_buf;
    } else
        m_serno = "";
*/
    debug_print("Bus\t%i\n", m_bus);
    debug_print("Port\t%i\n", m_port);
    debug_print("idVendor\t0x%02X\n", m_vid);
    debug_print("idProduct\t0x%02X\n", m_pid);
    debug_print("iSerialNumber\t%s\n", m_serno.c_str());
    debug_print("bDeviceClass\t0x%02X\n", m_dev_class);
    debug_print("bDeviceSubClass\t0x%02X\n", m_dev_subclass);
    debug_print("bDeviceProtocol\t0x%02X\n", m_dev_protocol);

    return;
}

void libusb_raw::gather_interface_information() 
{
    this->check_interface();

    libusb_config_descriptor* cfg_desc;
    const libusb_interface* usb_interface;
    const libusb_interface_descriptor* int_desc;

    // Currently only looking at config 0 (TODO)
    // Only very few devices use multiple configs
    int stat = libusb_get_config_descriptor(m_usb_dev, m_cur_cfg, &cfg_desc);
    check_and_throw(stat, "Failed to get config descriptor");
    usb_interface = &cfg_desc->interface[m_cur_interface_no];
    int_desc = &usb_interface->altsetting[m_cur_alt_setting];

    // Get info
    m_interface_class = int_desc->bInterfaceClass;
    m_interface_subclass = int_desc->bInterfaceSubClass;
    m_interface_protocol = int_desc->bInterfaceProtocol;
    debug_print("bInterfaceClass\t0x%02X\n", m_interface_class);
    debug_print("bInterfaceSubClass\t0x%02X\n", m_interface_subclass);
    debug_print("bInterfaceProtocol\t0x%02X\n", m_interface_protocol);

    return;
}

void libusb_raw::check_and_throw(int stat, const string& msg) const 
{
    if (stat < 0) {
        stringstream err_msg;
        err_msg << msg << " (" << libusb_error_name(stat) << ", " << stat << ")";
        debug_print("%s\n", err_msg.str().c_str());

        switch (stat) {
        case LIBUSB_ERROR_TIMEOUT:
        case LIBUSB_ERROR_BUSY:
            throw timeout(err_msg.str().c_str(), stat);
            break;

        case LIBUSB_ERROR_OVERFLOW:
        case LIBUSB_ERROR_IO:
        case LIBUSB_ERROR_NO_MEM:
        case LIBUSB_ERROR_OTHER:
            throw bad_io(err_msg.str().c_str(), stat);
            break;

        case LIBUSB_ERROR_ACCESS:
        case LIBUSB_ERROR_NO_DEVICE:
            throw bad_connection(err_msg.str().c_str(), stat);
            break;

        case LIBUSB_ERROR_INVALID_PARAM:
        case LIBUSB_ERROR_NOT_SUPPORTED:
        default:
            fprintf(stderr, "%s\n", err_msg.str().c_str());
            abort();
        }
    }
    return;
}

void libusb_raw::check_interface() 
{
    if (m_cur_interface_no == s_no_interface)
        throw bad_io("No USB interface claimed");
    return;
}

}