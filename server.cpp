#include "filesystem_interactions.hpp"
#include "tcp_server.hpp"
#include <signal.h>

int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE.

    server(parse_commandline_arguments(argc, argv)).run();

    return 0;
}
