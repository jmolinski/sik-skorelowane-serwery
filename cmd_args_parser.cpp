#include "cmd_args_parser.hpp"
#include "err.hpp"
#include <cstdint>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

constexpr unsigned DEFAULT_PORT = 8080;

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

    fs::path correlatedServersFilepath;
    try {
        correlatedServersFilepath = fs::path(argv[2]);
        if (!fs::is_regular_file(correlatedServersFilepath)) {
            fatal("Second argument must be a regular file");
        }
        correlatedServersFilepath = fs::canonical(correlatedServersFilepath);
    } catch (...) {
        fatal("Could not convert correlated-servers-file argument");
    }

    uint16_t port = DEFAULT_PORT;
    if (argc == 4) {
        try {
            int int_arg = std::stoi(argv[3]);
            if (int_arg < 0 || int_arg > UINT16_MAX) {
                throw std::out_of_range("");
            }
            port = static_cast<uint16_t>(int_arg);
        } catch (...) {
            fatal("Could not convert port argument or value out of range");
        }
    }

    return {filesDirectory, correlatedServersFilepath, port};
}
