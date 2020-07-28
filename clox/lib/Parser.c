#include "Parser.h"
#include "Compiler.h"
#include "Scanner.h"
#include <stdio.h>

void init_parser(Parser *parser, Scanner *scanner)
{
  parser->scanner = scanner;
  parser->had_error = false;
  parser->panic_mode = false;
}

void advance(Compiler *compiler)
{
  compiler->parser->previous = compiler->parser->current;

  for (;;)
  {
    compiler->parser->current = scan_token(compiler->parser->scanner);
    if (compiler->parser->current.type != TOKEN_ERROR)
      break;

    error_at_current(compiler, compiler->parser->current.start);
  }
}

bool check(Compiler *compiler, TokenType type)
{
  return compiler->parser->current.type == type;
}

bool match(Compiler *compiler, TokenType type)
{
  if (check(compiler, type))
  {
    advance(compiler);
    return true;
  }

  return false;
}

void consume(Compiler *compiler, TokenType type, const char *message)
{
  if (check(compiler, type))
  {
    advance(compiler);
    return;
  }

  error_at_current(compiler, message);
}

void synchronize(Compiler *compiler)
{
  compiler->parser->panic_mode = false;

  while (!check(compiler, TOKEN_EOF))
  {
    if (compiler->parser->previous.type == TOKEN_SEMICOLON)
      return;

    switch (compiler->parser->current.type)
    {
    case TOKEN_CLASS:
    case TOKEN_FUN:
    case TOKEN_VAR:
    case TOKEN_FOR:
    case TOKEN_IF:
    case TOKEN_WHILE:
    case TOKEN_PRINT:
    case TOKEN_RETURN:
      return;

    default:
        // Do nothing.
        ;
    }

    advance(compiler);
  }
}

void error_at_current(Compiler *compiler, const char *message)
{
  error_at(compiler, &compiler->parser->current, message);
}

void error(Compiler *compiler, const char *message)
{
  error_at(compiler, &compiler->parser->previous, message);
}

void error_at(Compiler *compiler, Token *token, const char *message)
{
  if (compiler->parser->panic_mode)
    return;
  compiler->parser->panic_mode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF)
  {
    fprintf(stderr, " at end");
  }
  else if (token->type == TOKEN_ERROR)
  {
    // Nothing.
  }
  else
  {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  compiler->parser->had_error = true;
}

ParseRule *get_rule(TokenType type) { return &rules[type]; }

void parse_precedence(Compiler *compiler, Precedence precedence)
{
  advance(compiler);

  ParseFn prefix_rule = get_rule(compiler->parser->previous.type)->prefix;
  if (prefix_rule == NULL)
  {
    error(compiler, "Expected expression.");
    return;
  }

  bool can_assign = precedence <= PREC_ASSIGNMENT;
  prefix_rule(compiler, can_assign);

  while (precedence <= get_rule(compiler->parser->current.type)->precedence)
  {
    advance(compiler);
    ParseFn infix_rule = get_rule(compiler->parser->previous.type)->infix;
    infix_rule(compiler, can_assign);
  }

  if (can_assign && match(compiler, TOKEN_EQUAL))
  {
    error(compiler, "Invalid assignment target.");
  }
}

ParseRule rules[] = {
    {grouping, call, PREC_CALL},     // TOKEN_LEFT_PAREN
    {NULL, NULL, PREC_NONE},         // TOKEN_RIGHT_PAREN
    {NULL, NULL, PREC_NONE},         // TOKEN_LEFT_BRACE
    {NULL, NULL, PREC_NONE},         // TOKEN_RIGHT_BRACE
    {NULL, NULL, PREC_NONE},         // TOKEN_COMMA
    {NULL, dot, PREC_CALL},          // TOKEN_DOT
    {unary, binary, PREC_TERM},      // TOKEN_MINUS
    {NULL, binary, PREC_TERM},       // TOKEN_PLUS
    {NULL, NULL, PREC_NONE},         // TOKEN_SEMICOLON
    {NULL, binary, PREC_FACTOR},     // TOKEN_SLASH
    {NULL, binary, PREC_FACTOR},     // TOKEN_STAR
    {unary, NULL, PREC_NONE},        // TOKEN_BANG
    {NULL, binary, PREC_COMPARISON}, // TOKEN_BANG_EQUAL
    {NULL, NULL, PREC_NONE},         // TOKEN_EQUAL
    {NULL, binary, PREC_EQUALITY},   // TOKEN_EQUAL_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER
    {NULL, binary, PREC_COMPARISON}, // TOKEN_GREATER_EQUAL
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS
    {NULL, binary, PREC_COMPARISON}, // TOKEN_LESS_EQUAL
    {variable, NULL, PREC_NONE},     // TOKEN_IDENTIFIER
    {string, NULL, PREC_NONE},       // TOKEN_STRING
    {number, NULL, PREC_NONE},       // TOKEN_NUMBER
    {NULL, logical_and, PREC_AND},   // TOKEN_AND
    {NULL, NULL, PREC_NONE},         // TOKEN_CLASS
    {NULL, NULL, PREC_NONE},         // TOKEN_ELSE
    {literal, NULL, PREC_NONE},      // TOKEN_FALSE
    {NULL, NULL, PREC_NONE},         // TOKEN_FOR
    {NULL, NULL, PREC_NONE},         // TOKEN_FUN
    {NULL, NULL, PREC_NONE},         // TOKEN_IF
    {literal, NULL, PREC_NONE},      // TOKEN_NIL
    {NULL, logical_or, PREC_OR},     // TOKEN_OR
    {NULL, NULL, PREC_NONE},         // TOKEN_PRINT
    {NULL, NULL, PREC_NONE},         // TOKEN_RETURN
    {NULL, NULL, PREC_NONE},         // TOKEN_SUPER
    {NULL, NULL, PREC_NONE},         // TOKEN_THIS
    {literal, NULL, PREC_NONE},      // TOKEN_TRUE
    {NULL, NULL, PREC_NONE},         // TOKEN_VAR
    {NULL, NULL, PREC_NONE},         // TOKEN_WHILE
    {NULL, NULL, PREC_NONE},         // TOKEN_ERROR
    {NULL, NULL, PREC_NONE},         // TOKEN_EOF
};
