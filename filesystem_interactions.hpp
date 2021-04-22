#ifndef SIK_SKORELOWANE_SERWERY_CMD_ARGS_PARSER_H
#define SIK_SKORELOWANE_SERWERY_CMD_ARGS_PARSER_H

#include "tcp_server.hpp"

basic_configuration parse_commandline_arguments(int argc, char *argv[]);

class server_resource_fs_resolver {
  public:
    virtual FILE *get_file_handle(std::filesystem::path dir, std::string fname,
                                  size_t &fsize) const;
};

#endif // SIK_SKORELOWANE_SERWERY_CMD_ARGS_PARSER_H
