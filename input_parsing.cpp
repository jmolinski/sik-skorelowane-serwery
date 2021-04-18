#include "input_parsing.hpp"
#include <sstream>

void safe_fread_bytes(FILE *f, char *buffer, size_t bytes) {
    size_t bytesRead = fread(buffer, sizeof(char), bytes, f);
    if (bytesRead != bytes) {
        if (feof(f)) {
            throw client_closed_connection_error();
        }
        throw io_function_error();
    }
}

char safe_fgetc(FILE *f) {
    int c = fgetc(f);
    if (c == EOF) {
        if (feof(f)) {
            throw client_closed_connection_error();
        }
        throw io_function_error();
    }
    return static_cast<char>(c);
}

std::string read_until_crlf(FILE *f) {
    std::ostringstream os;

    char c = safe_fgetc(f);
    os << c;

    bool previousCharIsCr = c == '\r';
    while (true) {
        c = safe_fgetc(f);
        os << c;
        if (c == '\n') {
            if (previousCharIsCr) {
                return os.str();
            }
        }
        previousCharIsCr = c == '\r';
    }
}

std::string readline_until_delim(FILE *f, char delim) {
    std::ostringstream os;

    bool previousCharIsCr = false;

    do {
        char c = safe_fgetc(f);
        os << c;

        if (c == delim) {
            return os.str();
        }

        if (c == '\n') {
            if (previousCharIsCr) {
                throw malformed_request_error();
            }
        }

        previousCharIsCr = c == '\r';
    } while (true);
}
