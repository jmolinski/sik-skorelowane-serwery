#include "tcp_server.hpp"
#include "err.hpp"
#include "input_parsing.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int LISTEN_QUEUE_LENGTH = 10;

http_request server::read_request(FILE *in_stream, FILE *out_stream) {
    try {
        return http_request(in_stream);
    } catch (malformed_request_error const &e) {
        http_response response(e);
        response.set_close_connection_header();
        response.send(out_stream);
        throw no_request_to_read_exception();
    } catch (nonfatal_http_communication_exception const &e) {
        http_response response(e);
        response.send(out_stream);

        // TODO
        throw no_request_to_read_exception();
    }
}

void server::run_request_response_loop(FILE *in_stream, FILE *out_stream) {
    while (true) {
        http_request request = read_request(in_stream, out_stream);
        // TODO connection: close header

        http_response response;
        response.send(out_stream);

        // Resource(request.statusLine.requestTarget);
    }
}

void server::handle_connection(int sock) {
    // Serwer po ustanowieniu połączenia z klientem oczekuje na żądanie klienta.
    // Serwer powinien zakończyć połączenie w przypadku przesłania przez klienta niepoprawnego
    // żądania. W takim przypadku serwer powinien wysłać komunikat o błędzie, ze statusem 400,
    // a następnie zakończyć połączenie.
    //
    // Jeśli żądanie klienta było jednak poprawne, to serwer powinien oczekiwać na
    // ewentualne kolejne żądanie tego samego klienta lub zakończenie połączenia przez
    // klienta.

    int out_sock = dup(sock);
    if (out_sock == -1) {
        syserr("dup on connection socket failed");
    }
    FILE *fin = fdopen(sock, "r");
    if (fin == nullptr) {
        syserr("opening connection socket failed");
    }
    FILE *fout = fdopen(out_sock, "w");
    if (fout == nullptr) {
        syserr("opening connection socket failed");
    }

    try {
        run_request_response_loop(fin, fout);
    } catch (no_request_to_read_exception const &e) {
        if (fclose(fin) == EOF) {
            syserr("closing connection failed");
        }
        if (fclose(fout) == EOF) {
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
        std::cout << "waiting for conntection\n";
        int msgsock = accept(sock, nullptr, nullptr);
        if (msgsock == -1) {
            syserr("accept");
        }
        std::cout << "connected\n";

        handle_connection(msgsock);
    }

    if (close(sock) == -1) {
        syserr("close");
    }
}
