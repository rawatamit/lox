cmake_minimum_required(VERSION 3.7)
 
include_directories(${PROJECT_SOURCE_DIR}/clox/include)

add_library(vmlib
            Chunk.c
            Memory.c
            Debug.c
            Value.c
            Object.c
            VM.c
            Scanner.c
            Parser.c
            Compiler.c)
