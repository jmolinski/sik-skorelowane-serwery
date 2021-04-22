#ifndef SIK_SKORELOWANE_SERWERY_CMD_ARGS_PARSER_H
#define SIK_SKORELOWANE_SERWERY_CMD_ARGS_PARSER_H

#include "server_basic_config.hpp"

basic_configuration parse_commandline_arguments(int argc, char *argv[]);

class abstract_server_resource_fs_resolver {
  public:
    virtual FILE *get_file_handle(std::filesystem::path dir, std::string fname,
                                  size_t &fsize) const = 0;
};

class server_resource_fs_resolver : public abstract_server_resource_fs_resolver {
    FILE *get_file_handle(std::filesystem::path dir, std::string fname,
                          size_t &fsize) const override;
};

#endif // SIK_SKORELOWANE_SERWERY_CMD_ARGS_PARSER_H
