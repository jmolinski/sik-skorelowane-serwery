#ifndef SIK_SKORELOWANE_SERWERY_HTTP_H
#define SIK_SKORELOWANE_SERWERY_HTTP_H

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

    bool has_invalid_headers;
    bool close_connection;

  public:
    explicit http_request(FILE *stream);

    request_status_line statusLine;
    http_headers headers;
};

class http_response {
    response_status_line statusLine;
    http_headers headers;
    std::vector<char> data;

  public:
    http_response();
    explicit http_response(nonfatal_http_communication_exception const &e);
    void send(FILE *stream);
};

#endif // SIK_SKORELOWANE_SERWERY_HTTP_H
