  cmake_minimum_required(VERSION 3.20)
  project(intvec C)
  set(CMAKE_C_STANDARD 11)
  add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -g -O0)

  add_executable(intvec_debug intvec.c)