#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "Scanner.h"
#include "Value.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Chunk Chunk;
typedef struct Parser Parser;
typedef struct ObjFunction ObjFunction;
typedef struct VM VM;

#define UINT8_COUNT (UINT8_MAX + 1)

struct Local {
  Token name;
  int depth;
  bool is_captured;
};

typedef struct Local Local;

enum FunctionType {
  TYPE_FUNCTION,
  TYPE_SCRIPT,
};

typedef enum FunctionType FunctionType;

struct Upvalue {
  bool is_local;
  uint8_t index;
};

typedef struct Upvalue Upvalue;

struct Compiler {
  struct Compiler *enclosing;
  Local locals[UINT8_COUNT];
  Upvalue upvalues[UINT8_COUNT];
  int local_count;
  int scope_depth;
  ObjFunction *fn;
  FunctionType fn_type;
  Parser *parser;
  VM *vm;
};

typedef struct Compiler Compiler;

void init_compiler(Compiler *compiler, Compiler *enclosing, Parser *parser,
                   VM *vm, FunctionType type);
ObjFunction *compile(VM *vm, const char *src);
void define_variable(Compiler *compiler, uint8_t global);
void declare_variable(Compiler *compiler);
void mark_initialized(Compiler *compiler);
void add_local(Compiler *compiler, Token name);
bool identifiers_equal(Token *a, Token *b);
int resolve_local(Compiler *compiler, Token *name);
int resolve_upvalue(Compiler *compiler, Token *name);
int add_upvalue(Compiler *compiler, uint8_t index, bool is_local);
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
ObjFunction *end_compiler(Compiler *compiler);

#endif
