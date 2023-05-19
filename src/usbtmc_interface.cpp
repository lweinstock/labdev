#include <labdev/usbtmc_interface.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

using namespace std;

namespace labdev {

usbtmc_interface::usbtmc_interface(const usb_config conf)
    : usb_interface(conf), m_cur_tag(0x01)
{
    return;
}

int usbtmc_interface::write_raw(const uint8_t* data, size_t len) 
{
    return this->write_dev_dep_msg(data, len);
}

int usbtmc_interface::read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms) 
{
    return this->read_dev_dep_msg(data, max_len, timeout_ms);
}

int usbtmc_interface::write_dev_dep_msg(const uint8_t* msg, size_t len,
    uint8_t transfer_attr) 
{
    // add space for header + total length must be multiple of 4
    size_t tot_len = s_header_len + len;
    if (tot_len%4 > 0)
        tot_len += 4 - tot_len%4;
    uint8_t* usbtmc_message = new uint8_t[tot_len];
    this->create_usbtmc_header(usbtmc_message, DEV_DEP_MSG_OUT,
        transfer_attr, len);

    // Append data
    for (size_t i = s_header_len; i < tot_len; i++) {
        usbtmc_message[i] = msg[i-s_header_len];
        if (i > len+s_header_len)
            usbtmc_message[i] = 0x00;   // zero padding
    }
    debug_print("%s\n", "Sending device dependent message");
    int nbytes = this->write_bulk((const uint8_t*)usbtmc_message, tot_len);
    debug_print_byte_data(usbtmc_message, nbytes, "Written %zu bytes: ", nbytes);

    // cleanup
    delete[] usbtmc_message;

    return nbytes;
}

int usbtmc_interface::read_dev_dep_msg(uint8_t* data, size_t max_len,
    int timeout_ms, uint8_t transfer_attr, uint8_t term_char) 
{
    uint8_t read_request[s_header_len];
    uint8_t rbuf[s_dflt_buf_size] = { 0x00 };

    // Send read request
    debug_print("%s\n", "Sending read request");
    this->create_usbtmc_header(read_request, REQUEST_DEV_DEP_MSG_IN,
        transfer_attr, sizeof(rbuf), term_char);
    this->write_bulk((const uint8_t*)read_request, s_header_len);

    // Read from bulk endpoint
    debug_print("%s\n", "Reading device dependent message");
    int len = this->read_bulk(rbuf, sizeof(rbuf), timeout_ms);

    // If an empty message was received, return immediatly
    if (len == 0)
        return len;

    // Check header
    int transfer_size = check_usbtmc_header(rbuf, DEV_DEP_MSG_IN);

    // Copy data into output array
    std::copy(rbuf + s_header_len, rbuf + len, data);
    int bytes_received = len - s_header_len;
    while (bytes_received < transfer_size) {
        int nbytes = this->read_bulk(rbuf, sizeof(rbuf), timeout_ms);
        if (bytes_received > max_len)
            throw bad_io("Buffer size too small");
        std::copy(rbuf, rbuf + nbytes, data + bytes_received);
        bytes_received += nbytes;
    }
    debug_print_byte_data(data, bytes_received, "Read %zu bytes: ", 
        bytes_received);

    // Increase bTag for next communication
    m_cur_tag++;
    
    return bytes_received;
}

int usbtmc_interface::write_vendor_specific(string msg) 
{
    // add space for header + total length must be multiple of 4
    size_t tot_len = s_header_len + msg.size() + 4 - msg.size()%4;
    uint8_t* usbtmc_message = new uint8_t[tot_len];
    this->create_usbtmc_header(usbtmc_message, VENDOR_SPECIFIC_OUT, 0x00,
        msg.size());

    // Append data
    for (size_t i = s_header_len; i < tot_len; i++) 
    {
        usbtmc_message[i] = msg.c_str()[i-s_header_len];
        if (i > msg.size()+s_header_len)
            usbtmc_message[i] = 0x00;   // zero padding
    }

    int nbytes = this->write_bulk((const uint8_t*)usbtmc_message, tot_len);
    // cleanup
    delete[] usbtmc_message;

    return nbytes;
}

string usbtmc_interface::read_vendor_specific(int timeout_ms) 
{
    uint8_t read_request[s_header_len], rbuf[s_dflt_buf_size];
    // Send read request
    debug_print("%s\n", "Sending vendor specific read request\n");
    this->create_usbtmc_header(read_request, REQUEST_VENDOR_SPECIFIC_IN,
        0x00, sizeof(rbuf), 0x00);
    this->write_bulk((const uint8_t*)read_request, s_header_len);

    // Read from bulk endpoint
    debug_print("%s\n", "Reading...\n");
    int len = this->read_bulk(rbuf, sizeof(rbuf), timeout_ms);

    // If an empty message was received, return immediatly
    if (len == 0)
        return "";

    // Check header
    int transfer_size = check_usbtmc_header(rbuf, DEV_DEP_MSG_IN);
    // Remove header from return value
    len -= s_header_len;
    string ret((const char*)rbuf + s_header_len, len);

    // If more data than received was anounced in the header, keep reading
    int bytes_left = transfer_size - len;
    while (bytes_left > 0) {
        int nbytes = this->read_bulk(rbuf, sizeof(rbuf), timeout_ms);
        ret.append((char*)rbuf, min(bytes_left, nbytes));
        bytes_left -= nbytes;
    }
    // Increase bTag for next communication
    m_cur_tag++;
    debug_print("Received vendor specific message (%lu) '%s'\n",
        ret.size(), ret.c_str());

    return ret;
}

void usbtmc_interface::clear_buffer() 
{
    uint8_t buf[1];
    this->write_control(LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE,
    INITIATE_CLEAR, 0x0000, 0x0000, buf, 0x0001);
    // TODO: check return value!
    return;
}

/*
void usbtmc_interface::claim_interface(int int_no, int alt_setting) 
{
    this->usb_interface::claim_interface(int_no, alt_setting);
    // Check for USBTMC interface
    if ( (this->get_interface_class() != LIBUSB_CLASS_APPLICATION) ||
            (this->get_interface_subclass() != LIBUSB_SUBCLASS_TMC) ) {
        fprintf(stderr, "Interface %i does not support USBTMC\n", int_no);
        abort();
    }
    debug_print("%s\n", "Device supports USBTMC");
    return;
}
*/

/*
 *      P R I V A T E   M E T H O D S
 */

void usbtmc_interface::create_usbtmc_header(uint8_t* header, uint8_t message_id, 
    uint8_t transfer_attr, uint32_t transfer_size, uint8_t term_char) 
{
    // Create USBTMC header
    header[0] = message_id;
    header[1] = m_cur_tag;
    header[2] = ~m_cur_tag;
    header[3] = 0x00;
    header[4] = 0xFF & transfer_size;
    header[5] = 0xFF & (transfer_size >> 8);
    header[6] = 0xFF & (transfer_size >> 16);
    header[7] = 0xFF & (transfer_size >> 24);
    switch (message_id) {
        case DEV_DEP_MSG_OUT:
            header[8] = 0x01 & transfer_attr;
            header[9] = 0x00;
            header[10] = 0x00;
            header[11] = 0x00;
            break;

        case REQUEST_DEV_DEP_MSG_IN:
            header[8] = 0x02 & transfer_attr;
            header[9] = term_char;
            header[10] = 0x00;
            header[11] = 0x00;
            if (transfer_attr & TERM_CHAR)
                m_term_char = term_char;
            break;

        case VENDOR_SPECIFIC_OUT:
        case REQUEST_VENDOR_SPECIFIC_IN:
            header[8] = 0x00;
            header[9] = 0x00;
            header[10] = 0x00;
            header[11] = 0x00;
            break;
    }

    return;
}

int usbtmc_interface::check_usbtmc_header(uint8_t* message,
    uint8_t message_id) 
{
    // Check MsgID field
    if ( message_id != message[0] ) {
        debug_print("Wrong MsgID returned : expected 0x%02X, received 0x%02X\n",
            message_id, message[0]);
        throw bad_protocol("Wrong MsgID received");
    }

    // Check bTag and ~bTag fields
    uint8_t inv_cur_tag = (uint8_t)(~m_cur_tag);
    if ( (message[1] != m_cur_tag) || (message[2] != inv_cur_tag) ) {
        debug_print("Wrong bTag/~bTag returned : expected 0x%02X/0x%02X, "
            "received 0x%02X/0x%02X\n", m_cur_tag, inv_cur_tag, message[1], 
            message[2]);
        throw bad_protocol("Wrong bTag/~bTag received");
    }

    // Check transfer size
    uint32_t transfer_size = ((uint32_t)message[4] <<  0) |
                             ((uint32_t)message[5] <<  8) |
                             ((uint32_t)message[6] << 16) |
                             ((uint32_t)message[7] << 24);

    debug_print("MsgID 0x%02X, bTag 0x%02X/0x%02X, TransferSize 0x%08X (%u)\n",
        message_id, m_cur_tag, inv_cur_tag, transfer_size, transfer_size);
    return transfer_size;
}

}
