#include <labdev/ld_debug.hh>

using namespace std;

namespace labdev {

void ld_debug_print(FILE* stream, const char* file, int line, 
    const char* function, const char* msg, ...)
{
    // Print header with file, line number, and function
    std::string fname(file);
    size_t pos = fname.find_last_of("/");
    if (pos != std::string::npos)
        fname = fname.substr(pos +   1, std::string::npos);
    fprintf(stderr, "%s:%d [%s()] - ", fname.c_str(), line, function);

    // Compose message from variable arg list
    va_list args;
    va_start(args, msg);
    vfprintf(stream, msg, args);
    va_end(args);

    return;
}

void ld_print_string_data(FILE* stream, std::string data)
{
    std::string out {};
    bool ellipses_printed {false};
    for (size_t i = 0; i < data.size(); i++) {
        // Skip middle section if data is too long
        if ( (i > 50) && (i < data.size()-50) ) {
            if (!ellipses_printed) {
                out.append("[...]");
                ellipses_printed = true;
            }
            continue;
        }

        // Reformat ASCII special characters
        char c = data.at(i);
        switch (c) {
            case '\n':  out.append("\\n"); break;
            case '\r':  out.append("\\r"); break;
            case '\0':  out.append("\\0"); break;
            default: out.push_back(c);
        }

    }
    fprintf(stream, "'%s'\n", out.c_str());
    return;
}

void ld_print_byte_data(FILE* stream, const uint8_t* data, size_t len)
{
    // Print string data, reformat special characters
    bool ellipses_printed {false};
    for (size_t i = 0; i < len; i++) {
        // Skip middle section if data is too long
        if ( (i > 10) && (i < len-10) ) {
            if (!ellipses_printed) {
                fprintf(stream, "[...] ");
                ellipses_printed = true;
            }
            continue;
        }
        fprintf(stream, "0x%02X ", data[i]);
    }
    fprintf(stream, "\n");
    return;
}

void ld_print_byte_data(FILE* stream, uint8_t* data, size_t len, 
    const char* file, int line, const char* function, const char* msg, ...)
{
    return;
}

}