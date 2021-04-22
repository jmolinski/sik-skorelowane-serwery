#include "filesystem_interactions.hpp"
#include "err.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace fs = std::filesystem;

constexpr unsigned DEFAULT_PORT = 8080;
constexpr int MINIMAL_VALID_PORT = 1024;

std::map<std::string, std::string> get_correlated_resources(std::string const &fname) {
    std::map<std::string, std::string> resources;

    std::ifstream myfile(fname);
    std::string resource, server, port;
    while (myfile >> resource >> server >> port) {
        if (resources.find(resource) == resources.end()) {
            resources.insert({resource, "http://" + server + ":" + port + resource});
        }
    }

    return resources;
}

basic_configuration parse_commandline_arguments(int argc, char *argv[]) {
    // serwer <nazwa-katalogu-z-plikami> <plik-z-serwerami-skorelowanymi> [<numer-portu-serwera>]

    // Parametr z nazwą katalogu jest parametrem obowiązkowym i może być podany jako ścieżka
    // bezwzględna lub względna. W przypadku ścieżki względnej serwer próbuje odnaleźć wskazany
    // katalog w bieżącym katalogu roboczym.

    // Parametr wskazujący na listę serwerów skorelowanych jest parametrem obowiązkowym

    // Parametr z numerem portu serwera jest parametrem opcjonalnym i wskazuje numer portu
    // na jakim serwer powinien nasłuchiwać połączeń od klientów. Domyślny numer portu to 8080.

    if (argc < 3 || argc > 4) {
        fatal("Usage: %s <files-directory> <correlated-servers-file> [port]", argv[0]);
    }

    fs::path filesDirectory;
    try {
        filesDirectory = fs::path(argv[1]);
        if (!fs::is_directory(filesDirectory)) {
            fatal("First argument must be a directory");
        }
        filesDirectory = fs::canonical(filesDirectory);
    } catch (...) {
        fatal("Could not convert files-directory argument");
    }

    std::map<std::string, std::string> correlatedResources;
    try {
        fs::path correlatedServersFilepath = fs::path(argv[2]);
        if (!fs::is_regular_file(correlatedServersFilepath)) {
            fatal("Second argument must be a regular file");
        }
        correlatedResources = get_correlated_resources(fs::canonical(correlatedServersFilepath));
    } catch (...) {
        fatal("Could not convert or open correlated-servers-file argument");
    }

    uint16_t port = DEFAULT_PORT;
    if (argc == 4) {
        try {
            int int_arg = std::stoi(argv[3]);
            if (int_arg < MINIMAL_VALID_PORT || int_arg > UINT16_MAX) {
                throw std::out_of_range("");
            }
            port = static_cast<uint16_t>(int_arg);
        } catch (...) {
            fatal("Could not convert port argument or value out of range");
        }
    }

    return {filesDirectory, correlatedResources, port};
}

FILE *server_resource_fs_resolver::get_file_handle(std::filesystem::path dir, std::string fname,
                                                   size_t &fsize) const {
    fname = std::string(++fname.begin(), fname.end());
    if (fname.empty()) {
        return nullptr;
    }
    std::filesystem::path relpath = std::filesystem::weakly_canonical(dir / fname);
    if (dir.string() != relpath.string().substr(0, dir.string().size())) {
        return nullptr;
    }
    if (!std::filesystem::is_regular_file(relpath) && !std::filesystem::is_symlink(relpath)) {
        return nullptr;
    }
    std::ifstream my_file(relpath.native());
    if (!my_file.good()) {
        return nullptr;
    }

    fsize = std::filesystem::file_size(relpath);
    return fopen(relpath.c_str(), "r");
}