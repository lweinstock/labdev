#include <labdev/utils/utils.hh>
#include <labdev/ld_debug.hh>

#include <numeric>

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

bool equal(double a, double b, double epsilon) {
    return (abs(a-b) < epsilon);
}

template<typename T> float get_mean(vector<T> val)
{
    return static_cast<float>(accumulate(val.begin(), val.end(), 0))/val.size();
}

template float get_mean<float>(vector<float> val);
template float get_mean<int>(vector<int> val);

template<typename T> float get_stdev(vector<T> val)
{
    float v_mean = get_mean(val);
    // Add up the squares using a lambda function
    float v_mean2 = static_cast<float>(accumulate(val.begin(), val.end(), 0, 
        [&](T a, T b) { return a + b*b; }))/val.size();
    return sqrt(v_mean2 - v_mean*v_mean);
}

template float get_stdev(vector<float> val);
template float get_stdev(vector<int> val);

}