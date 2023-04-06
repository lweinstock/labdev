#ifndef LD_UTILS_HH
#define LD_UTILS_HH

#include <string>
#include <vector>

namespace labdev {

// Split string into vector of strings by given delimiters
std::vector<std::string> split(std::string list, std::string delim,
    size_t max_size = -1);

// Compare two floating point numbers
bool equal(double a, double b, double epsilon = 1e-6);

// Calculate mean (didnt know that didnt exist before...)
template<typename T> double get_mean(std::vector<T> vec);

// Calculate standard deviation (didnt know that didnt exist either...)
template<typename T> double get_stdev(std::vector<T> vec);

}

#endif