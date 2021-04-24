#ifndef SIK_SKORELOWANE_SERWERY_HTTP_H
#define SIK_SKORELOWANE_SERWERY_HTTP_H

#include "filesystem_interactions.hpp"
#include "input_parsing.hpp"
#include <map>
#include <vector>

struct request_status_line {
    std::string method;
    std::string requestTarget;
};

struct response_status_line {
    std::string httpVersion;
    uint16_t statusCode;
    std::string reasonPhrase;

    std::string to_string();
};

struct http_headers {
    std::map<std::string, std::string> headers;

    std::string to_string();
};

class http_request {
    void read_status_line(FILE *stream);
    void read_headers(FILE *stream);
    bool read_header(FILE *stream);

  public:
    http_request() {
    }
    explicit http_request(FILE *stream);

    request_status_line statusLine;
    http_headers headers;
    bool close_connection;
    bool targetDoesNotExist;
};

class http_response {
    response_status_line statusLine;
    http_headers headers;
    bool skip_sending_message_body;
    FILE *fileHandle;
    size_t fileSize;

  public:
    http_response(resource &r, std::string const &method);
    explicit http_response(nonfatal_http_communication_exception const &e);
    ~http_response();

    void send(FILE *stream);
    void set_close_connection_header();
};

#endif // SIK_SKORELOWANE_SERWERY_HTTP_H
