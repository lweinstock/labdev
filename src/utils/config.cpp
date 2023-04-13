#include <labdev/utils/config.hh>
#include <labdev/ld_debug.hh>
#include <labdev/exceptions.hh>

using namespace std;

namespace labdev {

config::config(string conf_path) {
    m_file.open(conf_path, ifstream::in);

    debug_print("Reading configuration '%s':\n", conf_path.c_str());
    if ( m_file.fail() ) {
        printf("Cannot open file %s\n", conf_path.c_str());
        abort();
    }

    // Extract all line from config file
    string line;
    while ( getline(m_file, line) ) {
        // Skip empty line
        if ( line.size() == 0 ) continue;
        // Skip comments starting with "#"
        if ( line.at(line.find_first_not_of(' ')) == '#' ) continue;
        stringstream line_stream(line);

        // First element is the key, rest of line is the value for that key
        string key, value;
        line_stream >> key;
        line_stream >> ws; // skip whitespaces
        getline(line_stream, value);

        m_conf.insert( pair<string,string>(key, value) );
    }
    #ifdef LD_DEBUG
    for (auto it = m_conf.begin(); it != m_conf.end(); ++it)
        printf("  '%s'\t: %s\n", it->first.c_str(), it->second.c_str());
    #endif

    m_file.close();

    return;
}

string config::get_string(string key) {
    map<string,string>::iterator it;
    if ( (it = m_conf.find(key)) == m_conf.end())
        throw labdev::exception("Cannot find key '" + key + "'");
    return it->second;
}

bool config::get_bool(string key) {
    string val = this->get_string(key);
    if (val == "TRUE")
        return true;
    else if (val != "FALSE")
        throw labdev::exception("Key '" + key + "' is not a valid bool!");
    return false;
}

int config::get_int(string key) {
    return stoi( this->get_string(key) );
}

float config::get_float(string key) {
    return stof( this->get_string(key) );
}

}