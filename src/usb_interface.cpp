#include <labdev/usb_interface.hh>
#include <labdev/exceptions.hh>
#include "ld_debug.hh"

namespace labdev {

    libusb_context* usb_interface::s_default_ctx = NULL;
    int usb_interface::s_dev_count = 0;

    usb_interface::usb_interface():
    m_usb_dev(NULL),
    m_usb_handle(NULL),
    m_cur_cfg(0),
    m_cur_alt_setting(0),
    m_cur_interface_no(-1),
    m_cur_ep_in_addr(0),
    m_cur_ep_out_addr(0),
    m_connected(false),
    m_dev_class(0),
    m_dev_subclass(0),
    m_dev_protocol(0),
    m_interface_class(0),
    m_interface_subclass(0),
    m_interface_protocol(0) 
    {
        return;
    }

    usb_interface::usb_interface(uint16_t vendor_id, uint16_t product_id,
    std::string serial_number):
    usb_interface() 
    {
        this->open(vendor_id, product_id, serial_number);
        return;
    }

    usb_interface::usb_interface(uint8_t bus_no, uint8_t port_no):
    usb_interface() 
    {
        this->open(bus_no, port_no);
        return;
    }

    usb_interface::usb_interface(usb_config &conf) : usb_interface()
    {
        if (conf.vid != 0x0000 && conf.pid != 0x0000) {
                this->open(conf.vid, conf.pid, conf.serial);
                return;
        }
        this->open(conf.bus_no, conf.port_no);
        return;
    }

    usb_interface::~usb_interface() 
    {
        this->close();
        return;
    }

    void usb_interface::open(uint16_t vendor_id, uint16_t product_id,
    std::string serial_number) 
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
            libusb_device* tmp_dev = dev_list[idev];
            libusb_device_descriptor desc;
            stat = libusb_get_device_descriptor(tmp_dev, &desc);
            check_and_throw(ndev, "Failed to get device descriptor");
            debug_print("dev %i - ID 0x%04X:0x%04X\n", idev, desc.idVendor,
                desc.idProduct);

            // Check for VID and PID
            if ( (desc.idVendor == vendor_id) && (desc.idProduct == product_id) ) {
                // Also check for serial number, if specified
                if ( !serial_number.empty() ) {
                    libusb_device_handle* tmp_handle;
                    stat = libusb_open(tmp_dev, &tmp_handle);
                    check_and_throw(stat, "Failed to get usb handle");

                    uint8_t serial_str[100];
                    stat = libusb_get_string_descriptor_ascii(tmp_handle,
                        desc.iSerialNumber, serial_str, 100);
                    check_and_throw(stat, "Failed to get string descriptor");
                    debug_print("SerialNumber %s\n", serial_str);

                    if (strcmp(serial_number.c_str(), (const char*)serial_str) == 0) {
                        m_usb_dev = tmp_dev;
                        break;
                    }
                } else {
                    m_usb_dev = tmp_dev;
                    break;
                }
            }
        }

        // Get I/O handle
        if (m_usb_dev) {
            stat = libusb_open(m_usb_dev, &m_usb_handle);
            check_and_throw(stat, "Failed to get usb handle");
            this->gather_device_information();
        } else {
            fprintf(stderr, "Device ID 0x%04X:0x%04X not found\n", vendor_id, product_id);
            abort();
        }
        s_dev_count++;
        debug_print("Opened device, new device count = %i\n", s_dev_count);

        libusb_free_device_list(dev_list, 1);
        m_connected = true;

        return;
    }

    void usb_interface::open(uint8_t bus_no, uint8_t port_no) 
    {
        int stat;
        // If first device, start new libusb session
        if (s_dev_count == 0) {
            stat = libusb_init(&s_default_ctx);
            check_and_throw(stat, "libusb init failed");
            debug_print("new libusb session initialized (%i)\n", stat);
        }

        // Search for deivce with given bus and port number
        libusb_device** dev_list;
        int ndev = libusb_get_device_list(s_default_ctx, &dev_list);
        check_and_throw(ndev, "Failed to get device list");

        for (int idev = 0; idev < ndev; idev++) {
            libusb_device* tmp_dev = dev_list[idev];
            uint8_t tmp_bus = libusb_get_bus_number(tmp_dev);
            uint8_t tmp_port = libusb_get_port_number(tmp_dev);
            debug_print("dev %i - bus 0x%02X port 0x%02X\n", idev, tmp_bus, tmp_port);
            // Check for bus and port number
            if ( (tmp_port == port_no) && (tmp_bus == bus_no) ) {
                m_usb_dev = tmp_dev;
                break;
            }
        }

        // Get I/O handle
        if (m_usb_dev) {
            stat = libusb_open(m_usb_dev, &m_usb_handle);
            check_and_throw(stat, "Failed to get usb handle");
            this->gather_device_information();
        } else {
            fprintf(stderr, "Device bus 0x%02X port 0x%02X not found\n", bus_no, port_no);
            abort();
        }
        s_dev_count++;
        debug_print("Opened device, new device count = %i\n", s_dev_count);

        libusb_free_device_list(dev_list, 1);
        m_connected = true;

        return;
    }

    void usb_interface::close() 
    {
        // Release claimed interfaces and device
        if (m_cur_interface_no != s_no_interface)
            libusb_release_interface(m_usb_handle, m_cur_interface_no);
        if (m_usb_handle)
            libusb_close(m_usb_handle);
        m_cur_interface_no = s_no_interface;
        // Close context if last device released
        s_dev_count--;
        m_connected = false;
        debug_print("Closed device, new device count = %i\n", s_dev_count);
        if (s_dev_count == 0) {
            debug_print("%s\n", "Last device closed, exiting libusb\n");
            libusb_exit(s_default_ctx);
        }
        return;
    }

    int usb_interface::write_raw(const uint8_t* data, size_t len) 
    {
        this->check_interface();
        size_t bytes_left = len;
        size_t bytes_written = 0;
        ssize_t nbytes = 0;

        while ( bytes_left > 0 ) {
            nbytes = this->write_bulk(&data[bytes_written], bytes_left);
            check_and_throw(nbytes, "Failed to write to device");
            bytes_left -= nbytes;

            debug_print("Written %zu bytes: ", nbytes);
            #ifdef LD_DEBUG
            if (nbytes > 20) {
                for (int i = 0; i < 10; i++)
                    printf("0x%02X ", data[bytes_written + i]);
                printf("[...] ");
                for (int i = nbytes-10; i < nbytes; i++)
                    printf("0x%02X ", data[bytes_written + i]);
            } else {
                for (int i = 0; i < nbytes; i++)
                    printf("0x%02X ", data[bytes_written + i]);
            }
            printf("(%zi bytes left)\n", bytes_left);
            #endif

            bytes_written += nbytes;
        }
        return bytes_written;
    };

    int usb_interface::read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms) 
    {
        this->check_interface();
        ssize_t nbytes = 0;
        uint8_t rbuf[s_dflt_buf_size] = {'\0'};
        nbytes = this->read_bulk(rbuf, sizeof(rbuf), timeout_ms);
        check_and_throw(nbytes, "Failed to read from device");

        debug_print("Read %zi bytes: ", nbytes);
        #ifdef LD_DEBUG
        if (nbytes > 20) {
            for (int i = 0; i < 10; i++)
                printf("0x%02X ", data[i]);
            printf("[...] ");
            for (int i = nbytes-10; i < nbytes; i++)
                printf("0x%02X ", data[i]);
        } else {
            for (int i = 0; i < nbytes; i++)
                printf("0x%02X ", data[i]);
        }
        printf("\n");
        #endif

        return nbytes;
    };

    std::string usb_interface::get_info() const 
    {
        // Format example: usb;002;001
        std::string ret("usb;" + std::to_string(m_bus) + ";" 
            + std::to_string(m_port) );
        return ret;
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

    int usb_interface::read_control(uint8_t request_type, uint8_t request, uint16_t value, uint16_t index,
    const uint8_t* data, int len) 
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

        int nbytes = -1, stat;
        stat = libusb_bulk_transfer(
            m_usb_handle,
            LIBUSB_ENDPOINT_OUT | m_cur_ep_out_addr,
            (uint8_t*)data,
            len,
            &nbytes,
            1000);
        check_and_throw(stat, "Bulk transfer failed to send data");

        // Verbose debug print
        debug_print("Sent %i bytes : ", nbytes);
        #ifdef LD_DEBUG
        if (nbytes > 20) {
            for (int i = 0; i < 10; i++)
                printf("0x%02X ", data[i]);
            printf("[...] ");
            for (int i = nbytes-10; i < nbytes; i++)
                printf("0x%02X ", data[i]);
        } else {
            for (int i = 0; i < nbytes; i++)
                printf("0x%02X ", data[i]);
        }
        printf("\n");
        #endif

        return nbytes;
    }

    int usb_interface::read_bulk(uint8_t* data, int max_len, int timeout_ms) 
    {
        this->check_interface();

        int nbytes = 0, stat;
        stat = libusb_bulk_transfer(
            m_usb_handle,
            LIBUSB_ENDPOINT_IN | m_cur_ep_in_addr,
            data,
            max_len,
            &nbytes,
            timeout_ms);
        check_and_throw(stat, "Bulk transfer failed to read data");

        // Verbose byte-wise debug print
        debug_print("Read %i bytes : ", nbytes);
        #ifdef LD_DEBUG
        if (nbytes > 20) {
            for (int i = 0; i < 10; i++)
                printf("0x%02X ", data[i]);
            printf("[...] ");
            for (int i = nbytes-10; i < nbytes; i++)
                printf("0x%02X ", data[i]);
        } else {
            for (int i = 0; i < nbytes; i++)
                printf("0x%02X ", data[i]);
        }
        printf("\n");
        #endif

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

    void usb_interface::claim_interface(int interface_no, int alt_setting) 
    {
        int stat;
        char buf[100];
        // Release current interface
        if ( (interface_no != m_cur_interface_no) &&
             (m_cur_interface_no != s_no_interface) ) {
            stat = libusb_release_interface(m_usb_handle, m_cur_interface_no);
            sprintf(buf, "Failed to release interface %i", m_cur_interface_no);
            check_and_throw(stat, buf);
        }
        // Detach kernel drivers if active
        if ( libusb_kernel_driver_active(m_usb_handle, interface_no) ) {
            stat = libusb_detach_kernel_driver(m_usb_handle, interface_no);
            check_and_throw(stat, "Failed to detach kernel driver from interface");
        }
        // Claim new interface
        stat = libusb_claim_interface(m_usb_handle, interface_no);
        sprintf(buf, "Failed to claim interface %i", interface_no);
        check_and_throw(stat, buf);
        m_cur_interface_no = interface_no;
        debug_print("Successfully claimed interface %i\n", m_cur_interface_no);

        // Apply alternate settingsm if specified
        // (FYI: most devices dont use alt settings)
        if (alt_setting) {
            stat = libusb_set_interface_alt_setting(m_usb_handle,
                m_cur_interface_no, alt_setting);
            sprintf(buf, "Failed to apply alternate settings %i", alt_setting);
            check_and_throw(stat, buf);
            m_cur_alt_setting = alt_setting;
            debug_print("Applied alternate settings %i\n", m_cur_alt_setting);
        }

        this->gather_interface_information();
        return;
    }

    void usb_interface::set_endpoint_in(uint8_t ep_addr) 
    {
        m_cur_ep_in_addr = ep_addr;
        return;
    }

    void usb_interface::set_endpoint_out(uint8_t ep_addr) 
    {
        m_cur_ep_out_addr = ep_addr;
        return;
    }

    /*
     *      P R I V A T E   M E T H O D S
     */

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

        debug_print("Bus\t\t %i\n", m_bus);
        debug_print("Port\t\t %i\n", m_port);
        debug_print("idVendor\t 0x%02X\n", m_vid);
        debug_print("idProduct\t 0x%02X\n", m_pid);
        debug_print("iSerial\t %s\n", m_serial_no.c_str());
        debug_print("bDeviceClass\t 0x%02X\n", m_dev_class);
        debug_print("bDeviceSubClass 0x%02X\n", m_dev_subclass);
        debug_print("bDeviceProtocol 0x%02X\n", m_dev_protocol);

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

    void usb_interface::check_and_throw(int stat, const std::string& msg) const 
    {
        if (stat < 0) {
            char err_msg[256] = {'\0'};
            sprintf(err_msg, "%s (%s, %i)", msg.c_str(),
                libusb_error_name(stat), stat);
            debug_print("%s\n", err_msg);

            switch (stat) {
            case LIBUSB_ERROR_TIMEOUT:
            case LIBUSB_ERROR_BUSY:
                throw timeout(err_msg, stat);
                break;

            case LIBUSB_ERROR_OVERFLOW:
            case LIBUSB_ERROR_IO:
            case LIBUSB_ERROR_NO_MEM:
            case LIBUSB_ERROR_OTHER:
                throw bad_io(err_msg, stat);
                break;

            case LIBUSB_ERROR_ACCESS:
            case LIBUSB_ERROR_NO_DEVICE:
                throw bad_connection(err_msg, stat);
                break;

            case LIBUSB_ERROR_INVALID_PARAM:
            case LIBUSB_ERROR_NOT_SUPPORTED:
            default:
                fprintf(stderr, "%s\n", err_msg);
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