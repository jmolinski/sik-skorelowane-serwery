#include "http.hpp"
#include "exceptions.hpp"
#include <algorithm>
#include <cstring>
#include <regex>
#include <sstream>

constexpr const char *CRLF = "\r\n";
constexpr const char *HTTP_VERSION = "HTTP/1.1";
constexpr const char *SERVER_NAME = "sik-server";

const std::regex REQUEST_PATH_REGEX(R"r(\/[a-zA-Z0-9.\-\/]*)r");
const std::regex HEADER_NAME_REGEX("[a-zA-Z0-9_-]+");

constexpr size_t BUFFER_SIZE = 4096;

static inline std::string string_to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

http_request::http_request(FILE *stream) : closeConnection(false), targetDoesNotExist(false) {
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

    char httpVersionBuffer[20] = {0};
    safe_fread_bytes(stream, httpVersionBuffer, strlen(HTTP_VERSION));
    if (strcmp(HTTP_VERSION, httpVersionBuffer) != 0) {
        throw malformed_request_error("malformed status line: http version");
    }

    if (safe_fgetc(stream) != '\r' || safe_fgetc(stream) != '\n') {
        throw malformed_request_error("malformed status line: line end");
    }
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
    while (read_header(stream)) {
    }

    if (headers.headers.find("connection") != headers.headers.end()) {
        closeConnection = string_to_lower(headers.headers.find("connection")->second) == "close";
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
    : skipSendingMessageBody(true), fileHandle(nullptr) {
    statusLine.httpVersion = HTTP_VERSION;
    statusLine.statusCode = e.get_status_code();
    statusLine.reasonPhrase = e.what();

    headers.headers.insert({"Content-Length", "0"});
    headers.headers.insert({"Content-Type", "application/octet-stream"});
    headers.headers.insert({"Server", SERVER_NAME});

    if (statusLine.statusCode == 400 || statusLine.statusCode == 500 ||
        statusLine.statusCode == 501 || e.closeConnection) {
        headers.headers.insert({"Connection", "close"});
    }
}

http_response::http_response(resource &r, std::string const &method, bool closeCommunication)
    : skipSendingMessageBody(true), fileHandle(nullptr) {
    statusLine.httpVersion = HTTP_VERSION;

    if (r.isLocalFile) {
        statusLine.statusCode = 200;
        statusLine.reasonPhrase = "OK";
        headers.headers.insert({"Content-Length", std::to_string(r.filesize)});
        fileSize = r.filesize;
        fileHandle = r.fileHandle;
        r.fileHandle = nullptr;
        skipSendingMessageBody = method == "HEAD" || fileSize == 0;
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
    if (closeCommunication) {
        headers.headers.insert({"Connection", "close"});
    }
}

http_response::~http_response() {
    if (fileHandle != nullptr) {
        fclose(fileHandle);
    }
}

static inline void safe_fwrite_bytes(const char *buffer, size_t count, FILE *f) {
    if (fwrite(buffer, 1, count, f) != count) {
        throw client_closed_connection_error();
    }
}

void http_response::send(FILE *stream) {
    std::string statusLineStr = statusLine.to_string();
    const char *statusLineBuffer = statusLineStr.c_str();
    std::string headersStr = headers.to_string();
    const char *headersBuffer = headersStr.c_str();

    safe_fwrite_bytes(statusLineBuffer, strlen(statusLineBuffer), stream);
    safe_fwrite_bytes(headersBuffer, strlen(headersBuffer), stream);
    safe_fwrite_bytes(CRLF, strlen(CRLF), stream);

    if (!skipSendingMessageBody) {
        char buffer[BUFFER_SIZE];
        size_t toWrite = fileSize;

        while (toWrite > 0) {
            size_t bytesToRead = std::min(toWrite, BUFFER_SIZE);
            safe_fread_bytes(fileHandle, buffer, bytesToRead);
            toWrite -= bytesToRead;
            safe_fwrite_bytes(buffer, bytesToRead, stream);
        }
    }

    if (fflush(stream) == EOF) {
        throw client_closed_connection_error();
    }
    if (headers.headers.find("Connection") != headers.headers.end()) {
        throw no_request_to_read_exception();
    }
}
