cmake_minimum_required(VERSION 3.25)
project(daemon)

set(CMAKE_CXX_STANDARD 17 "-Wall -Wextra -Werror -Wpedantic")

add_library(Daemon
        include/Daemon.h
        src/Daemon.cpp
        include/FileHandles.h
        src/FileHandles.cpp
        include/Request.h include/Result.h include/Payload.h include/DaemonException.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../library/include)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(Daemon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Daemon Library)