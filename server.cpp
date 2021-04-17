#include "cmd_args_parser.hpp"
#include "err.hpp"
#include "tcp_server.hpp"

int main(int argc, char *argv[]) {
    if (fclose(stdin) != 0) {
        syserr("fclose on stdin failed");
    }
    server(parse_commandline_arguments(argc, argv)).run();

    return 0;
}
