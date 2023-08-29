cmake_minimum_required(VERSION 3.25)
project(filesystem)

set(CMAKE_CXX_STANDARD 17 "-Wall -Wextra -Werror -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

add_subdirectory(daemon)
add_subdirectory(library)
add_subdirectory(tests)

add_executable(daemon_main daemon_main.cpp)
target_link_libraries(daemon_main LINK_PUBLIC Daemon)

target_link_libraries(tests Library Daemon)
target_link_libraries(test_multiple_signal Library Daemon)
target_link_libraries(test_multiple_writers Library Daemon)
target_link_libraries(test_multiple_diff_files Library Daemon)
