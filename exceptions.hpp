#ifndef SIK_SKORELOWANE_SERWERY_EXCEPTIONS_H
#define SIK_SKORELOWANE_SERWERY_EXCEPTIONS_H

#include <exception>
#include <string>

class tcp_server_generic_exception : public std::exception {
    std::string message;
    std::string customMessage;

  public:
    tcp_server_generic_exception(){};
    tcp_server_generic_exception(std::string msg) : customMessage(msg) {
    }

    const char *what() const throw() {
        return customMessage.size() ? customMessage.c_str() : message.c_str();
    }
};

class nonfatal_http_communication_exception : public tcp_server_generic_exception {
    std::string message = "nonfatal exception";

  public:
    const uint16_t statusCode = 500;
    using tcp_server_generic_exception::tcp_server_generic_exception;
};

class out_of_memory_error : public nonfatal_http_communication_exception {
    std::string message = "out of memory error";

  public:
    const uint16_t statusCode = 500;
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class io_function_error : public nonfatal_http_communication_exception {
    std::string message = "internal server error";

  public:
    const uint16_t statusCode = 500;
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class invalid_request_error : public nonfatal_http_communication_exception {
    std::string message = "request was invalid";

  public:
    const uint16_t statusCode = 400;
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class malformed_request_error : public invalid_request_error {
    std::string message = "request was malformed";

  public:
    using invalid_request_error::invalid_request_error;
};

class not_supported_error : public nonfatal_http_communication_exception {
    std::string message = "request contains not supported elements";

  public:
    const uint16_t statusCode = 501;
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class client_closed_connection_error : public std::exception {};
class no_request_to_read_exception : public std::exception {};

#endif // SIK_SKORELOWANE_SERWERY_EXCEPTIONS_H
