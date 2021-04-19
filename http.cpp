#include "http.hpp"
#include "exceptions.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>

constexpr const char *CRLF = "\r\n";
constexpr const char *HTTP_VERSION = "HTTP/1.1";
constexpr uint8_t HTTP_VERSION_SIZE = 8;
constexpr const char *SERVER_NAME = "sik-server";

static inline bool is_ignored_header(std::string const &s) {
    return s != "content-type" && s != "connection" && s != "content-length" && s != "server";
}

http_request::http_request(FILE *stream) : has_invalid_headers(false), close_connection(false) {
    if (feof(stream)) {
        throw no_request_to_read_exception();
    }

    read_status_line(stream);
    read_headers(stream);

    if (headers.headers.find("content-length") != headers.headers.end()) {
        auto it = headers.headers.find("content-length");
        if (it->second != "0") {
            try {
                unsigned cnt = std::stoi(it->second);
                for (unsigned i = 0; i < cnt; ++i) {
                    safe_fgetc(stream);
                }
            } catch (...) {
                throw malformed_request_error("can't parse content-length header");
            }
            throw invalid_request_error("request content-length greater than 0");
        }
    }

    if (statusLine.method != "GET" && statusLine.method != "HEAD") {
        throw not_supported_error("method not supported");
    }
    if (has_invalid_headers) {
        throw invalid_request_error("invalid headers");
    }
}

void http_request::read_status_line(FILE *stream) {
    statusLine.method = readline_until_delim(stream, ' ');
    statusLine.requestTarget = readline_until_delim(stream, ' ');

    char httpVersionBuffer[HTTP_VERSION_SIZE + 1] = {0};
    safe_fread_bytes(stream, httpVersionBuffer, HTTP_VERSION_SIZE);
    if (strcmp(HTTP_VERSION, httpVersionBuffer) != 0) {
        throw malformed_request_error("malformed status line: http version");
    }

    // TODO
    std::cout << statusLine.method << ' ' << statusLine.requestTarget << ' ' << HTTP_VERSION
              << '\n';

    if (safe_fgetc(stream) == '\r') {
        if (safe_fgetc(stream) == '\n') {
            return;
        }
    }

    throw malformed_request_error("malformed status line: line end");
}

bool http_request::read_header(FILE *stream) {
    std::string fieldname, fieldvalue;

    if (!parse_headerline(stream, fieldname, fieldvalue)) {
        // End of headers section.
        return false;
    }

    std::transform(fieldname.begin(), fieldname.end(), fieldname.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (!is_ignored_header(fieldname)) {
        if (headers.headers.find(fieldname) != headers.headers.end()) {
            has_invalid_headers = true;
        } else {
            headers.headers.insert({fieldname, fieldvalue});
        }
    }

    return true;
}

void http_request::read_headers(FILE *stream) {
    bool should_continue;
    do {
        should_continue = read_header(stream);
    } while (should_continue);

    if (headers.headers.find("connection") != headers.headers.end()) {
        if (headers.headers.find("connection")->second == "close") {
            close_connection = true;
        }
    }
}

std::string response_status_line::to_string() {
    std::ostringstream os;
    os << httpVersion << ' ' << statusCode << ' ' << reasonPhrase << CRLF;
    return os.str();
}

std::string http_headers::to_string() {
    std::ostringstream os;
    for (auto it : headers) {
        os << it.first << ": " << it.second << CRLF;
    }
    return os.str();
}

http_response::http_response() {
    statusLine.httpVersion = HTTP_VERSION;
    statusLine.statusCode = 200;
    statusLine.reasonPhrase = "OK";
}

http_response::http_response(nonfatal_http_communication_exception const &e) {
    statusLine.httpVersion = HTTP_VERSION;
    statusLine.statusCode = e.statusCode;
    statusLine.reasonPhrase = e.what();

    headers.headers.insert({"Content-Length", "0"});
    headers.headers.insert({"Content-Type", "application/octet-stream"});
    headers.headers.insert({"Server", SERVER_NAME});
}

void http_response::send(FILE *stream) {
    std::string statusLineStr = statusLine.to_string();
    const char *statusLineBuffer = statusLineStr.c_str();

    std::string headersStr = headers.to_string();
    const char *headersBuffer = headersStr.c_str();

    // TODO error handling
    fwrite(statusLineBuffer, 1, strlen(statusLineBuffer), stream);
    fwrite(headersBuffer, 1, strlen(headersBuffer), stream);
    fwrite(CRLF, 1, strlen(CRLF), stream);

    if (data.size()) {
        fwrite(data.data(), 1, data.size(), stream);
    }
}
