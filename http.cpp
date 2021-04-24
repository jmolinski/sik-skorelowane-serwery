#include "http.hpp"
#include "exceptions.hpp"
#include <algorithm>
#include <cstring>
#include <regex>
#include <sstream>

constexpr const char *CRLF = "\r\n";
constexpr const char *HTTP_VERSION = "HTTP/1.1";
constexpr uint8_t HTTP_VERSION_SIZE = 8;
constexpr const char *SERVER_NAME = "sik-server";

const std::regex REQUEST_PATH_REGEX(R"r(\/[a-zA-Z0-9.\-\/]*)r");
const std::regex HEADER_NAME_REGEX("[a-zA-Z0-9_-]+");

constexpr size_t BUFFER_SIZE = 4096;

static inline std::string string_to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

http_request::http_request(FILE *stream) : close_connection(false), targetDoesNotExist(false) {
    try {
        read_status_line(stream);
        read_headers(stream);
    } catch (std::bad_alloc &ex) {
        throw out_of_memory_error();
    }

    if (headers.headers.find("content-length") != headers.headers.end()) {
        if (headers.headers.find("content-length")->second != "0") {
            throw invalid_request_error("request content-length greater than 0");
        }
    }

    if (statusLine.method != "GET" && statusLine.method != "HEAD") {
        throw not_supported_error("method not supported");
    } else if (targetDoesNotExist) {
        throw does_not_exist_error("request-target containts illegal characters");
    }
}

void http_request::read_status_line(FILE *stream) {
    statusLine.method = readline_until_delim(stream, ' ');
    statusLine.requestTarget = readline_until_delim(stream, ' ');

    if (statusLine.requestTarget.empty() || statusLine.requestTarget[0] != '/') {
        throw invalid_request_error("request-target must start with /");
    }
    if (!std::regex_match(statusLine.requestTarget, REQUEST_PATH_REGEX)) {
        targetDoesNotExist = true;
    }

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

bool http_request::read_header(FILE *stream) {
    std::string fieldname, fieldvalue;

    if (!parse_headerline(stream, fieldname, fieldvalue)) {
        // End of headers section.
        return false;
    }

    fieldname = string_to_lower(fieldname);

    if (!std::regex_match(fieldname, HEADER_NAME_REGEX)) {
        throw invalid_request_error("header name contains illegal characters");
    }

    if (fieldname == "connection" || fieldname == "content-length") {
        if (headers.headers.find(fieldname) != headers.headers.end()) {
            throw invalid_request_error("contains duplicate headers");
        }
        headers.headers.insert({fieldname, fieldvalue});
    }

    return true;
}

void http_request::read_headers(FILE *stream) {
    bool should_continue;
    do {
        should_continue = read_header(stream);
    } while (should_continue);

    if (headers.headers.find("connection") != headers.headers.end()) {
        std::string fieldvalue = string_to_lower(headers.headers.find("connection")->second);
        if (fieldvalue == "close") {
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

http_response::http_response(nonfatal_http_communication_exception const &e)
    : skip_sending_message_body(true), fileHandle(nullptr) {
    statusLine.httpVersion = HTTP_VERSION;
    statusLine.statusCode = e.get_status_code();
    statusLine.reasonPhrase = e.what();

    headers.headers.insert({"Content-Length", "0"});
    headers.headers.insert({"Content-Type", "application/octet-stream"});
    headers.headers.insert({"Server", SERVER_NAME});

    if (statusLine.statusCode == 400 || statusLine.statusCode == 500 ||
        statusLine.statusCode == 501) {
        set_close_connection_header();
    }
}

http_response::http_response(resource &r, std::string const &method)
    : skip_sending_message_body(true), fileHandle(nullptr) {
    statusLine.httpVersion = HTTP_VERSION;

    if (r.isLocalFile) {
        statusLine.statusCode = 200;
        statusLine.reasonPhrase = "OK";
        headers.headers.insert({"Content-Length", std::to_string(r.filesize)});
        fileSize = r.filesize;
        fileHandle = r.fileHandle;
        r.fileHandle = nullptr;
        skip_sending_message_body = method == "HEAD" || fileSize == 0;
    } else if (r.isRemoteFile) {
        statusLine.statusCode = 302;
        statusLine.reasonPhrase = "Found";
        headers.headers.insert({"Content-Length", "0"});
        headers.headers.insert({"Location", r.remoteLocation});
    } else {
        statusLine.statusCode = 404;
        statusLine.reasonPhrase = "Not found";
        headers.headers.insert({"Content-Length", "0"});
    }

    headers.headers.insert({"Content-Type", "application/octet-stream"});
    headers.headers.insert({"Server", SERVER_NAME});
}

http_response::~http_response() {
    if (fileHandle != nullptr) {
        fclose(fileHandle);
    }
}

void http_response::set_close_connection_header() {
    headers.headers.insert({"Connection", "close"});
}

void http_response::send(FILE *stream) {
    std::string statusLineStr = statusLine.to_string();
    const char *statusLineBuffer = statusLineStr.c_str();

    std::string headersStr = headers.to_string();
    const char *headersBuffer = headersStr.c_str();

    // If sending to client fails no_request_to_read_exception is thrown to force the server
    // to close the connection without failing.
    if (fwrite(statusLineBuffer, 1, strlen(statusLineBuffer), stream) !=
        strlen(statusLineBuffer)) {
        throw no_request_to_read_exception();
    }
    if (fwrite(headersBuffer, 1, strlen(headersBuffer), stream) != strlen(headersBuffer)) {
        throw no_request_to_read_exception();
    }
    if (fwrite(CRLF, 1, strlen(CRLF), stream) != strlen(CRLF)) {
        throw no_request_to_read_exception();
    }

    if (!skip_sending_message_body) {
        char buffer[BUFFER_SIZE];
        size_t toWrite = fileSize;

        while (toWrite > 0) {
            size_t bytesToRead = std::min(toWrite, BUFFER_SIZE);
            safe_fread_bytes(fileHandle, buffer, bytesToRead);
            toWrite -= bytesToRead;
            if (fwrite(buffer, 1, bytesToRead, stream) != bytesToRead) {
                throw no_request_to_read_exception();
            }
        }
    }

    if (fflush(stream) == EOF) {
        throw no_request_to_read_exception();
    }
}
