CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)
project(lox)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER /usr/bin/clang-10)
set(CMAKE_CXX_COMPILER /usr/bin/clang++-10)
add_compile_options(
  "-Wall" "-Wpedantic" "-Wextra" "-fexceptions"
  "-g" "-O0")

## project headers and subdirectories
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(lib)

add_executable(${CMAKE_PROJECT_NAME} main.cc lib)
add_executable(ast_generator ${PROJECT_SOURCE_DIR}/lib/ASTGenerator.cc)

target_link_libraries(${CMAKE_PROJECT_NAME}
                      loxlib)
