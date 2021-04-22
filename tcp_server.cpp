#include "tcp_server.hpp"
#include "err.hpp"
#include "filesystem_interactions.hpp"
#include "http.hpp"
#include "input_parsing.hpp"
#include "resource.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

constexpr int LISTEN_QUEUE_LENGTH = 10;

void server::run_request_response_loop(FILE *in_stream, FILE *out_stream) {
    while (true) {
        http_request request;
        try {
            request = http_request(in_stream);
        } catch (invalid_request_error const &e) {
            http_response response(e);
            response.set_close_connection_header();
            response.send(out_stream);
            throw no_request_to_read_exception();
        } catch (nonfatal_http_communication_exception const &e) {
            http_response response(e);
            response.send(out_stream);
            continue;
        } catch (client_closed_connection_error const &e) {
            throw no_request_to_read_exception();
        }

        try {
            server_resource_fs_resolver resolver;
            resource optional_resource =
                resource(resolver, config, request.statusLine.requestTarget);
            http_response response = http_response(optional_resource, request.statusLine.method);
            if (request.close_connection) {
                response.set_close_connection_header();
            }
            response.send(out_stream);
            if (request.close_connection) {
                throw no_request_to_read_exception();
            }
        } catch (nonfatal_http_communication_exception const &e) {
            http_response response(e);
            response.send(out_stream);
        }
    }
}

void server::handle_connection(int sock) {
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
    std::cout << "Listening on port " << config.port << std::endl;

    while (true) {
        std::cout << "Waiting for connections..." << std::endl;
        int msgsock = accept(sock, nullptr, nullptr);
        if (msgsock == -1) {
            syserr("accept");
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
