CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)

include_directories(${PROJECT_SOURCE_DIR}/clox/include)

add_subdirectory(lib)

add_executable(clox main.c)

target_link_libraries(clox vmlib)
