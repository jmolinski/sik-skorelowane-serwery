cmake_minimum_required(VERSION 3.1)
project(sik_skorelowane_serwery)

set(CMAKE_CXX_FLAGS "-g -Wall -Wextra -Werror -Wpedantic")
set(CMAKE_CXX_STANDARD 17)

# http://stackoverflow.com/questions/10555706/
macro(add_executable _name)
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} err)
    endif ()
endmacro()

add_library(err err.cpp)

add_executable(serwer
        server.cpp
        tcp_server.cpp
        filesystem_interactions.cpp
        input_parsing.cpp
        http.cpp
        resource.cpp
        )
target_link_libraries(serwer stdc++fs)

add_executable(test_server
        test_server.cpp
        tcp_server.cpp
        filesystem_interactions.cpp
        input_parsing.cpp
        http.cpp
        resource.cpp
        )
target_link_libraries(test_server stdc++fs)

add_executable(test_http
        test_http.cpp
        input_parsing.cpp
        http.cpp
        resource.cpp
        filesystem_interactions.cpp
        )
target_link_libraries(test_http stdc++fs)

install(TARGETS DESTINATION .)

enable_testing()
add_test(status_line_ok_no_headers test_http 1)
add_test(method_not_supported test_http 2)
add_test(malformed_status_line_path_spaces_1 test_http 3)
add_test(malformed_status_line_httpversion_space test_http 4)
add_test(illegal_chars_in_request_target test_http 5)
add_test(letter_casing_header test_http 6)
add_test(header_no_spaces test_http 7)
add_test(header_no_fieldname test_http 9)
add_test(header_no_fielvalue test_http 10)
add_test(duplicate_ignored_header test_http 11)
add_test(duplicate_not_ignored_header test_http 12)
add_test(content_length_0_header test_http 13)
add_test(content_length_not_parsable_value test_http 14)
add_test(content_length_greater_than_0 test_http 15)
add_test(two_valid_requests test_http 16)
add_test(invalid_header_name test_http 17)
add_test(missing_closing_crlf test_http 18)
add_test(missing_closing_crlf_2 test_http 19)
add_test(empty_string test_http 20)
add_test(cut_header_eof test_http 21)
add_test(target_not_starting_with_slash test_http 22)
add_test(target_ending_with_slash test_http 23)
add_test(too_long_resource_path test_http 24)
add_test(too_long_header_value_path test_http 25)
add_test(request_path_slash_only test_http 26)
add_test(header_value_with_spaces test_http 27)
add_test(request_path_characters_regex test_http 28)

add_test(response_malformed test_http 30)
add_test(response_out_of_mem test_http 31)
add_test(response_method_not_supported test_http 32)
add_test(response_error_closed_connection test_http 33)
add_test(response_not_found test_http 34)
add_test(response_found test_http 35)
add_test(response_local_get test_http 36)
add_test(response_local_head test_http 37)
