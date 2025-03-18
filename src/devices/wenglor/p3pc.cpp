#include <labdev/devices/wenglor/p3pc.hh>
#include <labdev/exceptions.hh>
#include <unistd.h>

using namespace std;

namespace labdev 
{

p3pc::p3pc(unique_ptr<serial_iface> ser) : ld_device("P3PC")
{
    this->connect(std::move(ser));
    return;
}

p3pc::~p3pc()
{
    if (this->connected()) 
        this->disconnect();
    return;
}

void p3pc::connect(unique_ptr<ld_iface> comm)
{
    if ( this->connected() ) {
        string err = this->get_info() + " : device is already connected";
        throw device_error(err);
        return;
    }

    Interface_type type = comm->type();
    if (type == SERIAL) {
        // Convert to tcpip interface
        unique_ptr<serial_iface> ser(
            dynamic_cast<serial_iface*>(comm.release()));
        
         // Check baud -> 115200 baud
         if ( ser->get_baud() != p3pc::BAUD ) {
            fprintf(stderr, "FY6900 only supports %i baud 8N1\n", p3pc::BAUD);
            abort();
        }

        // Everything seems to be in order
        m_comm = std::move(ser);
    } else {
        string err = this->get_info() + " : interface is not supported";
        throw device_error(err); 
    } 

    this->init();
}

void p3pc::disconnect()
{
    // Shutdown sequence; maybe port is closed? (see above)
    vector<uint8_t> resp = m_comm->query_byte({0x03, 0x40, 0x21, 0x00});
    m_comm.reset();
    return;
}

float p3pc::get_distance()
{
    auto resp = this->get_pd();
    uint32_t dist = (resp[3] << 24) | (resp[4] << 16) 
                  | (resp[5] <<  8) | (resp[6] <<  0);
    return 1e-3 * static_cast<float>(dist);
}

std::string p3pc::get_vendor_name()
{
    // Vendor name: index 0x0010
    this->request_od_read(0x0010);
    //vector<uint8_t> resp = m_comm->query_byte({0x05, 0x76, 0x26, 0x10, 0x00, 0x00});
    usleep(10e3);

    // Update On-request Data buffer
    this->get_pd();

    // Get On-request Data
    auto name = this->get_od();
    return string(name.begin() + 3, name.end());
}

std::string p3pc::get_vendor_text()
{
    // Vendor text: index 0x0011
    this->request_od_read(0x0011);
    //vector<uint8_t> resp = m_comm->query_byte({0x05, 0x67, 0x26, 0x11, 0x00, 0x00});
    usleep(10e3);

    // Update On-request Data buffer
    this->get_pd();

    // Get On-request Data
    auto name = this->get_od();
    return string(name.begin() + 3, name.end());
}

/*
 *  I Q 2 - P A C K E T
 */

p3pc::iq2_packet::iq2_packet(vector<uint8_t> raw)
{
    if (raw.size() > 255)
        throw bad_protocol("Packet size is too large");
    
    m_len = raw.at(0);
    if (raw.size() != m_len + 1)
        throw bad_protocol("Size field does not match size of payload");
    
    m_ch_crc = raw.at(1);
    m_payload.insert(m_payload.begin(), raw.begin() + 2, raw.end());

    return;
}

p3pc::iq2_packet::iq2_packet(uint8_t channel, vector<uint8_t> payload)
{
    if (payload.size() > 254)
        throw bad_protocol("Payload size is too large");
    m_len = payload.size() + 1;
    m_ch_crc = channel | calc_crc6(channel, payload);
    m_payload = payload;
    return;
}

std::vector<uint8_t> p3pc::iq2_packet::get() const
{
    vector<uint8_t> raw {};
    raw.push_back(m_len);
    raw.push_back(m_ch_crc);
    raw.insert(raw.end(), m_payload.begin(), m_payload.end());
    return raw;
}

bool p3pc::iq2_packet::crc_good() const
{
    uint8_t crc = calc_crc6(this->get_channel(), this->get_payload());
    return (this->get_crc() == crc);
}


uint8_t p3pc::iq2_packet::calc_crc6(uint8_t channel, std::vector<uint8_t> payload)
{
    uint8_t crc = channel ^ 0x52; // 0x52 is the IO-Link seed value
    for (auto p : payload)
        crc ^= p;

    // Compress 8 bit checksum to 6 bit
    uint8_t crc6 = 0x00;
    crc6 |= (((crc >> 7) ^ (crc >> 5) ^ (crc >> 3) ^ (crc >> 1)) & 0x01) << 5;
    crc6 |= (((crc >> 6) ^ (crc >> 4) ^ (crc >> 2) ^ (crc >> 0)) & 0x01) << 4;
    crc6 |= (((crc >> 7) ^ (crc >> 6)) &  0x01) << 3;
    crc6 |= (((crc >> 5) ^ (crc >> 4)) &  0x01) << 2;
    crc6 |= (((crc >> 3) ^ (crc >> 2)) &  0x01) << 1;
    crc6 |= (((crc >> 1) ^ (crc >> 0)) &  0x01) << 0;
    return crc6;
}

/*
 *  P R I V A T E   M E T H O D S
 */

void p3pc::init()
{
    vector<uint8_t> resp {};
    // Both queries are required for the LDS to work;
    // According to the IO-Link spec there is a Standardized Master
    // Interface (SMI), that does Port Identification and Configuration
    // (see overview on p.228). Maybe this is whats happening here?
    resp = m_comm->query_byte({0x04, 0x73, 0x11, 0x09, 0x03});
    resp = m_comm->query_byte({0x03, 0x70, 0x21, 0x03});
    usleep(100e3);
    return;
}

vector<uint8_t> p3pc::query(vector<uint8_t> request)
{
    m_comm->write_byte(request);
    usleep(100e3);              // give it some time...

    uint8_t len {};
    m_comm->read_raw(&len, 1);  // First byte is length of message

    vector<uint8_t> resp {};
    while (resp.size() < len)   // Read until full message was received
    {
        auto temp = m_comm->read_byte(len);
        resp.insert(resp.end(), temp.begin(), temp.end());
        usleep(10e3);
    }

    iq2_packet packet(resp);    // TODO: Check CRC, length, etc.
    return packet.get_payload();
}

vector<uint8_t> p3pc::request_od_read(uint16_t index, uint8_t subindex)
{
    uint8_t idx_lo = static_cast<uint8_t>(index & 0x00FF);
    uint8_t idx_hi = static_cast<uint8_t>( (index >> 8) & 0x00FF);
    vector<uint8_t> payload {iq2_packet::OD_READ_REQ, idx_lo, idx_hi, subindex};
    iq2_packet msg(iq2_packet::IO_LINK, payload);
    return m_comm->query_byte(msg.get());
}

vector<uint8_t> p3pc::get_pd()
{
    iq2_packet msg(iq2_packet::IO_LINK, {iq2_packet::PD_READ});
    return this->query(msg.get());
}

vector<uint8_t> p3pc::get_od()
{
    iq2_packet msg(iq2_packet::IO_LINK, {iq2_packet::OD_READ});
    return this->query(msg.get());
}

}