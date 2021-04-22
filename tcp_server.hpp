#ifndef SIK_SKORELOWANE_SERWERY_TCP_SERVER_H
#define SIK_SKORELOWANE_SERWERY_TCP_SERVER_H

#include "server_basic_config.hpp"

class server {
  public:
    explicit server(basic_configuration _config) : config(std::move(_config)) {
    }

    void run();

  private:
    void handle_connection(int);
    void run_request_response_loop(FILE *in_stream, FILE *out_stream);

    basic_configuration config;
};

#endif // SIK_SKORELOWANE_SERWERY_TCP_SERVER_H
