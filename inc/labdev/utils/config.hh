#ifndef LD_CONFIG_HH
#define LD_CONFIG_HH

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>

namespace labdev {

/*
 *  Small utility to read simple config files.
 *  The format is '<key>\t<value>\n' and comments start with #.
 *  Supported are integers, floats, strings, and bools (TRUE and FALSE)
 *
 *  Example:
 *  # This is a comment
 *  value1  12.34
 *  value2  1234
 *  my_bool TRUE
 *  HELLO   Hello World
 *
 */

class config {
public:
    config(std::string conf_path);
    ~config() {};

    std::string get_string(std::string key);
    bool get_bool(std::string key);
    int get_int(std::string key);
    float get_float(std::string key);

private:
    std::ifstream m_file;
    std::map<std::string,std::string> m_conf;
};

}

#endif