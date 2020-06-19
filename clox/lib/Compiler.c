#include "Compiler.h"
#include "Opcode.h"
#include "Parser.h"
#include "Scanner.h"
#include "Object.h"
#include "VM.h"
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG_PRINT_CODE
#include "Debug.h"
#endif

static Chunk *current_chunk(Compiler *compiler) { return compiler->chunk; }

void init_compiler(Compiler *compiler)
{
  compiler->local_count = 0;
  compiler->scope_depth = 0;
}

bool compile(VM *vm, const char *src, Chunk *chunk)
{
  Scanner scanner;
  init_scanner(&scanner, src);

  Parser parser;
  init_parser(&parser, &scanner);

  Compiler compiler;
  init_compiler(&compiler);
  compiler.parser = &parser;
  compiler.chunk = chunk;
  compiler.vm = vm;

  advance(&compiler);

  while (!match(&compiler, TOKEN_EOF))
  {
    declaration(&compiler);
  }

  end_compiler(&compiler);
  return !parser.had_error;
}

void define_variable(Compiler *compiler, uint8_t global)
{
  if (compiler->scope_depth > 0)
  {
    mark_initialized(compiler);
    return;
  }

  emit_bytes(compiler, OP_DEFINE_GLOBAL, global);
}

void declare_variable(Compiler *compiler)
{
  if (compiler->scope_depth == 0)
    return;

  Token *name = &compiler->parser->previous;

  for (int i = compiler->local_count - 1; i >= 0; --i)
  {
    Local *local = &compiler->locals[i];

    if (local->depth != -1 && local->depth < compiler->scope_depth)
    {
      break;
    }

    if (identifiers_equal(name, &local->name))
    {
      error(compiler, "Variable with this name already declared in this scope.");
    }
  }

  add_local(compiler, *name);
}

void mark_initialized(Compiler *compiler)
{
  compiler->locals[compiler->local_count - 1].depth =
      compiler->scope_depth;
}

bool identifiers_equal(Token *a, Token *b)
{
  return a->length == b->length && memcmp(a->start, b->start, a->length) == 0;
}

void add_local(Compiler *compiler, Token name)
{
  if (compiler->local_count == UINT8_COUNT)
  {
    error(compiler, "Too many local variables in function.");
  }
  else
  {
    Local *local = &compiler->locals[compiler->local_count++];
    local->name = name;
    local->depth = -1;
  }
}

void begin_scope(Compiler *compiler)
{
  compiler->scope_depth += 1;
}

void end_scope(Compiler *compiler)
{
  compiler->scope_depth -= 1;

  while (compiler->local_count > 0 &&
         compiler->locals[compiler->local_count - 1].depth >
             compiler->scope_depth)
  {
    emit_byte(compiler, OP_POP);
    --compiler->local_count;
  }
}

void emit_byte(Compiler *compiler, uint8_t byte)
{
  write_chunk(current_chunk(compiler), byte, compiler->parser->previous.line);
}

void emit_bytes(Compiler *compiler, uint8_t byte1, uint8_t byte2)
{
  emit_byte(compiler, byte1);
  emit_byte(compiler, byte2);
}

void emit_constant(Compiler *compiler, Value value)
{
  emit_bytes(compiler, OP_CONSTANT, make_constant(compiler, value));
}

void patch_jump(Compiler *compiler, int offset)
{
  // -2 for bytecode for jump offset
  int jmp = current_chunk(compiler)->size - offset - 2;

  if (jmp > UINT8_MAX)
  {
    error(compiler, "Too much code to jump over.");
  }

  current_chunk(compiler)->code[offset] = (jmp >> 8) & 0xff;
  current_chunk(compiler)->code[offset + 1] = jmp & 0xff;
}

int emit_jump(Compiler *compiler, uint8_t inst)
{
  emit_byte(compiler, inst);
  emit_byte(compiler, 0xff);
  emit_byte(compiler, 0xff);
  return current_chunk(compiler)->size - 2;
}

void emit_return(Compiler *compiler) { emit_byte(compiler, OP_RETURN); }

uint8_t make_constant(Compiler *compiler, Value value)
{
  size_t index = add_constant(current_chunk(compiler), value);

  if (index > UINT8_MAX)
  {
    error(compiler, "Too many constants in one chunk.");
    return 0;
  }

  return (uint8_t)index;
}

void end_compiler(Compiler *compiler)
{
  emit_return(compiler);

#ifdef DEBUG_PRINT_CODE
  if (!compiler->parser->had_error)
  {
    disassemble_chunk(current_chunk(), "code");
  }
#endif
}

void declaration(Compiler *compiler)
{
  switch (compiler->parser->current.type)
  {
  case TOKEN_VAR:
    match(compiler, TOKEN_VAR);
    var_declaration(compiler);
    break;

  default:
    statement(compiler);
    break;
  }

  if (compiler->parser->panic_mode)
  {
    synchronize(compiler);
  }
}

// 'var' ID ('=' expr)? ';'
void var_declaration(Compiler *compiler)
{
  uint8_t global = parse_variable(compiler, "Expected variable name.");

  if (match(compiler, TOKEN_EQUAL))
  {
    expression(compiler);
  }
  else
  {
    emit_byte(compiler, OP_NIL);
  }

  consume(compiler, TOKEN_SEMICOLON, "Expected ';' after variable declaration.");
  define_variable(compiler, global);
}

void statement(Compiler *compiler)
{
  switch (compiler->parser->current.type)
  {
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

  default:
    expression_statement(compiler);
    break;
  }
}

void if_statement(Compiler *compiler)
{
  consume(compiler, TOKEN_LEFT_PAREN, "Expected '(' after 'if'.");
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after condition.");

  int then_jmp = emit_jump(compiler, OP_JUMP_IF_FALSE);
  emit_byte(compiler, OP_POP);
  statement(compiler);
  int else_jmp = emit_jump(compiler, OP_JUMP);
  patch_jump(compiler, then_jmp);
  emit_byte(compiler, OP_POP);

  if (match(compiler, TOKEN_ELSE))
  {
    statement(compiler);
  }

  patch_jump(compiler, else_jmp);
}

void expression_statement(Compiler *compiler)
{
  expression(compiler);
  consume(compiler, TOKEN_SEMICOLON, "Expected ';' after expression.");
  emit_byte(compiler, OP_POP);
}

void print_statement(Compiler *compiler)
{
  expression(compiler);
  consume(compiler, TOKEN_SEMICOLON, "Expected ';' after value.");
  emit_byte(compiler, OP_PRINT);
}

void block(Compiler *compiler)
{
  while (!check(compiler, TOKEN_RIGHT_BRACE) && !check(compiler, TOKEN_EOF))
  {
    declaration(compiler);
  }

  consume(compiler, TOKEN_RIGHT_BRACE, "Expected '}' after block.");
}

void expression(Compiler *compiler) { parse_precedence(compiler, PREC_ASSIGNMENT); }

void grouping(Compiler *compiler, bool can_assign)
{
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

void unary(Compiler *compiler, bool can_assign)
{
  TokenType op = compiler->parser->previous.type;

  // operand
  parse_precedence(compiler, PREC_UNARY);

  switch (op)
  {
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

void binary(Compiler *compiler, bool can_assign)
{
  TokenType op = compiler->parser->previous.type;

  // compile right operand
  ParseRule *rule = get_rule(op);
  parse_precedence(compiler, (Precedence)(rule->precedence + 1));

  // emit op instruction
  switch (op)
  {
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

void number(Compiler *compiler, bool can_assign)
{
  double value = strtod(compiler->parser->previous.start, NULL);
  emit_constant(compiler, number_val(value));
}

void string(Compiler *compiler, bool can_assign)
{
  emit_constant(compiler,
                object_val(
                    (Obj *)copy_string(
                        compiler->vm,
                        compiler->parser->previous.start + 1,
                        compiler->parser->previous.length - 2)));
}

void literal(Compiler *compiler, bool can_assign)
{
  switch (compiler->parser->previous.type)
  {
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

void variable(Compiler *compiler, bool can_assign)
{
  named_variable(compiler, compiler->parser->previous, can_assign);
}

void named_variable(Compiler *compiler, Token name, bool can_assign)
{
  Opcode get_op, set_op;
  int arg = resolve_local(compiler, &name);

  if (arg != -1)
  {
    get_op = OP_GET_LOCAL;
    set_op = OP_SET_LOCAL;
  }
  else
  {
    arg = identifier_constant(compiler, &name);
    get_op = OP_GET_GLOBAL;
    set_op = OP_SET_GLOBAL;
  }

  if (can_assign && match(compiler, TOKEN_EQUAL))
  {
    expression(compiler);
    emit_bytes(compiler, set_op, (uint8_t)arg);
  }
  else
  {
    emit_bytes(compiler, get_op, (uint8_t)arg);
  }
}

int resolve_local(Compiler *compiler, Token *name)
{
  for (int i = compiler->local_count - 1; i >= 0; --i)
  {
    Local *local = &compiler->locals[i];
    if (identifiers_equal(&local->name, name))
    {
      if (local->depth == -1)
      {
        error(compiler, "Cannot read local variable in its own initializer.");
      }
      else
      {
        return i;
      }
    }
  }

  return -1;
}

uint8_t parse_variable(Compiler *compiler, const char *msg)
{
  consume(compiler, TOKEN_IDENTIFIER, msg);
  declare_variable(compiler);
  if (compiler->scope_depth > 0)
    return 0;
  return identifier_constant(compiler, &compiler->parser->previous);
}

uint8_t identifier_constant(Compiler *compiler, Token *name)
{
  return make_constant(compiler, object_val((Obj *)copy_string(compiler->vm, name->start, name->length)));
}
