#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "Scanner.h"
#include "Value.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Chunk Chunk;
typedef struct Parser Parser;
typedef struct VM VM;

#define UINT8_COUNT (UINT8_MAX + 1)

struct Local {
  Token name;
  int depth;
};

typedef struct Local Local;

struct Compiler {
  Local locals[UINT8_COUNT];
  int local_count;
  int scope_depth;
  Parser *parser;
  Chunk *chunk;
  VM *vm;
};

typedef struct Compiler Compiler;

void init_compiler(Compiler *compiler);
bool compile(VM *vm, const char *src, Chunk *chunk);
void define_variable(Compiler *compiler, uint8_t global);
void declare_variable(Compiler *compiler);
void mark_initialized(Compiler *compiler);
void add_local(Compiler *compiler, Token name);
bool identifiers_equal(Token *a, Token *b);
int resolve_local(Compiler *compiler, Token *name);
void named_variable(Compiler *compiler, Token name, bool can_assign);
void begin_scope(Compiler *compiler);
void end_scope(Compiler *compiler);
void emit_byte(Compiler *compiler, uint8_t byte);
void emit_bytes(Compiler *compiler, uint8_t byte1, uint8_t byte2);
void emit_loop(Compiler *compiler, int loop_start);
void emit_return(Compiler *compiler);
void emit_constant(Compiler *compiler, Value value);
int emit_jump(Compiler *compiler, uint8_t inst);
void patch_jump(Compiler *compiler, int offset);
uint8_t make_constant(Compiler *compiler, Value value);
void end_compiler(Compiler *compiler);

#endif
