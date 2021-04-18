#include "http.hpp"
#include "exceptions.hpp"
#include <cstring>
#include <sstream>
#include <iostream>

constexpr const char *CRLF = "\r\n";
constexpr const char *HTTP_VERSION = "HTTP/1.1";
constexpr uint8_t HTTP_VERSION_SIZE = 8;

http_request::http_request(FILE *stream) {
    if (feof(stream)) {
        throw no_request_to_read_exception();
    }

    read_status_line(stream);
    read_headers(stream);

    if (statusLine.method != "GET" && statusLine.method != "HEAD") {
        std::cout << statusLine.method << '\n';
        throw not_supported_error("method not supported");
    }

    // TODO content-length > 0 error
}

void http_request::read_status_line(FILE *stream) {
    statusLine.method = readline_until_delim(stream, ' ');
    std::cout << statusLine.method << '\n';
    statusLine.requestTarget = readline_until_delim(stream, ' ');
    std::cout << statusLine.requestTarget << '\n';

    char httpVersionBuffer[HTTP_VERSION_SIZE + 1] = {0};
    safe_fread_bytes(stream, httpVersionBuffer, HTTP_VERSION_SIZE);
    if (strcmp(HTTP_VERSION, httpVersionBuffer) != 0) {
        throw malformed_request_error("malformed status line: http version");
    }

    if (safe_fgetc(stream) == '\r') {
        if (safe_fgetc(stream) == '\n') {
            return;
        }
    }

    throw malformed_request_error("malformed status line: line end");
}

void http_request::read_headers(FILE *stream) {
    safe_fread_bytes(stream, nullptr, 0);
}

std::string response_status_line::to_string() {
    std::ostringstream os;
    os << httpVersion << ' ' << statusCode << ' ' << reasonPhrase << CRLF;
    return os.str();
}

http_response::http_response() {
    statusLine.httpVersion = HTTP_VERSION;
    statusLine.statusCode = 200;
    statusLine.reasonPhrase = "OK";
}

http_response::http_response(nonfatal_http_communication_exception const& e) {
    statusLine.httpVersion = HTTP_VERSION;
    statusLine.statusCode = e.statusCode;
    statusLine.reasonPhrase = e.what();

    // TODO content-length
}

void http_response::send(FILE *stream) {
    std::string statusLineStr = statusLine.to_string();
    const char* statusLineBuffer = statusLineStr.c_str();
    fwrite(statusLineBuffer, 1, strlen(statusLineBuffer), stream);
    fwrite(CRLF, 1, strlen(CRLF), stream);

    // TODO
}
