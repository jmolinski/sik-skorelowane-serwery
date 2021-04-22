#include "resource.hpp"
#include "exceptions.hpp"

resource::resource(abstract_server_resource_fs_resolver &fileResolver,
                   basic_configuration const &config, std::string const &relFilepath)
    : isLocalFile(false), isRemoteFile(false) {
    try {
        fileHandle = fileResolver.get_file_handle(config.filesDirectory, relFilepath, filesize);
    } catch (...) {
        fileHandle = nullptr;
    }

    if (fileHandle != nullptr) {
        isLocalFile = true;
    } else if (config.correlatedResources.find(relFilepath) != config.correlatedResources.end()) {
        isRemoteFile = true;
        remoteLocation = config.correlatedResources.find(relFilepath)->second;
    }
}

resource::~resource() {
    if (isLocalFile && fileHandle != nullptr) {
        fclose(fileHandle);
    }
}
