cmake_minimum_required (VERSION 3.1)
project(sik_skorelowane_serwery)

set(CMAKE_CXX_FLAGS "-g -Wall")
set(CMAKE_CXX_STANDARD 17)

# http://stackoverflow.com/questions/10555706/
macro (add_executable _name)
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} err)
    endif()
endmacro()

add_library(err err.cpp)

add_executable(serwer
        server.cpp)
target_link_libraries(serwer stdc++fs)

install(TARGETS DESTINATION .)
