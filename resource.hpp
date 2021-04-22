#ifndef SIK_SKORELOWANE_SERWERY_RESOURCE_HPP
#define SIK_SKORELOWANE_SERWERY_RESOURCE_HPP

#include "filesystem_interactions.hpp"
#include "server_basic_config.hpp"

struct resource {
    bool isLocalFile;
    bool isRemoteFile;
    size_t filesize;
    FILE *fileHandle;
    std::string remoteLocation;

    resource(abstract_server_resource_fs_resolver &pr, basic_configuration const &config,
             std::string const &path);

    ~resource();
};

#endif // SIK_SKORELOWANE_SERWERY_RESOURCE_HPP
