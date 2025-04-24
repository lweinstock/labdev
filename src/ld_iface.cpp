#include <labdev/ld_iface.hh>
#include <labdev/exceptions.hh>
#include <labdev/ld_debug.hh>

#include <sys/time.h>

using namespace std;

namespace labdev{

void ld_iface::write_byte(const vector<uint8_t> data)
{
    this->write_raw(data.data(), data.size());
    return;
}

void ld_iface::write(const string& msg) {
    uint8_t wbuf[DFLT_BUF_SIZE] = {0};
    copy(msg.begin(), msg.end(), begin(wbuf));
    this->write_raw(wbuf, msg.size());

    debug_print_string_data(msg, "Sent %zu bytes: ", msg.size());

    return;
}

vector<uint8_t> ld_iface::read_byte(size_t max_len, unsigned timeout_ms)
{
    uint8_t rbuf[DFLT_BUF_SIZE] = {0};
    max_len = min(max_len, DFLT_BUF_SIZE);  // Limited size
    ssize_t nbytes = this->read_raw(rbuf, max_len, timeout_ms);
    vector<uint8_t> ret(rbuf, rbuf + nbytes);
    return ret;
}

string ld_iface::read(unsigned timeout_ms) 
{
    uint8_t rbuf[DFLT_BUF_SIZE] = {0};
    ssize_t nbytes = this->read_raw(rbuf, DFLT_BUF_SIZE, timeout_ms);
    string ret((char*)rbuf, nbytes);

    debug_print_string_data(ret, "Read %zu bytes: ", ret.size());
    
    return ret;
}

string ld_iface::read_until(const string& delim, size_t& pos, 
    unsigned timeout_ms) 
{
    string ret("");
    struct timeval sta, sto;
    gettimeofday(&sta, NULL);
    do {
        string rbuf = this->read(timeout_ms);
        if (rbuf.size() > 0)
            ret.append(rbuf);
        pos = ret.rfind(delim);

        // Check timeout
        gettimeofday(&sto, NULL);
        unsigned diff_ms = (sto.tv_sec-sta.tv_sec)*1000 
            + (sto.tv_usec-sta.tv_usec)/1000;
        if (diff_ms > timeout_ms) 
            throw timeout("Did not receive delimiter '" + delim + "' in time");
    } while ( pos == string::npos );
    return ret;
}

string ld_iface::read_until(const string& delim, 
    unsigned timeout_ms) 
{
    size_t temp = 0;
    return this->read_until(delim, temp, timeout_ms);    
}

string ld_iface::query(const string& msg, unsigned timeout_ms) {
    this->write(msg);
    return this->read(timeout_ms);
}

vector<uint8_t> ld_iface::query_byte(const vector<uint8_t> data, unsigned timeout_ms)
{
    this->write_byte(data);
    return this->read_byte(timeout_ms);
}

}