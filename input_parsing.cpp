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

std::string readline_until_delim(FILE *f, char delim) {
    std::ostringstream os;

    bool previousCharIsCr = false;
    do {
        char c = safe_fgetc(f);
        if (c == delim) {
            return os.str();
        }
        os << c;
        if (c == '\n' && previousCharIsCr) {
            throw malformed_request_error();
        }
        previousCharIsCr = c == '\r';
    } while (true);
}

char read_headerline_token(FILE *stream, std::stringstream &os, char first_char, char delim1,
                           char delim2) {
    char c = first_char;
    bool previousCharIsCr = c == '\r';
    while (c != delim1 && c != delim2) {
        os << c;
        c = safe_fgetc(stream);

        if (c == '\n' && previousCharIsCr) {
            throw malformed_request_error("can't parse header line");
        }
    }

    return c;
}

bool parse_headerline(FILE *stream, std::string &fieldname, std::string &fieldvalue) {
    std::stringstream os;

    // Handle potential \r\n denoting the end of headers section.
    char c = safe_fgetc(stream);
    if (c == '\r') {
        c = safe_fgetc(stream);
        if (c == '\n') {
            return false;
        }
        os << '\r';
    } else {
        os << c;
        c = safe_fgetc(stream);
    }

    read_headerline_token(stream, os, c, ':', ':');
    fieldname = os.str();
    os.clear();

    do {
        c = safe_fgetc(stream);
    } while (c == ' ');

    c = read_headerline_token(stream, os, c, ' ', '\r');
    fieldvalue = os.str();

    while (c == ' ') {
        c = safe_fgetc(stream);
    }

    if (c == '\r') {
        if (safe_fgetc(stream) == '\n') {
            return true;
        }
    }
    throw malformed_request_error("malformed end of header line");
}
