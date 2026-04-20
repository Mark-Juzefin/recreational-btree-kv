cmake_minimum_required(VERSION 3.20)
project(step05_file_io C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(bytefile_test
    bytefile_test.c
    bytefile.c
)
