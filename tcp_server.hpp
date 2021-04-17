#ifndef SIK_SKORELOWANE_SERWERY_TCP_SERVER_H
#define SIK_SKORELOWANE_SERWERY_TCP_SERVER_H

#include <filesystem>

struct basic_configuration {
    const std::filesystem::path filesDirectory;
    const std::filesystem::path correlatedServersFilepath;
    const uint16_t port;
};

class server {
  public:
    explicit server(basic_configuration _config) : config(std::move(_config)) {
    }

    void run();

  private:
    void handle_connection(int);

    basic_configuration config;
};

#endif // SIK_SKORELOWANE_SERWERY_TCP_SERVER_H
