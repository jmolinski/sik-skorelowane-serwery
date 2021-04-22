#include "err.hpp"
#include "tcp_server.hpp"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    if (fclose(stdin) != 0) {
        syserr("fclose on stdin failed");
    }

    server({std::filesystem::path("."), std::map<std::string, std::string>(), 8890}).run();

    return 0;
}
