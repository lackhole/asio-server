cmake_minimum_required(VERSION 3.17)
project(asio_server)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(boost)

add_executable(asio_server main.cpp)

target_link_libraries(asio_server boost)