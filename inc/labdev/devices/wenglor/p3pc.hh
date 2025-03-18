#ifndef LD_P3PC_HH
#define LD_P3PC_HH

#include <labdev/devices/ld_device.hh>
#include <labdev/serial_iface.hh>

#include <memory>
#include <vector>
#include <string>
#include <cstdint>

namespace labdev 
{

/** \brief Wenglor P3PC series laser distance sensor controlled by EFBL003 IO-link master.
 * 
 * This class only works with the P3PC series LDSs by wenglor IN COMBINATION
 * with an EFBL003 IO-Link master using a FT230X USB-to-UART converter.
 * 
 * Settings and protocol had to be reverse-engineered since wenglor does not
 * publish the specifications, which is why there a few magic numbers appearing 
 * in the implementation of this class.
 * This is also the reason why the implementation is kept that minimal.
 */

class p3pc : public ld_device
{
public:
    p3pc() : ld_device("P3PC") {};
    p3pc(std::unique_ptr<serial_iface> ser);
    ~p3pc();

    static constexpr unsigned BAUD = 115200;



    void connect(std::unique_ptr<ld_iface> comm) override;
    void disconnect() override;

    /// Returns distance in mm
    float get_distance();

    /// Queries and returns vendor name
    std::string get_vendor_name();

    /// Queries and returns vendor text
    std::string get_vendor_text();

    /// \brief Packet structure for communication with EFBL003 IO-Link master
    class iq2_packet 
    {
    public:
        /// No default ctor
        iq2_packet() = delete;
        /// Create packet from raw data input
        iq2_packet(std::vector<uint8_t> raw);
        /// Create packet from structured input 
        iq2_packet(uint8_t channel, std::vector<uint8_t> payload);

        // IQ communication channels for IO-Link master
        static constexpr uint8_t ERROR        = 0x00;
        static constexpr uint8_t IO_LINK      = 0x40;
        static constexpr uint8_t IQ_INTERFACE = 0xC0;

        // IQ commands for IO-Link master
        static constexpr uint8_t SET_OP_MODE  = 0x21;
        static constexpr uint8_t PD_READ      = 0x22;
        static constexpr uint8_t PD_WRITE     = 0x23;
        static constexpr uint8_t OD_WRITE_REQ = 0x24;
        static constexpr uint8_t OD_WRITE     = 0x25;
        static constexpr uint8_t OD_READ_REQ  = 0x26;
        static constexpr uint8_t OD_READ      = 0x27;

        /// Returns the raw data packet
        std::vector<uint8_t> get() const;
        /// Returns the CRC6 checksum
        uint8_t get_crc() const { return (m_ch_crc & 0x3F); }
        /// Returns true if the CRC is correct
        bool crc_good() const;
        /// Returns the communication channel
        uint8_t get_channel() const { return (m_ch_crc & 0xC0); }
        /// Returns the length field
        uint8_t get_len() const { return m_len; }
        /// Returns the payload of the packet
        std::vector<uint8_t> get_payload() const { return m_payload; }

        /// Returns IQ2 own CRC6 checksum (similar to IO-Link spec)
        static uint8_t calc_crc6(uint8_t channel, std::vector<uint8_t> payload);

    private:
        uint8_t m_ch_crc {0x00};
        uint8_t m_len {0x00};
        std::vector<uint8_t> m_payload {};
    };

private:
    void init();

    /// Get data package according to protocol
    std::vector<uint8_t> query(std::vector<uint8_t> request);

    /// Request a On-request Data (OD) read
    std::vector<uint8_t> request_od_read(uint16_t index, uint8_t subindex = 0x00);

    /// Get Process Data (PD) (i.e. the distance)
    std::vector<uint8_t> get_pd();

    /// Get On-request Data (OD) (i.e. everything else)
    std::vector<uint8_t> get_od();

};

}

#endif
