#include <labdev/ld_interface.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

using namespace std;

namespace labdev{

void ld_interface::write_byte(const vector<uint8_t> data)
{
    this->write_raw(data.data(), data.size());
    return;
}

void ld_interface::write(const string& msg) {
    uint8_t wbuf[s_dflt_buf_size] = {0};
    copy(msg.begin(), msg.end(), begin(wbuf));
    this->write_raw(wbuf, msg.size());

    debug_print_string_data(msg, "Sent %zu bytes: ", msg.size());

    return;
}

vector<uint8_t> ld_interface::read_byte(unsigned timeout_ms)
{
    uint8_t rbuf[s_dflt_buf_size] = {0};
    ssize_t nbytes = this->read_raw(rbuf, s_dflt_buf_size, timeout_ms);
    vector<uint8_t> ret(rbuf, rbuf + nbytes);
    return ret;
}

string ld_interface::read(unsigned timeout_ms) {
    uint8_t rbuf[s_dflt_buf_size] = {0};
    ssize_t nbytes = this->read_raw(rbuf, s_dflt_buf_size, timeout_ms);
    string ret((char*)rbuf, nbytes);

    debug_print_string_data(ret, "Read %zu bytes: ", ret.size());
    
    return ret;
}

string ld_interface::read_until(const string& delim, size_t& pos, 
unsigned timeout_ms) {
    string ret("");
    while ( (pos = ret.find(delim)) == string::npos ) {
        string rbuf = this->read(timeout_ms);
        if (rbuf.size() > 0)
            ret.append(rbuf);
    }
    return ret;
}

string ld_interface::read_until(const string& delim, 
unsigned timeout_ms) {
    size_t temp= 0;
    return this->read_until(delim, temp, timeout_ms);    
}

string ld_interface::query(const string& msg, unsigned timeout_ms) {
    write(msg);
    return read(timeout_ms);
}

}