#ifndef SIK_SKORELOWANE_SERWERY_CMD_ARGS_PARSER_H
#define SIK_SKORELOWANE_SERWERY_CMD_ARGS_PARSER_H

#include <filesystem>
#include <map>

struct basic_configuration {
    const std::filesystem::path filesDirectory;
    const std::map<std::string, std::string> correlatedResources;
    const uint16_t port;
};

basic_configuration parse_commandline_arguments(int argc, char *argv[]);

class server_resource_fs_resolver {
  public:
    virtual FILE *get_file_handle(std::filesystem::path dir, std::string fname,
                                  size_t &fsize) const;
};

struct resource {
    bool isLocalFile;
    bool isRemoteFile;
    size_t filesize;
    FILE *fileHandle;
    std::string remoteLocation;

    resource(server_resource_fs_resolver &pr, basic_configuration const &config,
             std::string const &path);

    ~resource();
};

#endif // SIK_SKORELOWANE_SERWERY_CMD_ARGS_PARSER_H
