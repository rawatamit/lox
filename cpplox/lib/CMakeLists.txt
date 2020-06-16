cmake_minimum_required(VERSION 3.7)
 
include_directories(${PROJECT_SOURCE_DIR}/cpplox/include)

add_library(loxlib
            Token.cc
            Scanner.cc
            Parser.cc
            ErrorHandler.cc
            Environment.cc
            Resolver.cc
            Interpreter.cc
            LoxClass.cc
            )
