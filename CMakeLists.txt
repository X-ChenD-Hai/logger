cmake_minimum_required(VERSION 3.10)
project(logger LANGUAGES CXX C)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(@cpp-ipc)

add_subdirectory(src)

add_executable(logger_test test/test.cpp)
target_link_libraries(logger_test PRIVATE ipc)
