#ifndef LD_VISA_INTERFACE_H
#define LD_VISA_INTERFACE_H

#include <string>
#include <vector>

#include <labdev/interface.hh>
#include <labdev/exceptions.hh>

#ifdef LDVISA

#ifdef __APPLE__
    #include <RsVisa/visa.h>
#else
    #include <visa.h>
#endif

namespace labdev {
    class visa_interface : public interface {
    public:
        visa_interface();
        visa_interface(std::string visa_id);
        ~visa_interface();

        // Find all available VISA resource, returns list of VISA IDs
        std::vector<std::string> find_resources(std::string regex = "?*INSTR");

        // Open/close device
        void open(const std::string &visa_id);
        void close() override;


        int write_raw(const uint8_t* data, size_t len) override;
        int read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms) override;

        Interface_type type() const override { return visa; }

        bool connected() const override { return m_connected; }

        // Clear I/O buffers
        void flush_buffer(uint16_t flag = VI_READ_BUF | VI_WRITE_BUF);
        void clear_device();

    private:
        static const size_t MAX_BUF_SIZE = 1024*1024;   // 1MB buffer
        static const unsigned DFLT_TIMEOUT_MS = 2000;   // 2s timeout

        static ViSession s_default_rm;
        static int s_interface_ctr;

        ViSession m_instr;
        std::string m_visa_id;
        bool m_connected;
        unsigned m_timeout;

        void init();
        void check_and_throw(ViStatus stat, const std::string &msg) const;
    };
}

#else

/*
 *      Empty dummy class if visa support is disabled in makefile
 */

namespace labdev {
    class visa_interface : public interface {
    public:
        visa_interface() {
            fprintf(stderr, "labdev compiled without VISA support. To enable recompile using 'make VISA=1'.\n");
            abort();
        }

        visa_interface(std::string visa_id) : visa_interface() {}

        ~visa_interface() {}

        void close() override {};

        int write_raw(const uint8_t* data, size_t len) override { return -1; }
        int read_raw(uint8_t* data, size_t max_len, unsigned timeout_ms) override
            { return -1; }

        Interface_type type() const override { return visa; }

        bool connected() const override { return false; }

        // Clear I/O buffers
        void flush_buffer() {};
        void clear_device() {};

    private:
    };
}

#endif

#endif
