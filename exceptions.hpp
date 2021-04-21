#ifndef SIK_SKORELOWANE_SERWERY_EXCEPTIONS_H
#define SIK_SKORELOWANE_SERWERY_EXCEPTIONS_H

#include <exception>
#include <string>

class tcp_server_generic_exception : public std::exception {
    std::string customMessage;

    virtual const char *get_default_message() const = 0;

  public:
    tcp_server_generic_exception(){};
    tcp_server_generic_exception(std::string msg) : customMessage(msg) {
    }

    const char *what() const throw() {
        return customMessage.size() ? customMessage.c_str() : get_default_message();
    }
};

class nonfatal_http_communication_exception : public tcp_server_generic_exception {
    const char *get_default_message() const {
        return "nonfatal exception";
    }

  public:
    virtual uint16_t get_status_code() const {
        return 500;
    }
    using tcp_server_generic_exception::tcp_server_generic_exception;
};

class out_of_memory_error : public nonfatal_http_communication_exception {
    const char *get_default_message() const {
        return "out of memory error";
    }

  public:
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class io_function_error : public nonfatal_http_communication_exception {
    const char *get_default_message() const {
        return "internal server error";
    }

  public:
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class invalid_request_error : public nonfatal_http_communication_exception {
    const char *get_default_message() const {
        return "request was invalid";
    }

  public:
    uint16_t get_status_code() const {
        return 400;
    }
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class malformed_request_error : public invalid_request_error {
    const char *get_default_message() const {
        return "request was malformed";
    }

  public:
    using invalid_request_error::invalid_request_error;
};

class not_supported_error : public nonfatal_http_communication_exception {
    const char *get_default_message() const {
        return "request contains not supported elements";
    }

  public:
    uint16_t get_status_code() const {
        return 501;
    }
    using nonfatal_http_communication_exception::nonfatal_http_communication_exception;
};

class client_closed_connection_error : public std::exception {};
class no_request_to_read_exception : public std::exception {};

#endif // SIK_SKORELOWANE_SERWERY_EXCEPTIONS_H
