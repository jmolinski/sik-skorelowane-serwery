#ifndef SIK_SKORELOWANE_SERWERY_SERVER_BASIC_CONFIG_H
#define SIK_SKORELOWANE_SERWERY_SERVER_BASIC_CONFIG_H

#include <filesystem>
#include <map>

struct basic_configuration {
    const std::filesystem::path filesDirectory;
    const std::map<std::string, std::string> correlatedResources;
    const uint16_t port;
};

#endif // SIK_SKORELOWANE_SERWERY_SERVER_BASIC_CONFIG_H
