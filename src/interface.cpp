#include <labdev/interface.hh>
#include <labdev/exceptions.hh>
#include "ld_debug.hh"

using std::string;

namespace labdev{

    void interface::write(const string& msg) {
        uint8_t wbuf[s_dflt_buf_size] = {0};
        std::copy(msg.begin(), msg.end(), std::begin(wbuf));
        this->write_raw(wbuf, msg.size());

        debug_print("%s", "Sent message '");
        #ifdef LD_DEBUG
        size_t nbytes = msg.size();
        if (msg.size() > 100) {
            for (int i = 0; i < 50; i++)
                printf("%c", msg.at(i));
            printf(" [...] ");
            for (int i = nbytes-50; i < nbytes; i++)
                printf("%c", msg.at(i));
        } else {
            printf("%s", msg.c_str());
        }
        printf("'\n");
        #endif

        return;
    }

    string interface::read(unsigned timeout_ms) {
        uint8_t rbuf[s_dflt_buf_size] = {0};
        ssize_t nbytes = this->read_raw(rbuf, s_dflt_buf_size, timeout_ms);
        string ret((char*)rbuf, nbytes);

        debug_print("Read %zi bytes: ", nbytes);
        #ifdef LD_DEBUG
        if (ret.size() > 100) {
            for (int i = 0; i < 50; i++)
                printf("%c", ret.at(i));
            printf(" [...] ");
            for (int i = nbytes-50; i < nbytes; i++)
                printf("%c", ret.at(i));
        } else {
            printf("%s", ret.c_str());
        }
        printf("'\n");
        #endif

        return ret;
    }

    string interface::read_until(const string& delim, size_t& pos, 
    unsigned timeout_ms) {
        string ret("");
        // Read and append until delimeter is found
        while ( (pos = ret.find(delim)) == string::npos ) {
            string rbuf = this->read(timeout_ms);
            if (rbuf.size() > 0)
                ret.append(rbuf);
        }
        return ret;
    }

    string interface::query(const string& msg, unsigned timeout_ms) {
        write(msg);
        return read(timeout_ms);
    }

}