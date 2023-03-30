#ifndef LD_UTILS_HH
#define LD_UTILS_HH

#include <string>
#include <vector>

namespace labdev {

// Split string into vector of strings by given delimiters
std::vector<std::string> split(std::string list, std::string delim,
    size_t max_size = -1);

// Compare two floating point numbers
bool compare_doubles(double a, double b, double epsilon = 1e-6);

}

#endif