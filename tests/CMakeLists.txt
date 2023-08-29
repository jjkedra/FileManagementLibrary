cmake_minimum_required(VERSION 3.25)
project(tests)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Werror -Wpedantic)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(tests tests.cpp)
add_executable(test_multiple_signal multiple_processes_signal_test.cpp)
add_executable(test_multiple_writers multiple_processes_writers_test.cpp)
add_executable(test_multiple_diff_files multiple_processes_diff_files_test.cpp)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY  https://github.com/google/googletest.git
        GIT_TAG         release-1.11.0
)
FetchContent_MakeAvailable(googletest)

target_link_libraries(tests gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
