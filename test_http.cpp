#include "http.hpp"
#include <cassert>
#include <cstdio>

struct File {
    FILE *f;

    File(std::string filename) {
        std::string path = "/home/jmolinski/dv/sik/sik-skorelowane-serwery/test_data/" + filename;
        f = fopen(path.c_str(), "r");
        if (f == nullptr) {
            printf("fopen failed!\n");
            exit(1);
        }
    }
    ~File() {
        fclose(f);
    }
};

template <typename T>
void assert_raises(std::string const &fname) {
    File f = (fname);
    try {
        http_request hr(f.f);
        assert(false);
    } catch (T &e) {
    } catch (...) {
        assert(false);
    }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    assert(argc > 0);
    int test_num = std::stoi(argv[1]);

    switch (test_num) {
        case 1: {
            File f("status_line_ok_no_headers");
            http_request hr(f.f);
            assert(hr.statusLine.method == "HEAD");
            assert(hr.statusLine.requestTarget == "/");
            assert(!hr.close_connection);
            break;
        }
        case 2: {
            assert_raises<not_supported_error>("method_not_supported");
            break;
        }
        case 3: {
            assert_raises<invalid_request_error>("malformed_status_line_path_spaces_1");
            break;
        }
        case 4: {
            assert_raises<malformed_request_error>("malformed_status_line_httpversion_space");
            break;
        }
        case 5: {
            assert_raises<invalid_request_error>("illegal_chars_in_request_target");
            break;
        }
        case 6: {
            File f("letter_casing_header");
            http_request hr(f.f);
            assert(hr.headers.headers.size() == 1);
            assert(hr.headers.headers.find("connection")->second == "close");
            assert(hr.close_connection);
            break;
        }
        case 7: {
            File f("header_no_spaces");
            http_request hr(f.f);
            assert(hr.headers.headers.find("connection")->second == "close");
            assert(hr.close_connection);
            break;
        }
        case 8: {
            assert_raises<invalid_request_error>("invalid_connection_header_value");
            break;
        }
        case 9: {
            assert_raises<invalid_request_error>("header_no_fieldname");
            break;
        }
        case 10: {
            assert_raises<invalid_request_error>("header_no_fielvalue");
            break;
        }
        case 11: {
            File f("duplicate_ignored_header");
            http_request hr(f.f);
            assert(hr.headers.headers.size() == 0);
            break;
        }
        case 12: {
            assert_raises<invalid_request_error>("duplicate_not_ignored_header");
            break;
        }
        case 13: {
            File f("content_length_0_header");
            http_request hr(f.f);
            assert(hr.headers.headers.size() == 1);
            break;
        }
        case 14: {
            assert_raises<invalid_request_error>("content_length_not_parsable_value");
            break;
        }
        case 15: {
            assert_raises<invalid_request_error>("content_length_greater_than_0");
            break;
        }
        case 16: {
            File f("two_valid_requests");
            http_request hr1(f.f);
            http_request hr2(f.f);

            assert(hr1.statusLine.method == "GET");
            assert(hr2.statusLine.method == "HEAD");
            break;
        }
    }

    return 0;
}
