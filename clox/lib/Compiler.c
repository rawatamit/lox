#include "Compiler.h"
#include "Memory.h"
#include "Object.h"
#include "Opcode.h"
#include "Parser.h"
#include "Scanner.h"
#include "VM.h"
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG_PRINT_CODE
#include "Debug.h"
#endif

static Chunk *current_chunk(Compiler *compiler) { return &compiler->fn->chunk; }

void init_compiler(Compiler *compiler, Compiler *enclosing, Parser *parser,
                   VM *vm, FunctionType type) {
  compiler->enclosing = enclosing;
  compiler->current_class =
      (enclosing == NULL) ? NULL : enclosing->current_class;
  compiler->local_count = 0;
  compiler->scope_depth = 0;
  compiler->parser = parser;
  compiler->vm = vm;
  compiler->fn = new_function(vm);
  compiler->fn_type = type;
  // for GC
  compiler->vm->compiler = compiler;

  if (type != TYPE_SCRIPT) {
    compiler->fn->name =
        copy_string(vm, parser->previous.start, parser->previous.length);
  }

  Local *local = &compiler->locals[compiler->local_count++];
  local->depth = 0;
  if (type != TYPE_FUNCTION) {
    local->name.start = "this";
    local->name.length = 4;
  } else {
    local->name.start = "";
    local->name.length = 0;
  }
  local->is_captured = false;
}

ObjFunction *compile(VM *vm, const char *src) {
  Scanner scanner;
  init_scanner(&scanner, src);

  Parser parser;
  init_parser(&parser, &scanner);

  Compiler compiler;
  init_compiler(&compiler, NULL, &parser, vm, TYPE_SCRIPT);
  compiler.fn->name = copy_string(vm, "top-level", 9);

  advance(&compiler);

  while (!match(&compiler, TOKEN_EOF)) {
    declaration(&compiler);
  }

  ObjFunction *fn = end_compiler(&compiler);
  return parser.had_error ? NULL : fn;
}

void mark_compiler_roots(Compiler *compiler) {
  while (compiler != NULL) {
    mark_object(compiler->vm, (Obj *)compiler->fn);
    compiler = compiler->enclosing;
  }
}

void define_variable(Compiler *compiler, uint8_t global) {
  if (compiler->scope_depth > 0) {
    mark_initialized(compiler);
    return;
  }

  emit_bytes(compiler, OP_DEFINE_GLOBAL, global);
}

void declare_variable(Compiler *compiler) {
  if (compiler->scope_depth == 0)
    return;

  Token *name = &compiler->parser->previous;

  for (int i = compiler->local_count - 1; i >= 0; --i) {
    Local *local = &compiler->locals[i];

    if (local->depth != -1 && local->depth < compiler->scope_depth) {
      break;
    }

    if (identifiers_equal(name, &local->name)) {
      error(compiler,
            "Variable with this name already declared in this scope.");
    }
  }

  add_local(compiler, *name);
}

void mark_initialized(Compiler *compiler) {
  if (compiler->scope_depth == 0)
    return;

  compiler->locals[compiler->local_count - 1].depth = compiler->scope_depth;
}

bool identifiers_equal(Token *a, Token *b) {
  return a->length == b->length && memcmp(a->start, b->start, a->length) == 0;
}

void add_local(Compiler *compiler, Token name) {
  if (compiler->local_count == UINT8_COUNT) {
    error(compiler, "Too many local variables in function.");
  } else {
    Local *local = &compiler->locals[compiler->local_count++];
    local->name = name;
    local->depth = -1;
    local->is_captured = false;
  }
}

void begin_scope(Compiler *compiler) { compiler->scope_depth += 1; }

void end_scope(Compiler *compiler) {
  compiler->scope_depth -= 1;

  while (compiler->local_count > 0 &&
         compiler->locals[compiler->local_count - 1].depth >
             compiler->scope_depth) {
    if (compiler->locals[compiler->local_count - 1].is_captured) {
      emit_byte(compiler, OP_CLOSE_UPVALUE);
    } else {
      emit_byte(compiler, OP_POP);
    }
    --compiler->local_count;
  }
}

void emit_loop(Compiler *compiler, int loop_start) {
  emit_byte(compiler, OP_LOOP);
  int offset = current_chunk(compiler)->size - loop_start + 2;
  if (offset > UINT16_MAX)
    error(compiler, "Loop body too large.");

  emit_byte(compiler, (offset >> 8) & 0xff);
  emit_byte(compiler, offset & 0xff);
}

void emit_byte(Compiler *compiler, uint8_t byte) {
  write_chunk(compiler->vm, current_chunk(compiler), byte,
              compiler->parser->previous.line);
}

void emit_bytes(Compiler *compiler, uint8_t byte1, uint8_t byte2) {
  emit_byte(compiler, byte1);
  emit_byte(compiler, byte2);
}

void emit_constant(Compiler *compiler, Value value) {
  emit_bytes(compiler, OP_CONSTANT, make_constant(compiler, value));
}

void patch_jump(Compiler *compiler, int offset) {
  // -2 for bytecode for jump offset
  int jmp = current_chunk(compiler)->size - offset - 2;

  if (jmp > UINT8_MAX) {
    error(compiler, "Too much code to jump over.");
  }

  current_chunk(compiler)->code[offset] = (jmp >> 8) & 0xff;
  current_chunk(compiler)->code[offset + 1] = jmp & 0xff;
}

int emit_jump(Compiler *compiler, uint8_t inst) {
  emit_byte(compiler, inst);
  emit_byte(compiler, 0xff);
  emit_byte(compiler, 0xff);
  return current_chunk(compiler)->size - 2;
}

void emit_return(Compiler *compiler) {
  if (compiler->fn_type == TYPE_INITIALIZER) {
    emit_bytes(compiler, OP_GET_LOCAL, 0);
    emit_byte(compiler, OP_RETURN);
  } else {
    emit_bytes(compiler, OP_NIL, OP_RETURN);
  }
}

uint8_t make_constant(Compiler *compiler, Value value) {
  size_t index = add_constant(compiler->vm, current_chunk(compiler), value);

  if (index > UINT8_MAX) {
    error(compiler, "Too many constants in one chunk.");
    return 0;
  }

  return (uint8_t)index;
}

ObjFunction *end_compiler(Compiler *compiler) {
  emit_return(compiler);
  ObjFunction *fn = compiler->fn;

#ifdef DEBUG_PRINT_CODE
  if (!compiler->parser->had_error) {
    disassemble_chunk(current_chunk(compiler), fn->name->chars, stderr);
  }
#endif

  return fn;
}

void declaration(Compiler *compiler) {
  switch (compiler->parser->current.type) {
  case TOKEN_VAR:
    match(compiler, TOKEN_VAR);
    var_declaration(compiler);
    break;

  case TOKEN_FUN:
    match(compiler, TOKEN_FUN);
    fun_declaration(compiler);
    break;

  case TOKEN_CLASS:
    match(compiler, TOKEN_CLASS);
    class_declaration(compiler);
    break;

  default:
    statement(compiler);
    break;
  }

  if (compiler->parser->panic_mode) {
    synchronize(compiler);
  }
}

void class_declaration(Compiler *compiler) {
  consume(compiler, TOKEN_IDENTIFIER, "Expected class name.");
  Token name = compiler->parser->previous;
  uint8_t name_constant = identifier_constant(compiler, &name);
  declare_variable(compiler);

  emit_bytes(compiler, OP_CLASS, name_constant);
  define_variable(compiler, name_constant);

  ClassCompiler class_compiler;
  class_compiler.name = compiler->parser->previous;
  class_compiler.enclosing = compiler->current_class;
  class_compiler.has_superclass = false;
  compiler->current_class = &class_compiler;

  if (match(compiler, TOKEN_LESS)) {
    consume(compiler, TOKEN_IDENTIFIER, "Expected superclass name.");
    variable(compiler, false);

    if (identifiers_equal(&name, &compiler->parser->previous)) {
      error(compiler, "A class cannot inherit from itself.");
    }

    class_compiler.has_superclass = true;

    begin_scope(compiler);
    add_local(compiler, synthetic_token("super"));
    define_variable(compiler, 0);

    named_variable(compiler, name, false);
    emit_byte(compiler, OP_INHERIT);
  }

  named_variable(compiler, name, false);
  consume(compiler, TOKEN_LEFT_BRACE, "Expected '{' before class body.");
  while (!check(compiler, TOKEN_RIGHT_BRACE) && !check(compiler, TOKEN_EOF)) {
    method(compiler);
  }

  consume(compiler, TOKEN_RIGHT_BRACE, "Expected '}' after class body.");
  emit_byte(compiler, OP_POP);

  if (class_compiler.has_superclass) {
    end_scope(compiler);
  }

  compiler->current_class = compiler->current_class->enclosing;
}

void method(Compiler *compiler) {
  consume(compiler, TOKEN_IDENTIFIER, "Expected method name.");
  uint8_t constant = identifier_constant(compiler, &compiler->parser->previous);
  FunctionType type = TYPE_METHOD;

  if (compiler->parser->previous.length == 4 &&
      memcmp(compiler->parser->previous.start, "init", 4) == 0) {
    type = TYPE_INITIALIZER;
  }

  function(compiler, type);
  emit_bytes(compiler, OP_METHOD, constant);
}

void fun_declaration(Compiler *compiler) {
  uint8_t global = parse_variable(compiler, "Expected function name.");
  mark_initialized(compiler);
  function(compiler, TYPE_FUNCTION);
  define_variable(compiler, global);
}

void function(Compiler *compiler, FunctionType type) {
  Compiler ncompiler;
  init_compiler(&ncompiler, compiler, compiler->parser, compiler->vm, type);
  compiler = &ncompiler;

  // a function starts a new scope
  begin_scope(compiler);

  // parameter list
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after function name.");
  if (!check(compiler, TOKEN_RIGHT_PAREN)) {
    do {
      ++compiler->fn->arity;
      if (compiler->fn->arity > 255) {
        error_at_current(compiler, "Cannot have more than 255 parameters.");
      }

      uint8_t param = parse_variable(compiler, "Expected parameter name.");
      define_variable(compiler, param);
    } while (match(compiler, TOKEN_COMMA));
  }
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");

  // body
  consume(compiler, TOKEN_LEFT_BRACE, "Expected '{' before function body.");
  block(compiler);

  // get function created by compiler
  ObjFunction *fn = end_compiler(compiler);

  // move back to the upper function
  // as we may need to write upvalues
  compiler = compiler->enclosing;
  // for GC, set compiler as enclosing compiler
  compiler->vm->compiler = compiler;

  emit_bytes(compiler, OP_CLOSURE,
             make_constant(compiler, object_val((Obj *)fn)));

  for (int i = 0; i < fn->upvalue_count; ++i) {
    // NOTE: compiler is the enclosing function for this function
    // and we stored the upvalues inside this compiler
    // write them out
    emit_byte(compiler, ncompiler.upvalues[i].is_local ? 1 : 0);
    emit_byte(compiler, ncompiler.upvalues[i].index);
  }
}

// 'var' ID ('=' expr)? ';'
void var_declaration(Compiler *compiler) {
  uint8_t global = parse_variable(compiler, "Expected variable name.");

  if (match(compiler, TOKEN_EQUAL)) {
    expression(compiler);
  } else {
    emit_byte(compiler, OP_NIL);
  }

  consume(compiler, TOKEN_SEMICOLON,
          "Expected ';' after variable declaration.");
  define_variable(compiler, global);
}

void statement(Compiler *compiler) {
  switch (compiler->parser->current.type) {
  case TOKEN_PRINT:
    match(compiler, TOKEN_PRINT);
    print_statement(compiler);
    break;

  case TOKEN_LEFT_BRACE:
    match(compiler, TOKEN_LEFT_BRACE);
    begin_scope(compiler);
    block(compiler);
    end_scope(compiler);
    break;

  case TOKEN_IF:
    match(compiler, TOKEN_IF);
    if_statement(compiler);
    break;

  case TOKEN_WHILE:
    match(compiler, TOKEN_WHILE);
    while_statement(compiler);
    break;

  case TOKEN_FOR:
    match(compiler, TOKEN_FOR);
    for_statement(compiler);
    break;

  case TOKEN_RETURN:
    match(compiler, TOKEN_RETURN);
    return_statement(compiler);
    break;

  default:
    expression_statement(compiler);
    break;
  }
}

void if_statement(Compiler *compiler) {
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after 'if'.");
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after condition.");

  int then_jmp = emit_jump(compiler, OP_JUMP_IF_FALSE);
  emit_byte(compiler, OP_POP);
  statement(compiler);
  int else_jmp = emit_jump(compiler, OP_JUMP);
  patch_jump(compiler, then_jmp);
  emit_byte(compiler, OP_POP);

  if (match(compiler, TOKEN_ELSE)) {
    statement(compiler);
  }

  patch_jump(compiler, else_jmp);
}

void while_statement(Compiler *compiler) {
  int loop_start = current_chunk(compiler)->size;
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after 'while'.");
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after condition.");

  int exit_jump = emit_jump(compiler, OP_JUMP_IF_FALSE);
  emit_byte(compiler, OP_POP);
  statement(compiler);
  emit_loop(compiler, loop_start);
  patch_jump(compiler, exit_jump);
  emit_byte(compiler, OP_POP);
}

void for_statement(Compiler *compiler) {
  begin_scope(compiler);
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after 'for'.");

  if (match(compiler, TOKEN_SEMICOLON)) {
    // nothing to do
  } else if (match(compiler, TOKEN_VAR)) {
    var_declaration(compiler);
  } else {
    expression_statement(compiler);
  }

  int loop_start = current_chunk(compiler)->size;

  int exit_jump = -1;
  if (!match(compiler, TOKEN_SEMICOLON)) {
    expression(compiler);
    consume(compiler, TOKEN_SEMICOLON, "Expected ';' after loop condition.");

    // jump out of loop if false
    exit_jump = emit_jump(compiler, OP_JUMP_IF_FALSE);
    emit_byte(compiler, OP_POP);
  }

  if (!match(compiler, TOKEN_RIGHT_PAREN)) {
    int body_jump = emit_jump(compiler, OP_JUMP);
    int increment_start = current_chunk(compiler)->size;
    expression(compiler);
    emit_byte(compiler, OP_POP);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after for clauses.");

    emit_loop(compiler, loop_start);
    loop_start = increment_start;
    patch_jump(compiler, body_jump);
  }

  statement(compiler);

  emit_loop(compiler, loop_start);
  if (exit_jump != -1) {
    patch_jump(compiler, exit_jump);
    emit_byte(compiler, OP_POP);
  }

  end_scope(compiler);
}

void expression_statement(Compiler *compiler) {
  expression(compiler);
  consume(compiler, TOKEN_SEMICOLON, "Expected ';' after expression.");
  emit_byte(compiler, OP_POP);
}

void print_statement(Compiler *compiler) {
  expression(compiler);
  consume(compiler, TOKEN_SEMICOLON, "Expected ';' after value.");
  emit_byte(compiler, OP_PRINT);
}

void return_statement(Compiler *compiler) {
  if (compiler->fn_type == TYPE_SCRIPT) {
    error(compiler, "Cannot return from top-level code.");
  }

  if (match(compiler, TOKEN_SEMICOLON)) {
    emit_return(compiler);
  } else {
    if (compiler->fn_type == TYPE_INITIALIZER) {
      error(compiler, "Cannot return a value from an initialiser.");
    }

    expression(compiler);
    consume(compiler, TOKEN_SEMICOLON, "Expected ';' after return value.");
    emit_byte(compiler, OP_RETURN);
  }
}

void block(Compiler *compiler) {
  while (!check(compiler, TOKEN_RIGHT_BRACE) && !check(compiler, TOKEN_EOF)) {
    declaration(compiler);
  }

  consume(compiler, TOKEN_RIGHT_BRACE, "Expected '}' after block.");
}

void expression(Compiler *compiler) {
  parse_precedence(compiler, PREC_ASSIGNMENT);
}

void grouping(Compiler *compiler, bool can_assign) {
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

void logical_and(Compiler *compiler, bool can_assign) {
  int end_jump = emit_jump(compiler, OP_JUMP_IF_FALSE);
  emit_byte(compiler, OP_POP);
  parse_precedence(compiler, PREC_AND);
  patch_jump(compiler, end_jump);
}

void logical_or(Compiler *compiler, bool can_assign) {
  int else_jump = emit_jump(compiler, OP_JUMP_IF_FALSE);
  int end_jump = emit_jump(compiler, OP_JUMP);

  patch_jump(compiler, else_jump);
  emit_byte(compiler, OP_POP);

  parse_precedence(compiler, PREC_OR);
  patch_jump(compiler, end_jump);
}

void unary(Compiler *compiler, bool can_assign) {
  TokenType op = compiler->parser->previous.type;

  // operand
  parse_precedence(compiler, PREC_UNARY);

  switch (op) {
  case TOKEN_MINUS:
    emit_byte(compiler, OP_NEGATE);
    break;
  case TOKEN_BANG:
    emit_byte(compiler, OP_NOT);
    break;
  default:
    break;
  }
}

void binary(Compiler *compiler, bool can_assign) {
  TokenType op = compiler->parser->previous.type;

  // compile right operand
  ParseRule *rule = get_rule(op);
  parse_precedence(compiler, (Precedence)(rule->precedence + 1));

  // emit op instruction
  switch (op) {
  case TOKEN_PLUS:
    emit_byte(compiler, OP_ADD);
    break;
  case TOKEN_MINUS:
    emit_byte(compiler, OP_SUBTRACT);
    break;
  case TOKEN_SLASH:
    emit_byte(compiler, OP_DIVIDE);
    break;
  case TOKEN_STAR:
    emit_byte(compiler, OP_MULTIPLY);
    break;
  case TOKEN_BANG_EQUAL:
    emit_bytes(compiler, OP_EQUAL, OP_NOT);
    break;
  case TOKEN_EQUAL_EQUAL:
    emit_byte(compiler, OP_EQUAL);
    break;
  case TOKEN_GREATER:
    emit_byte(compiler, OP_GREATER);
    break;
  case TOKEN_GREATER_EQUAL:
    emit_bytes(compiler, OP_LESS, OP_NOT);
    break;
  case TOKEN_LESS:
    emit_byte(compiler, OP_LESS);
    break;
  case TOKEN_LESS_EQUAL:
    emit_bytes(compiler, OP_GREATER, OP_NOT);
    break;
  default:
    break;
  }
}

void this_(Compiler *compiler, bool can_assign) {
  if (compiler->current_class != NULL) {
    variable(compiler, false);
  } else {
    error(compiler, "Cannot use 'this' outside of a class.");
  }
}

void super_(Compiler *compiler, bool can_assign) {
  if (compiler->current_class == NULL) {
    error(compiler, "Cannot use 'super' outside of a class.");
  } else if (!compiler->current_class->has_superclass) {
    error(compiler, "Cannot use 'super' in a class without a superclass.");
  }

  consume(compiler, TOKEN_DOT, "Expected '.' after 'super'.");
  consume(compiler, TOKEN_IDENTIFIER, "Expected superclass method name.");
  uint8_t name = identifier_constant(compiler, &compiler->parser->previous);
  named_variable(compiler, synthetic_token("this"), false);

  if (match(compiler, TOKEN_LEFT_PAREN)) {
    uint8_t arg_count = argument_list(compiler);
    named_variable(compiler, synthetic_token("super"), false);
    emit_bytes(compiler, OP_SUPER_INVOKE, name);
    emit_byte(compiler, arg_count);
  } else {
    named_variable(compiler, synthetic_token("super"), false);
    emit_bytes(compiler, OP_GET_SUPER, name);
  }
}

void call(Compiler *compiler, bool can_assign) {
  uint8_t arg_count = argument_list(compiler);
  emit_bytes(compiler, OP_CALL, arg_count);
}

void dot(Compiler *compiler, bool can_assign) {
  consume(compiler, TOKEN_IDENTIFIER, "Expected property name after '.'.");
  uint8_t name = identifier_constant(compiler, &compiler->parser->previous);

  if (can_assign && match(compiler, TOKEN_EQUAL)) {
    expression(compiler);
    emit_bytes(compiler, OP_SET_PROPERTY, name);
  } else if (match(compiler, TOKEN_LEFT_PAREN)) {
    uint8_t arg_count = argument_list(compiler);
    emit_bytes(compiler, OP_INVOKE, name);
    emit_byte(compiler, arg_count);
  } else {
    emit_bytes(compiler, OP_GET_PROPERTY, name);
  }
}

uint8_t argument_list(Compiler *compiler) {
  uint8_t arg_count = 0;

  if (!check(compiler, TOKEN_RIGHT_PAREN)) {
    do {
      expression(compiler);

      if (arg_count == 255) {
        error(compiler, "Cannot have more than 255 arguments.");
      }

      ++arg_count;
    } while (match(compiler, TOKEN_COMMA));
  }

  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");
  return arg_count;
}

void number(Compiler *compiler, bool can_assign) {
  double value = strtod(compiler->parser->previous.start, NULL);
  emit_constant(compiler, number_val(value));
}

void string(Compiler *compiler, bool can_assign) {
  emit_constant(compiler,
                object_val((Obj *)copy_string(
                    compiler->vm, compiler->parser->previous.start + 1,
                    compiler->parser->previous.length - 2)));
}

void literal(Compiler *compiler, bool can_assign) {
  switch (compiler->parser->previous.type) {
  case TOKEN_NIL:
    emit_byte(compiler, OP_NIL);
    break;
  case TOKEN_TRUE:
    emit_byte(compiler, OP_TRUE);
    break;
  case TOKEN_FALSE:
    emit_byte(compiler, OP_FALSE);
    break;
  default:
    break;
  }
}

void variable(Compiler *compiler, bool can_assign) {
  named_variable(compiler, compiler->parser->previous, can_assign);
}

void named_variable(Compiler *compiler, Token name, bool can_assign) {
  Opcode get_op, set_op;
  int arg = resolve_local(compiler, &name);

  if (arg != -1) {
    get_op = OP_GET_LOCAL;
    set_op = OP_SET_LOCAL;
  } else if ((arg = resolve_upvalue(compiler, &name)) != -1) {
    get_op = OP_GET_UPVALUE;
    set_op = OP_SET_UPVALUE;
  } else {
    arg = identifier_constant(compiler, &name);
    get_op = OP_GET_GLOBAL;
    set_op = OP_SET_GLOBAL;
  }

  if (can_assign && match(compiler, TOKEN_EQUAL)) {
    expression(compiler);
    emit_bytes(compiler, set_op, (uint8_t)arg);
  } else {
    emit_bytes(compiler, get_op, (uint8_t)arg);
  }
}

int resolve_local(Compiler *compiler, Token *name) {
  for (int i = compiler->local_count - 1; i >= 0; --i) {
    Local *local = &compiler->locals[i];
    if (identifiers_equal(&local->name, name)) {
      if (local->depth == -1) {
        error(compiler, "Cannot read local variable in its own initialiser.");
      } else {
        return i;
      }
    }
  }

  return -1;
}

int resolve_upvalue(Compiler *compiler, Token *name) {
  if (compiler->enclosing == NULL)
    return -1;

  int local = resolve_local(compiler->enclosing, name);
  if (local != -1) {
    compiler->enclosing->locals[local].is_captured = true;
    return add_upvalue(compiler, (uint8_t)local, true);
  }

  int upvalue = resolve_upvalue(compiler->enclosing, name);
  if (upvalue != -1) {
    return add_upvalue(compiler, (uint8_t)upvalue, false);
  }

  return -1;
}

int add_upvalue(Compiler *compiler, uint8_t index, bool is_local) {
  int upvalue_count = compiler->fn->upvalue_count;

  for (int i = 0; i < upvalue_count; ++i) {
    Upvalue *upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->is_local == is_local) {
      return i;
    }
  }

  if (upvalue_count == UINT8_COUNT) {
    error(compiler, "Too many closure variables in function.");
    return 0;
  }

  compiler->upvalues[upvalue_count].is_local = is_local;
  compiler->upvalues[upvalue_count].index = index;
  return compiler->fn->upvalue_count++;
}

uint8_t parse_variable(Compiler *compiler, const char *msg) {
  consume(compiler, TOKEN_IDENTIFIER, msg);
  declare_variable(compiler);
  if (compiler->scope_depth > 0)
    return 0;
  return identifier_constant(compiler, &compiler->parser->previous);
}

uint8_t identifier_constant(Compiler *compiler, Token *name) {
  return make_constant(compiler, object_val((Obj *)copy_string(
                                     compiler->vm, name->start, name->length)));
}

Token synthetic_token(const char *text) {
  Token token = {.start = text, .length = (int)strlen(text), .line = 0};
  return token;
}
