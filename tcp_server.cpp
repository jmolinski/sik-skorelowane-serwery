#include "tcp_server.hpp"
#include "err.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int LISTEN_QUEUE_LENGTH = 10;

void server::handle_connection(int sock) {
    // Serwer po ustanowieniu połączenia z klientem oczekuje na żądanie klienta.
    // Serwer powinien zakończyć połączenie w przypadku przesłania przez klienta niepoprawnego
    // żądania. W takim przypadku serwer powinien wysłać komunikat o błędzie, ze statusem 400,
    // a następnie zakończyć połączenie.
    //
    // Jeśli żądanie klienta było jednak poprawne, to serwer powinien oczekiwać na
    // ewentualne kolejne żądanie tego samego klienta lub zakończenie połączenia przez
    // klienta.

    FILE *f = fdopen(sock, "rb");
    if (f == nullptr) {
        syserr("opening connection socket failed");
    }

    try {
        run_request_response_loop(f);
    } catch (no_request_to_read_exception const &e) {
        if (fclose(f) == EOF) {
            syserr("closing connection failed");
        }
    }
}

void server::run() {
    int sock = socket(PF_INET, SOCK_STREAM, 0); // Creating IPv4 TCP socket.
    if (sock < 0) {
        syserr("socket");
    }

    sockaddr_in server_address = {
        AF_INET,             // IPv4
        htons(config.port),  // Listening on port config.port.
        {htonl(INADDR_ANY)}, // Listening on all interfaces.
        {0},
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

        handle_connection(msgsock);
    }

    if (close(sock) == -1) {
        syserr("close");
    }
}
