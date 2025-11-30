cmake_minimum_required(VERSION 3.22)
project(logger C)

set(CMAKE_C_STANDARD 99)

add_executable(logger main.c)
target_compile_options(logger PRIVATE -Wall -Wextra -Wpedantic -Werror)
