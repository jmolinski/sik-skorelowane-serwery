#ifndef SIK_SKORELOWANE_SERWERY_HTTP_H
#define SIK_SKORELOWANE_SERWERY_HTTP_H

#include "input_parsing.hpp"

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

struct http_headers {};

class http_request {
    void read_status_line(FILE *stream);
    void read_headers(FILE *stream);

  public:
    explicit http_request(FILE *stream);

    request_status_line statusLine;
    http_headers headers;
};

class http_response {
    response_status_line statusLine;

  public:
    http_response();
    explicit http_response(nonfatal_http_communication_exception const& e);
    void send(FILE *stream);
};

#endif // SIK_SKORELOWANE_SERWERY_HTTP_H
