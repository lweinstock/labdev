#include <labdev/usb_interface.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <sstream>
#include <algorithm>

using namespace std;

namespace labdev {

libusb_context* usb_interface::s_default_ctx = NULL;
int usb_interface::s_dev_count = 0;

usb_interface::usb_interface(const usb_config conf) : usb_interface() 
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
        check_and_throw(ndev, "Failed to get device descriptor");
        uint8_t tmp_bus = libusb_get_bus_number(tmp_dev);
        uint8_t tmp_port = libusb_get_port_number(tmp_dev);

        debug_print("device %i: VID:PID=0x%04X:0x%04X (bus:port=%03u:%03u)\n", 
            idev, desc.idVendor, desc.idProduct, tmp_bus, tmp_port);

        // Check for bus and port number
        if ( (tmp_port == conf.port_no) && (tmp_bus == conf.bus_no) ) {
            m_usb_dev = tmp_dev;
            break;
        }

        // Check for VID and PID
        if ( (desc.idVendor == conf.vid) && (desc.idProduct == conf.pid) ) {
            m_usb_dev = tmp_dev;
            break;
        }
        return;
    }

    // Get I/O handle
    if (m_usb_dev) {
        stat = libusb_open(m_usb_dev, &m_usb_handle);
        check_and_throw(stat, "Failed to get usb handle");
        this->gather_device_information();
    } else {
        fprintf(stderr, "Device ID 0x%04X:0x%04X BUS%03u:PORT%03u not found\n", 
            conf.vid, conf.pid, conf.bus_no, conf.port_no);
        abort();
    }
    s_dev_count++;
    debug_print("Opened device, new device count = %i\n", s_dev_count);

    libusb_free_device_list(dev_list, 1);
    return;
}

usb_interface::~usb_interface()
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
    return;
}

int usb_interface::write_raw(const uint8_t* data, size_t len) 
{
    return this->write_bulk(data, len);
}

int usb_interface::read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms) 
{
    return this->read_bulk(data, max_len, timeout_ms);
}

string usb_interface::get_info() const 
{
    // Format example: usb;002;001
    string ret("usb;" + to_string(m_bus) + ";" + to_string(m_port) );
    return ret;
}

void usb_interface::clear()
{
    libusb_clear_halt(m_usb_handle, m_ep_in_addr);
    libusb_clear_halt(m_usb_handle, m_ep_out_addr);
    return;
}

int usb_interface::write_control(uint8_t request_type, uint8_t request,
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

int usb_interface::read_control(uint8_t request_type, uint8_t request, 
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


int usb_interface::write_bulk(const uint8_t* data, int len) 
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

int usb_interface::read_bulk(uint8_t* data, int max_len, int timeout_ms) 
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

int usb_interface::write_interrupt(const uint8_t* data, int len) 
{
    // TODO
    return 0;
}

int usb_interface::read_interrupt(uint8_t* data, int max_len,
    int timeout_ms) 
{
    // TODO
    return 0;
}

void usb_interface::claim_interface(unsigned interface_no, unsigned alt_setting) 
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

void usb_interface::set_endpoint_in(unsigned ep_no) 
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
    debug_print("Endpoint %u address (IN) 0x%02X (wMaxPacketSize %i)\n", ep_no, 
        m_ep_in_addr, ep_desc->wMaxPacketSize);
    return;
}

void usb_interface::set_endpoint_out(unsigned ep_no) 
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
    debug_print("Endpoint %u address (OUT) 0x%02X (wMaxPacketSize %lu)\n", ep_no, 
        m_ep_out_addr, m_max_pkt_size_out);
    return;
}

/*
 *      P R I V A T E   M E T H O D S
 */

usb_interface::usb_interface(): m_usb_dev(NULL), m_usb_handle(NULL),
    m_cur_cfg(0), m_cur_alt_setting(0), m_cur_interface_no(-1),
    m_ep_in_addr(0), m_ep_out_addr(0), m_max_pkt_size_in(64), m_max_pkt_size_out(64),
    m_vid(0x0000), m_pid(0x0000), m_serial_no(""), m_bus(0xFF), m_port(0xFF),
    m_dev_class(0), m_dev_subclass(0), m_dev_protocol(0),
    m_interface_class(0), m_interface_subclass(0), m_interface_protocol(0) 
{
    return;
}

void usb_interface::gather_device_information() 
{
    if (!m_usb_dev)
        return;

    // Get bus address
    m_bus = libusb_get_bus_number(m_usb_dev);
    m_port = libusb_get_port_number(m_usb_dev);

    // Get device info
    char strdesc_buf[100] = {'\0'};
    libusb_device_descriptor desc;
    int stat = libusb_get_device_descriptor(m_usb_dev, &desc);
    check_and_throw(stat, "Failed to get device descriptor");
    m_vid = desc.idVendor;
    m_pid = desc.idProduct;
    m_dev_class = desc.bDeviceClass;
    m_dev_subclass = desc.bDeviceSubClass;
    m_dev_protocol = desc.bDeviceProtocol;

    if (desc.iSerialNumber) {
        stat = libusb_get_string_descriptor_ascii(m_usb_handle,
            desc.iSerialNumber, (uint8_t*)strdesc_buf, 100);
        check_and_throw(stat, "Failed to get string descriptor");
        m_serial_no = strdesc_buf;
    } else
        m_serial_no = "-1";

    debug_print("Bus\t%i\n", m_bus);
    debug_print("Port\t%i\n", m_port);
    debug_print("idVendor\t0x%02X\n", m_vid);
    debug_print("idProduct\t0x%02X\n", m_pid);
    debug_print("iSerialNumber\t%s\n", m_serial_no.c_str());
    debug_print("bDeviceClass\t0x%02X\n", m_dev_class);
    debug_print("bDeviceSubClass\t0x%02X\n", m_dev_subclass);
    debug_print("bDeviceProtocol\t0x%02X\n", m_dev_protocol);

    return;
}

void usb_interface::gather_interface_information() 
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

void usb_interface::check_and_throw(int stat, const string& msg) const 
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

void usb_interface::check_interface() 
{
    if (m_cur_interface_no == s_no_interface)
        throw bad_io("No USB interface claimed");
    return;
}

}