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

private:
    void init();

    /// Get data package according to protocol
    std::vector<uint8_t> get_data(std::vector<uint8_t> request);

    /// Get Process Data (PD) (i.e. the distance)
    std::vector<uint8_t> get_process_data();

    /// Get On-request Data (OD) (i.e. everything else)
    std::vector<uint8_t> get_on_request_data();

};

}

#endif
