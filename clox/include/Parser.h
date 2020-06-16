#ifndef _PARSER_H_
#define _PARSER_H_

#include "Scanner.h"
#include <stdbool.h>

typedef struct VM VM;

struct Parser {
  Scanner *scanner;
  // current VM, we may allocate objects
  // for a VM, so it makes sense to have it here
  VM *vm;
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

typedef void (*ParseFn)();

struct ParseRule {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
};

typedef struct ParseRule ParseRule;

extern ParseRule rules[];

void init_parser(Parser *parser, Scanner *scanner);
void expression(Parser *parser);
void binary(Parser *parser);
void grouping(Parser *parser);
void unary(Parser *parser);
void number(Parser *parser);
void string(Parser *parser);
void literal(Parser *parser);
void advance(Parser *parser);
void consume(Parser *parser, TokenType type, const char *message);
ParseRule *get_rule(TokenType type);
void parse_precedence(Parser *parser, Precedence precedence);
void error_at_current(Parser *parser, const char *message);
void error(Parser *parser, const char *message);
void error_at(Parser *parser, Token *token, const char *message);

#endif
