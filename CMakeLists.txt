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

add_subdirectory(third_party/boost)

set(STREAM_SERVER_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")

add_executable(stream_server
    main.cc
    include/stream/storage_manager.cc)

target_link_libraries(stream_server PUBLIC boost)
target_include_directories(stream_server PUBLIC ${STREAM_SERVER_INCLUDE_DIR})
target_compile_definitions(stream_server PUBLIC STREAM_SERVER_BASE_DIR=${CMAKE_CURRENT_LIST_DIR})

#project(daytime_server)
#add_executable(daytime_server daytime_server.cc)
#target_link_libraries(daytime_server boost)
