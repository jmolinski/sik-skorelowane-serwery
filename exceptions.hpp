#ifndef SIK_SKORELOWANE_SERWERY_EXCEPTIONS_H
#define SIK_SKORELOWANE_SERWERY_EXCEPTIONS_H

#include <exception>
#include <string>

class tcp_server_generic_exception : public std::exception {
    std::string message;
    std::string custom_message;

  public:
    tcp_server_generic_exception(){};
    tcp_server_generic_exception(std::string msg) : custom_message(msg) {
    }

    const char *what() const throw() {
        return custom_message.size() ? custom_message.c_str() : message.c_str();
    }
};

class nonfatal_http_communication_exception : public tcp_server_generic_exception {
    std::string message = "nonfatal exception";

  public:
    const uint16_t status_code = 500;
    using tcp_server_generic_exception::tcp_server_generic_exception;
};

class io_function_error : public nonfatal_http_communication_exception {
    std::string message = "internal server error";

  public:
    const uint16_t status_code = 500;
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class malformed_request_error : public nonfatal_http_communication_exception {
    std::string message = "request was malformed";

  public:
    const uint16_t status_code = 400;
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class invalid_request_error : public nonfatal_http_communication_exception {
    std::string message = "request was invalid";

  public:
    const uint16_t status_code = 400;
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class not_supported_error : public nonfatal_http_communication_exception {
    std::string message = "request contains not supported elements";

  public:
    const uint16_t status_code = 501;
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class client_closed_connection_error : public std::exception {};
class no_request_to_read_exception : public std::exception {};

#endif // SIK_SKORELOWANE_SERWERY_EXCEPTIONS_H
