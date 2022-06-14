cmake_minimum_required(VERSION 3.10)
project(asio_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fsanitize=undefined,address")

message("\n\n============================================================")
message(">>> Compiler & Build information <<<")
string(TIMESTAMP timestamp "[%Y-%m-%d %H:%M:%S] %s" UTC)
message(">>> Build timestamp: ${timestamp}")
message("")
message("C++ VERSION   : ${CMAKE_CXX_STANDARD}")
message("COMPILER      : ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER_VERSION})")
message("COMPILER PATH : ${CMAKE_CXX_COMPILER}")
message("COMPILE FALGS : ${CMAKE_CXX_FLAGS}")
message("CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
message("============================================================\n")

add_subdirectory(boost)

add_executable(asio_server main.cpp)

target_link_libraries(asio_server boost)

project(daytime_server)
add_executable(daytime_server daytime_server.cc)
target_link_libraries(daytime_server boost)
