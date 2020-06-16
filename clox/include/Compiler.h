#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "Value.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Chunk Chunk;
typedef struct Parser Parser;

bool compile(const char *src, Chunk *chunk);
void emit_byte(Parser *parser, uint8_t byte);
void emit_return(Parser *parser);
void emit_constant(Parser *parser, Value value);
uint8_t make_constant(Parser *parser, Value value);
void end_compiler(Parser *parser);

#endif
