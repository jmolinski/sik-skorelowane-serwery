#include "tcp_server.hpp"
#include "err.hpp"
#include "filesystem_interactions.hpp"
#include "http.hpp"
#include "input_parsing.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int LISTEN_QUEUE_LENGTH = 10;

void server::run_request_response_loop(FILE *in_stream, FILE *out_stream) const {
    while (true) {
        try {
            std::cout << "Waiting for a request...\n";
            http_request request(in_stream);
            server_resource_fs_resolver resolver;
            std::cout << "Trying to find the requested resource\n";
            resource optional_resource(resolver, config, request.statusLine.requestTarget);
            std::cout << "Sending a response\n";
            http_response(optional_resource, request.statusLine.method, request.closeConnection)
                .send(out_stream);
        } catch (nonfatal_http_communication_exception const &e) {
            std::cout << "A problem happened, sending a reponse\n";
            http_response(e).send(out_stream);
        }
    }
}

void server::handle_connection(int sock) const {
    int out_sock = dup(sock);
    if (out_sock == -1) {
        fprintf(stderr, "Error when duping connection fd.\n");
        return;
    }
    FILE *fin = fdopen(sock, "r");
    if (fin == nullptr) {
        close(out_sock);
        fprintf(stderr, "Error when opening connection fd as file.\n");
        return;
    }
    FILE *fout = fdopen(out_sock, "w");
    if (fout == nullptr) {
        close(out_sock);
        fclose(fin);
        fprintf(stderr, "Error when opening connection fd as file.\n");
        return;
    }

    try {
        run_request_response_loop(fin, fout);
    } catch (no_request_to_read_exception const &e) {
        if (fclose(fin) == EOF) {
            fprintf(stderr, "Error when closing in file.\n");
        }
        if (fclose(fout) == EOF) {
            fprintf(stderr, "Error when closing out file.\n");
        }
    }
}

void server::run() const {
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        syserr("socket");
    }

    sockaddr_in server_address = {
        AF_INET,             // IPv4
        htons(config.port),  // Listening on port config.port.
        {htonl(INADDR_ANY)}, // Listening on all interfaces.
        {0},
    };

    if (bind(sock, (sockaddr *)&server_address, sizeof(server_address)) < 0) {
        syserr("bind");
    }

    if (listen(sock, LISTEN_QUEUE_LENGTH) < 0) {
        syserr("listen");
    }
    std::cout << "Listening on port " << config.port << std::endl;

    while (true) {
        std::cout << "\nWaiting for connections..." << std::endl;
        int msgsock = accept(sock, nullptr, nullptr);
        if (msgsock == -1) {
            fprintf(stderr, "Error when accepting connection.\n");
            continue;
        }
        std::cout << "Client connected." << std::endl;

        try {
            handle_connection(msgsock);
            std::cout << "Connection closed." << std::endl;
        } catch (...) {
            if (close(sock) == -1) {
                syserr("close");
            }
            fatal("unhandled exception");
        }
    }
}
