cmake_minimum_required(VERSION 3.10)
project(asio_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

message("\n\n============================================================")
message(">>> Compiler & Build information <<<")
message(">>> Build timestamp: ${timestamp}")
message("")
message("CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message("Compiler: ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER_VERSION})")
message("CMAKE_CXX_COMPILER:")
message("> ${CMAKE_CXX_COMPILER}")
message("============================================================\n")

add_subdirectory(boost)

add_executable(asio_server main.cpp)

target_link_libraries(asio_server boost)
