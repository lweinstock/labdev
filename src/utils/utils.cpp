#include <labdev/utils/utils.hh>
#include <labdev/ld_debug.hh>

#include <numeric>
#include <cmath>

using namespace std;

namespace labdev {

vector<string> split(string list, string delim, size_t max_size) {
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

template<typename T> double get_mean(vector<T> vec)
{
    // Very important: starting value has to be 0. and not 0
    // otherwise it treats everything as integers!!
    return accumulate(vec.begin(), vec.end(), 0.)/vec.size();
}

template double get_mean<double>(vector<double> vec);
template double get_mean<float>(vector<float> vec);
template double get_mean<int>(vector<int> vec);

template<typename T> double get_stdev(vector<T> vec)
{
    size_t n = vec.size();
    double vec_mean = get_mean(vec);
    // Add up the squares using a lambda function
    vector<double> dev(n);
    transform(vec.begin(), vec.end(), dev.begin(), 
        [&](double v) { return v - vec_mean; });
    double var = inner_product(dev.begin(), dev.end(), dev.begin(), 0.)/n;
    return sqrt(var);
}

template double get_stdev(vector<double> vec);
template double get_stdev(vector<float> vec);
template double get_stdev(vector<int> vec);

}