#include "filesystem_interactions.hpp"
#include "http.hpp"
#include <cassert>
#include <cstdio>
#include <iostream>

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

class fake_fs_not_exists : public server_resource_fs_resolver {
  public:
    FILE *get_file_handle(std::filesystem::path dir, std::string fname, size_t &fsize) const {
        (void)dir;
        (void)fname;
        fsize = 0;
        return nullptr;
    }
};

class fake_resolver_ret_handle : public server_resource_fs_resolver {
    char *tab;
    size_t data_size;

  public:
    fake_resolver_ret_handle(std::string data) {
        data_size = data.size();
        tab = new char[data.size() + 3];
        for (size_t i = 0; i < data.size() + 3; i++) {
            tab[i] = i < data.size() ? data[i] : 0;
        }
    }

    ~fake_resolver_ret_handle() {
        delete[] tab;
    }

    FILE *get_file_handle(std::filesystem::path dir, std::string fname, size_t &fsize) const {
        (void)dir;
        (void)fname;
        fsize = data_size;
        return fmemopen(tab, data_size, "r");
    }
};

basic_configuration empty_basic_config() {
    return basic_configuration{"", {}, 2137};
}

basic_configuration basic_config_res(std::map<std::string, std::string> resources) {
    return basic_configuration{"", resources, 2137};
}

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

std::string render_hr_response(http_response &hr, size_t bufferSize = 1000) {
    char *tab = new char[bufferSize];
    for (size_t i = 0; i < bufferSize; i++) {
        tab[i] = 0;
    }
    FILE *file = fmemopen(tab, bufferSize, "w");
    hr.send(file);

    std::string s = std::string(tab);
    delete[] tab;
    return s;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    assert(argc > 0);
    int test_num = std::stoi(argv[1]);

    switch (test_num) {
        case 1: {
            File f("status_line_ok_no_headers");
            http_request hr(f.f);
            assert(hr.statusLine.method == "HEAD");
            assert(hr.statusLine.requestTarget == "/a");
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
            assert_raises<does_not_exist_error>("illegal_chars_in_request_target");
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
        case 9: {
            assert_raises<invalid_request_error>("header_no_fieldname");
            break;
        }
        case 10: {
            File f("header_no_fielvalue");
            http_request hr(f.f);
            assert(hr.headers.headers.size() == 0);
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
        case 17: {
            assert_raises<invalid_request_error>("invalid_header_name");
            break;
        }
        case 18: {
            assert_raises<client_closed_connection_error>("missing_closing_crlf");
            break;
        }
        case 19: {
            assert_raises<client_closed_connection_error>("missing_closing_crlf_2");
            break;
        }
        case 20: {
            assert_raises<client_closed_connection_error>("empty_string");
            break;
        }
        case 21: {
            assert_raises<client_closed_connection_error>("cut_header_eof");
            break;
        }
        case 22: {
            assert_raises<invalid_request_error>("target_not_starting_with_slash");
            break;
        }
        case 23: {
            File f("target_ending_with_slash");
            http_request hr(f.f);
            break;
        }
        case 24: {
            assert_raises<invalid_request_error>("too_long_resource_path");
            break;
        }
        case 25: {
            assert_raises<invalid_request_error>("too_long_header_value_path");
            break;
        }
        case 26: {
            File f("request_path_slash_only");
            http_request hr(f.f);
            assert(hr.statusLine.method == "HEAD");
            assert(hr.statusLine.requestTarget == "/");
            break;
        }
        case 27: {
            File f("header_value_with_spaces");
            http_request hr(f.f);
            assert(hr.statusLine.method == "GET");
            assert(hr.statusLine.requestTarget == "/");
            assert(hr.headers.headers.size() == 1);
            assert(hr.close_connection);
            break;
        }
        case 28: {
            File f("request_path_characters_regex");
            http_request hr(f.f);
            assert(hr.statusLine.requestTarget == "/.-aAzZ09/-");
            break;
        }
        case 29: {
            assert_raises<invalid_request_error>("request_header_without_colon");
            break;
        }

            // --------------------------------------------------

        case 30: {
            http_response hr(malformed_request_error("test"));

            std::string s = render_hr_response(hr);
            std::string expected =
                "HTTP/1.1 400 test\r\nConnection: close\r\nContent-Length: 0\r\nContent-Type: "
                "application/octet-stream\r\nServer: sik-server\r\n\r\n";
            assert(s == expected);
            break;
        }
        case 31: {
            http_response hr(out_of_memory_error{});

            std::string s = render_hr_response(hr);
            std::string expected = "HTTP/1.1 500 out of memory error\r\nConnection: "
                                   "close\r\nContent-Length: 0\r\nContent-Type: "
                                   "application/octet-stream\r\nServer: sik-server\r\n\r\n";
            assert(s == expected);
            break;
        }
        case 32: {
            http_response hr(not_supported_error("aaa"));

            std::string s = render_hr_response(hr);
            std::string expected =
                "HTTP/1.1 501 aaa\r\nConnection: close\r\nContent-Length: 0\r\nContent-Type: "
                "application/octet-stream\r\nServer: sik-server\r\n\r\n";
            assert(s == expected);
            break;
        }
        case 33: {
            FILE *file = fmemopen(nullptr, 0, "w");
            http_response hr(not_supported_error("aaa"));

            try {
                hr.send(file);
            } catch (no_request_to_read_exception &e) {
                break;
            }
            assert(false);
        }
        case 34: {
            auto resolver = fake_fs_not_exists();
            resource res(resolver, empty_basic_config(), "/resource");
            http_response hr(res, "HEAD");

            std::string s = render_hr_response(hr);
            std::string expected = "HTTP/1.1 404 Not found\r\nContent-Length: 0\r\nContent-Type: "
                                   "application/octet-stream\r\nServer: sik-server\r\n\r\n";

            assert(s == expected);
            break;
        }
        case 35: {
            auto resolver = fake_fs_not_exists();
            resource res(resolver, basic_config_res({{"/resource", "test_location"}}),
                         "/resource");
            http_response hr(res, "HEAD");

            std::string s = render_hr_response(hr);
            std::string expected = "HTTP/1.1 302 Found\r\nContent-Length: 0\r\nContent-Type: "
                                   "application/octet-stream\r\nLocation: "
                                   "test_location\r\nServer: sik-server\r\n\r\n";

            assert(s == expected);
            break;
        }
        case 36: {
            auto resolver = fake_resolver_ret_handle("some data");
            resource res(resolver, empty_basic_config(), "/resource");
            http_response hr(res, "GET");

            std::string s = render_hr_response(hr);
            std::string expected =
                "HTTP/1.1 200 OK\r\nContent-Length: 9\r\nContent-Type: "
                "application/octet-stream\r\nServer: sik-server\r\n\r\nsome data";

            std::cout << s << '\n';
            std::cout.flush();

            assert(s == expected);
            break;
        }
        case 37: {
            auto resolver = fake_resolver_ret_handle("some data");
            resource res(resolver, empty_basic_config(), "/resource");
            http_response hr(res, "HEAD");

            std::string s = render_hr_response(hr);
            std::string expected = "HTTP/1.1 200 OK\r\nContent-Length: 9\r\nContent-Type: "
                                   "application/octet-stream\r\nServer: sik-server\r\n\r\n";

            assert(s == expected);
            break;
        }
        case 38: {
            http_response hr(does_not_exist_error("aaa"));

            std::string s = render_hr_response(hr);
            std::string expected = "HTTP/1.1 404 aaa\r\nContent-Length: 0\r\nContent-Type: "
                                   "application/octet-stream\r\nServer: sik-server\r\n\r\n";
            assert(s == expected);
            break;
        }
    }

    return 0;
}
