#include <labdev/utils/utils.hh>
#include <labdev/ld_debug.hh>

#include <numeric>
#include <cmath>

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

template<typename T> double get_mean(vector<T> val)
{
    // Very important: starting value has to be 0. and not 0
    // otherwise it treats everything as integers!!
    double ret = accumulate(val.begin(), val.end(), 0.);
    ret /= val.size();
    return ret;
}

template double get_mean<double>(vector<double> val);
template double get_mean<float>(vector<float> val);
template double get_mean<int>(vector<int> val);

template<typename T> double get_stdev(vector<T> val)
{
    double v_mean = get_mean(val);
    // Add up the squares using a lambda function
    double v_mean2 = accumulate(val.begin(), val.end(), 0., 
        [&](T a, T b) { return a + b*b; });
    v_mean2 /= val.size();
    double ret = sqrt( abs(v_mean2 - v_mean*v_mean) );
    if (isnan(ret)) // Floating point precision..
        return 0.;
    return ret;
}

template double get_stdev(vector<double> val);
template double get_stdev(vector<float> val);
template double get_stdev(vector<int> val);

}