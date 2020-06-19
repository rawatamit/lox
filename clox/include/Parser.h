#ifndef _PARSER_H_
#define _PARSER_H_

#include "Scanner.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct VM VM;
typedef struct Compiler Compiler;

struct Parser {
  Scanner *scanner;
  Token current;
  Token previous;
  bool had_error;
  bool panic_mode;
};

typedef struct Parser Parser;

enum Precedence {
  PREC_NONE,
  PREC_ASSIGNMENT, // =
  PREC_OR,         // or
  PREC_AND,        // and
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_TERM,       // + -
  PREC_FACTOR,     // * /
  PREC_UNARY,      // ! -
  PREC_CALL,       // . ()
  PREC_PRIMARY
};

typedef enum Precedence Precedence;

typedef void (*ParseFn)(Compiler *compiler, bool can_assign);

struct ParseRule {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

typedef struct ParseRule ParseRule;

extern ParseRule rules[];

void init_parser(Parser *parser, Scanner *scanner);
void declaration(Compiler *compiler);
void var_declaration(Compiler *compiler);
void statement(Compiler *compiler);
void if_statement(Compiler *compiler);
void while_statement(Compiler *compiler);
void for_statement(Compiler *compiler);
void expression_statement(Compiler *compiler);
void print_statement(Compiler *compiler);
void block(Compiler *compiler);
void expression(Compiler *compiler);
void binary(Compiler *compiler, bool can_assign);
void grouping(Compiler *compiler, bool can_assign);
void logical_and(Compiler *compiler, bool can_assign);
void logical_or(Compiler *compiler, bool can_assign);
void unary(Compiler *compiler, bool can_assign);
void number(Compiler *compiler, bool can_assign);
void string(Compiler *compiler, bool can_assign);
void literal(Compiler *compiler, bool can_assign);
void variable(Compiler *compiler, bool can_assign);
uint8_t parse_variable(Compiler *compiler, const char *msg);
uint8_t identifier_constant(Compiler *compiler, Token *name);
void advance(Compiler *compiler);
bool check(Compiler *compiler, TokenType type);
bool match(Compiler *compiler, TokenType type);
void consume(Compiler *compiler, TokenType type, const char *message);
void synchronize(Compiler *compiler);
ParseRule *get_rule(TokenType type);
void parse_precedence(Compiler *compiler, Precedence precedence);
void error_at_current(Compiler *compiler, const char *message);
void error(Compiler *compiler, const char *message);
void error_at(Compiler *compiler, Token *token, const char *message);

#endif
