cmake_minimum_required(VERSION 3.7)
 
include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(loxlib
            Token.cc
            Scanner.cc
            Parser.cc
            ErrorHandler.cc
            Interpreter.cc
            Environment.cc
            )
