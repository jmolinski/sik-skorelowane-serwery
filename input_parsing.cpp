#include "input_parsing.hpp"
#include <sstream>

void safe_fread_bytes(FILE *f, char *buffer, size_t bytes) {
    size_t bytes_read = fread(buffer, sizeof(char), bytes, f);
    if (bytes_read != bytes) {
        if (feof(f)) {
            throw client_closed_connection();
        }
        throw io_function_error();
    }
}

char safe_fgetc(FILE *f) {
    int c = fgetc(f);
    if (c == EOF) {
        if (feof(f)) {
            throw client_closed_connection();
        }
        throw io_function_error();
    }
    return static_cast<char>(c);
}

std::string read_until_crlf(FILE *f) {
    std::ostringstream os;

    char c = safe_fgetc(f);
    os << c;

    bool previous_char_is_crlf = c == '\r';
    while (true) {
        c = safe_fgetc(f);
        os << c;
        if (c == '\n') {
            if (previous_char_is_crlf) {
                return os.str();
            }
        }
        previous_char_is_crlf = c == '\r';
    }
}
