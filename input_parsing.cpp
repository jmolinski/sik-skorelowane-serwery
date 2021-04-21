#include "input_parsing.hpp"
#include <algorithm>
#include <sstream>

constexpr uint16_t MAX_TOKEN_LENGTH = 8192;

static inline std::string rstrip(const std::string &inpt) {
    if (inpt.empty()) {
        return inpt;
    }
    auto end_it = inpt.rbegin();
    while (*end_it == ' ') {
        ++end_it;
    }
    return std::string(inpt.begin(), end_it.base());
}

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
    uint16_t len = 0;
    bool previousCharIsCr = false;
    do {
        char c = safe_fgetc(f);
        if (c == delim) {
            return os.str();
        }
        os << c;
        len++;
        if (len > MAX_TOKEN_LENGTH) {
            throw invalid_request_error("too token in status line");
        }

        if (c == '\n' && previousCharIsCr) {
            throw malformed_request_error();
        }
        previousCharIsCr = c == '\r';
    } while (true);
}

char read_headerline_token(FILE *stream, std::stringstream &os, char first_char, char delim) {
    char c = first_char;
    uint16_t len = 0;
    bool previousCharIsCr = c == '\r';
    while (c != delim) {
        os << c;
        len++;
        if (len > MAX_TOKEN_LENGTH) {
            throw invalid_request_error("too long header name or value");
        }

        c = safe_fgetc(stream);

        if (c == '\n' && previousCharIsCr) {
            throw malformed_request_error("can't parse header line");
        }
    }

    return c;
}

std::string readline_until_crlf(FILE *stream, char first_char) {
    std::ostringstream os;
    os << first_char;
    uint16_t len = 1;
    bool previousCharIsCr = first_char == '\r';
    do {
        char c = safe_fgetc(stream);
        if (c == '\n' && previousCharIsCr) {
            auto s = os.str();
            return std::string(s.begin(), (++s.rbegin()).base());
        }
        os << c;
        len++;
        if (len > MAX_TOKEN_LENGTH) {
            throw invalid_request_error("too long token or malformed end of line");
        }
        previousCharIsCr = c == '\r';
    } while (true);
}

bool parse_headerline(FILE *stream, std::string &fieldname, std::string &fieldvalue) {
    std::stringstream os;

    // Handle potential \r\n denoting the end of headers section.
    char c = safe_fgetc(stream);
    if (c == ':') {
        throw malformed_request_error("header line without header name");
    }
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

    read_headerline_token(stream, os, c, ':');
    fieldname = os.str();
    os.str("");
    os.clear();

    do {
        c = safe_fgetc(stream);
    } while (c == ' ');

    fieldvalue = rstrip(readline_until_crlf(stream, c));
    return true;
}
