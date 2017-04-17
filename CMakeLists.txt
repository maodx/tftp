cmake_minimum_required(VERSION 3.6)
project(tftp2)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(tftp2 ${SOURCE_FILES})