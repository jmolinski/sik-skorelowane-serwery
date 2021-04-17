#include "err.hpp"
#include <cstdint>
#include <filesystem>
#include <string>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace fs = std::filesystem;

constexpr int LISTEN_QUEUE_LENGTH = 10;
constexpr unsigned DEFAULT_PORT = 8080;

struct basic_configuration {
    const fs::path filesDirectory;
    const fs::path correlatedServersFilepath;
    const uint16_t port;
};

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

class server {
    basic_configuration config;

    void handle_connection(int sock) {
        // Serwer po ustanowieniu połączenia z klientem oczekuje na żądanie klienta.
        // Serwer powinien zakończyć połączenie w przypadku przesłania przez klienta niepoprawnego
        // żądania. W takim przypadku serwer powinien wysłać komunikat o błędzie, ze statusem 400,
        // a następnie zakończyć połączenie.
        //
        // Jeśli żądanie klienta było jednak poprawne, to serwer powinien oczekiwać na
        // ewentualne kolejne żądanie tego samego klienta lub zakończenie połączenia przez
        // klienta.

        if (close(sock) == -1) {
            syserr("close on connection socket failed");
        }
    }

  public:
    explicit server(basic_configuration _config) : config(std::move(_config)) {
    }

    void run() {
        int sock = socket(PF_INET, SOCK_STREAM, 0); // Creating IPv4 TCP socket.
        if (sock < 0) {
            syserr("socket");
        }

        sockaddr_in server_address = {
            .sin_family = AF_INET,           // IPv4
            .sin_port = htons(config.port),  // Listening on port config.port.
            .sin_addr = {htonl(INADDR_ANY)}, // Listening on all interfaces.
            .sin_zero = {0},
        };

        // Bind the socket to a concrete address.
        if (bind(sock, (sockaddr *)&server_address, sizeof(server_address)) < 0) {
            syserr("bind");
        }

        // Switch to listening (passive open).
        if (listen(sock, LISTEN_QUEUE_LENGTH) < 0) {
            syserr("listen");
        }

        // Po uruchomieniu serwer powinien odnaleźć wskazany katalog z plikami i rozpocząć
        // nasłuchiwanie na połączenia TCP od klientów na wskazanym porcie. Jeśli otwarcie
        // katalogu, odczyt z katalogu, bądź otwarcie gniazda sieciowego nie powiodą się, to
        // program powinien zakończyć swoje działanie z kodem błędu EXIT_FAILURE.

        while (true) {
            int msgsock = accept(sock, nullptr, nullptr);
            if (msgsock == -1) {
                syserr("accept");
            }

            if (dup2(msgsock, 0) == -1) {
                syserr("dup2 failed");
            }
            if (close(msgsock) == -1) {
                syserr("close failed");
            }

            handle_connection(msgsock);
        }
    }
};

int main(int argc, char *argv[]) {
    if (fclose(stdin) != 0) {
        syserr("fclose on stdin failed");
    }
    server(parse_commandline_arguments(argc, argv)).run();
    return 0;
}
