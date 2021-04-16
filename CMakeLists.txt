cmake_minimum_required(VERSION 3.19)
project(sik_skorelowane_serwery)

set(CMAKE_CXX_STANDARD 17)

add_library(err err.c)

add_executable(serwer
        server.cpp)
target_link_libraries(serwer err)
