#include "filesystem_interactions.hpp"
#include "tcp_server.hpp"

int main(int argc, char *argv[]) {
    server(parse_commandline_arguments(argc, argv)).run();

    return 0;
}
