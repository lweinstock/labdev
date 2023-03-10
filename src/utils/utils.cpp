#include <labdev/utils/utils.hh>
#include "ld_debug.hh"

using namespace std;

namespace labdev {

    vector<string> split(string list, string delim,
        size_t max_size) {
        vector<string> ret;
        size_t last = 0, pos = 0;
        while ( (pos = list.find(delim.c_str(), last)) != string::npos) {
            ret.push_back( list.substr(last, pos - last) );
            last = pos + delim.size();
            // If max size is reached, append rest to the vector and stop!
            if (ret.size() == max_size-1) {
                ret.push_back( list.substr(last, string::npos) );
                break;
            }
        }
        return ret;
    }

    bool compare_doubles(double a, double b, double epsilon) {
    return (abs(a-b) < epsilon);
}

}