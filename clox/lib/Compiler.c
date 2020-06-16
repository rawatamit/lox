#include "Compiler.h"
#include "Opcode.h"
#include "Parser.h"
#include "Scanner.h"
#include "VM.h"
#include <stdlib.h>

#ifdef DEBUG_PRINT_CODE
#include "Debug.h"
#endif

Chunk *compiling_chunk;
static Chunk *current_chunk() { return compiling_chunk; }

bool compile(const char *src, Chunk *chunk) {
  Scanner scanner;
  init_scanner(&scanner, src);

  Parser parser;
  init_parser(&parser, &scanner);

  compiling_chunk = chunk;
  advance(&parser);
  expression(&parser);
  consume(&parser, TOKEN_EOF, "Expected end of expression.");
  end_compiler(&parser);
  return !parser.had_error;
}

void emit_byte(Parser *parser, uint8_t byte) {
  write_chunk(current_chunk(), byte, parser->previous.line);
}

void emit_bytes(Parser *parser, uint8_t byte1, uint8_t byte2) {
  emit_byte(parser, byte1);
  emit_byte(parser, byte2);
}

void emit_constant(Parser *parser, Value value) {
  emit_bytes(parser, OP_CONSTANT, make_constant(parser, value));
}

void emit_return(Parser *parser) { emit_byte(parser, OP_RETURN); }

uint8_t make_constant(Parser *parser, Value value) {
  size_t index = add_constant(current_chunk(), value);

  if (index > UINT8_MAX) {
    error(parser, "Too many constants in one chunk.");
    return 0;
  }

  return (uint8_t)index;
}

void end_compiler(Parser *parser) {
  emit_return(parser);

#ifdef DEBUG_PRINT_CODE
  if (!parser->had_error) {
    disassemble_chunk(current_chunk(), "code");
  }
#endif
}

void expression(Parser *parser) { parse_precedence(parser, PREC_ASSIGNMENT); }

void grouping(Parser *parser) {
  expression(parser);
  consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

void unary(Parser *parser) {
  TokenType op = parser->previous.type;

  // operand
  parse_precedence(parser, PREC_UNARY);

  switch (op) {
  case TOKEN_MINUS:
    emit_byte(parser, OP_NEGATE);
    break;
  default:
    break;
  }
}

void binary(Parser *parser) {
  TokenType op = parser->previous.type;

  // compile right operand
  ParseRule *rule = get_rule(op);
  parse_precedence(parser, (Precedence)(rule->precedence + 1));

  // emit op instruction
  switch (op) {
  case TOKEN_PLUS:
    emit_byte(parser, OP_ADD);
    break;
  case TOKEN_MINUS:
    emit_byte(parser, OP_SUBTRACT);
    break;
  case TOKEN_SLASH:
    emit_byte(parser, OP_DIVIDE);
    break;
  case TOKEN_STAR:
    emit_byte(parser, OP_MULTIPLY);
    break;
  default:
    break;
  }
}

void number(Parser *parser) {
  double value = strtod(parser->previous.start, NULL);
  emit_constant(parser, value);
}
