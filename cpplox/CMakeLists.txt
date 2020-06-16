CMAKE_MINIMUM_REQUIRED(VERSION 3.7 FATAL_ERROR)

include_directories(${PROJECT_SOURCE_DIR}/cpplox/include)

add_subdirectory(lib)

add_executable(ast_generator ${PROJECT_SOURCE_DIR}/cpplox/lib/ASTGenerator.cc)
add_executable(cpplox main.cc)

target_link_libraries(cpplox loxlib)
