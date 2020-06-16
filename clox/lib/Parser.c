#include "Parser.h"
#include "Scanner.h"
#include <stdio.h>

void init_parser(Parser *parser, Scanner *scanner) {
  parser->scanner = scanner;
  parser->had_error = false;
  parser->panic_mode = false;
}

void advance(Parser *parser) {
  parser->previous = parser->current;

  for (;;) {
    parser->current = scan_token(parser->scanner);
    if (parser->current.type != TOKEN_ERROR)
      break;

    error_at_current(parser, parser->current.start);
  }
}

void consume(Parser *parser, TokenType type, const char *message) {
  if (parser->current.type == type) {
    advance(parser);
    return;
  }

  error_at_current(parser, message);
}

void error_at_current(Parser *parser, const char *message) {
  error_at(parser, &parser->current, message);
}

void error(Parser *parser, const char *message) {
  error_at(parser, &parser->previous, message);
}

void error_at(Parser *parser, Token *token, const char *message) {
  if (parser->panic_mode)
    return;
  parser->panic_mode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser->had_error = true;
}

ParseRule *get_rule(TokenType type) { return &rules[type]; }

void parse_precedence(Parser *parser, Precedence precedence) {
  advance(parser);

  ParseFn prefix_rule = get_rule(parser->previous.type)->prefix;
  if (prefix_rule == NULL) {
    error(parser, "Expected expression.");
    return;
  }

  prefix_rule(parser);

  while (precedence <= get_rule(parser->current.type)->precedence) {
    advance(parser);
    ParseFn infix_rule = get_rule(parser->previous.type)->infix;
    infix_rule(parser);
  }
}

ParseRule rules[] = {
    {grouping, NULL, PREC_NONE}, // TOKEN_LEFT_PAREN
    {NULL, NULL, PREC_NONE},     // TOKEN_RIGHT_PAREN
    {NULL, NULL, PREC_NONE},     // TOKEN_LEFT_BRACE
    {NULL, NULL, PREC_NONE},     // TOKEN_RIGHT_BRACE
    {NULL, NULL, PREC_NONE},     // TOKEN_COMMA
    {NULL, NULL, PREC_NONE},     // TOKEN_DOT
    {unary, binary, PREC_TERM},  // TOKEN_MINUS
    {NULL, binary, PREC_TERM},   // TOKEN_PLUS
    {NULL, NULL, PREC_NONE},     // TOKEN_SEMICOLON
    {NULL, binary, PREC_FACTOR}, // TOKEN_SLASH
    {NULL, binary, PREC_FACTOR}, // TOKEN_STAR
    {NULL, NULL, PREC_NONE},     // TOKEN_BANG
    {NULL, NULL, PREC_NONE},     // TOKEN_BANG_EQUAL
    {NULL, NULL, PREC_NONE},     // TOKEN_EQUAL
    {NULL, NULL, PREC_NONE},     // TOKEN_EQUAL_EQUAL
    {NULL, NULL, PREC_NONE},     // TOKEN_GREATER
    {NULL, NULL, PREC_NONE},     // TOKEN_GREATER_EQUAL
    {NULL, NULL, PREC_NONE},     // TOKEN_LESS
    {NULL, NULL, PREC_NONE},     // TOKEN_LESS_EQUAL
    {NULL, NULL, PREC_NONE},     // TOKEN_IDENTIFIER
    {NULL, NULL, PREC_NONE},     // TOKEN_STRING
    {number, NULL, PREC_NONE},   // TOKEN_NUMBER
    {NULL, NULL, PREC_NONE},     // TOKEN_AND
    {NULL, NULL, PREC_NONE},     // TOKEN_CLASS
    {NULL, NULL, PREC_NONE},     // TOKEN_ELSE
    {NULL, NULL, PREC_NONE},     // TOKEN_FALSE
    {NULL, NULL, PREC_NONE},     // TOKEN_FOR
    {NULL, NULL, PREC_NONE},     // TOKEN_FUN
    {NULL, NULL, PREC_NONE},     // TOKEN_IF
    {NULL, NULL, PREC_NONE},     // TOKEN_NIL
    {NULL, NULL, PREC_NONE},     // TOKEN_OR
    {NULL, NULL, PREC_NONE},     // TOKEN_PRINT
    {NULL, NULL, PREC_NONE},     // TOKEN_RETURN
    {NULL, NULL, PREC_NONE},     // TOKEN_SUPER
    {NULL, NULL, PREC_NONE},     // TOKEN_THIS
    {NULL, NULL, PREC_NONE},     // TOKEN_TRUE
    {NULL, NULL, PREC_NONE},     // TOKEN_VAR
    {NULL, NULL, PREC_NONE},     // TOKEN_WHILE
    {NULL, NULL, PREC_NONE},     // TOKEN_ERROR
    {NULL, NULL, PREC_NONE},     // TOKEN_EOF
};