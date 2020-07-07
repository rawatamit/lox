CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)
project(lox)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER /usr/bin/clang-10)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-10)

add_compile_options(
  #"-DDEBUG_TRACE_EXECUTION"
  #"-DDEBUG_DISASSEMBLE"
  #"-DDEBUG_PRINT_CODE"
  "-Wall" "-Wpedantic" "-Wextra" "-fexceptions"
  "-g" "-O0")

add_subdirectory(cpplox)
add_subdirectory(clox)
