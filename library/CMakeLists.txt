cmake_minimum_required(VERSION 3.25)
project(library)

set(CMAKE_CXX_STANDARD 17 "-Wall -Wextra -Werror -Wpedantic")

add_library(Library
        include/Libfs.h
        src/Libfs.cpp
        include/LibraryException.h
        include/Ipc.h
        src/Ipc.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../daemon/include)

target_include_directories(Library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)