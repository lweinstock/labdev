#include <labdev/utils/config.hh>
#include "ld_debug.hh"

namespace labdev {

    config::config(std::string conf_path) {
        m_file.open(conf_path, std::ifstream::in);

        debug_print("Reading configuration '%s':\n", conf_path.c_str());
        if ( m_file.fail() ) {
            printf("Cannot open file %s\n", conf_path.c_str());
            abort();
        }

        // Extract all line from config file
        std::string line;
        while ( getline(m_file, line) ) {
            // Skip empty line
            if ( line.size() == 0 ) continue;
            // Skip comments starting with "#"
            if ( line.at(line.find_first_not_of(' ')) == '#' ) continue;
            std::stringstream line_stream(line);

            // First element is the key, rest of line is the value for that key
            std::string key, value;
            line_stream >> key;
            line_stream >> std::ws; // skip whitespaces
            getline(line_stream, value);

            m_conf.insert( std::pair<std::string,std::string>(key, value) );
        }
        #ifdef LD_DEBUG
        for (auto it = m_conf.begin(); it != m_conf.end(); ++it)
            printf("  '%s'\t: %s\n", it->first.c_str(), it->second.c_str());
        #endif

        m_file.close();

        return;
    }

    std::string config::get_string(std::string key) {
        std::map<std::string,std::string>::iterator it;
        if ( (it = m_conf.find(key)) == m_conf.end()) {
            debug_print("Cannot find key '%s'\n", key.c_str());
            return "";
        }
        return it->second;
    }

    bool config::get_bool(std::string key) {
        std::string val = this->get_string(key);
        if (val == "TRUE")
            return true;
        else if (val != "FALSE")
            debug_print("%s is not a valid bool\n", key.c_str());
        return false;
    }

    int config::get_int(std::string key) {
        return std::stoi( this->get_string(key) );
    }

    float config::get_float(std::string key) {
        return std::stof( this->get_string(key) );
    }

}