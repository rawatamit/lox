#include "Scanner.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static char advance(Scanner *scanner);
static char peek(Scanner *scanner);
static char peek_next(Scanner *scanner);
static void skip_whitespace(Scanner *scanner);
static bool is_at_end(Scanner *scanner);
static bool match(Scanner *scanner, char c);
static Token make_token(Scanner *scanner, TokenType type);
static Token error_token(Scanner *scanner, const char *msg);
static Token string(Scanner *scanner);
static Token number(Scanner *scanner);
static Token identifier(Scanner *scanner);
static TokenType identifier_type(Scanner *scanner);
static TokenType check_keyword(Scanner *scanner, int start, int length,
                               const char *rest, TokenType type);

void init_scanner(Scanner *scanner, const char *src) {
  scanner->start = src;
  scanner->current = src;
  scanner->line = 1;
}

Token scan_token(Scanner *scanner) {
  skip_whitespace(scanner);

  scanner->start = scanner->current;

  if (is_at_end(scanner))
    return make_token(scanner, TOKEN_EOF);

  char c = advance(scanner);

  if (isalpha(c))
    return identifier(scanner);
  if (isdigit(c))
    return number(scanner);

  switch (c) {
  case '(':
    return make_token(scanner, TOKEN_LEFT_PAREN);
  case ')':
    return make_token(scanner, TOKEN_RIGHT_PAREN);
  case '{':
    return make_token(scanner, TOKEN_LEFT_BRACE);
  case '}':
    return make_token(scanner, TOKEN_RIGHT_BRACE);
  case ';':
    return make_token(scanner, TOKEN_SEMICOLON);
  case ',':
    return make_token(scanner, TOKEN_COMMA);
  case '.':
    return make_token(scanner, TOKEN_DOT);
  case '-':
    return make_token(scanner, TOKEN_MINUS);
  case '+':
    return make_token(scanner, TOKEN_PLUS);
  case '*':
    return make_token(scanner, TOKEN_STAR);
  case '/':
    return make_token(scanner, TOKEN_SLASH);
  case '!':
    return make_token(scanner,
                      match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
  case '=':
    return make_token(scanner,
                      match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
  case '<':
    return make_token(scanner,
                      match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
  case '>':
    return make_token(scanner, match(scanner, '=') ? TOKEN_GREATER_EQUAL
                                                   : TOKEN_GREATER);
  case '"':
    return string(scanner);
  default:
    break;
  }

  return error_token(scanner, "Unexpected character.");
}

char advance(Scanner *scanner) { return *scanner->current++; }

char peek(Scanner *scanner) { return *scanner->current; }

char peek_next(Scanner *scanner) {
  if (is_at_end(scanner))
    return '\0';
  return scanner->current[1];
}

bool is_at_end(Scanner *scanner) { return *scanner->current == '\0'; }

bool match(Scanner *scanner, char c) {
  if (is_at_end(scanner))
    return false;
  if (*scanner->current != c)
    return false;
  ++scanner->current;
  return true;
}

void skip_whitespace(Scanner *scanner) {
  for (;;) {
    char c = peek(scanner);
    switch (c) {
    case ' ':
    case '\t':
    case '\r':
      advance(scanner);
      break;
    case '\n':
      ++scanner->line;
      advance(scanner);
      break;
    case '/':
      if (peek_next(scanner) == '/') {
        // A comment goes until the end of the line.
        while (peek(scanner) != '\n' && !is_at_end(scanner))
          advance(scanner);
      } else {
        return;
      }
      break;
    default:
      return;
    }
  }
}

Token string(Scanner *scanner) {
  while (peek(scanner) != '"' && !is_at_end(scanner)) {
    if (peek(scanner) == '\n')
      ++scanner->line;
    advance(scanner);
  }

  if (is_at_end(scanner))
    return error_token(scanner, "Unterminated string.");

  // The closing quote.
  advance(scanner);
  return make_token(scanner, TOKEN_STRING);
}

Token number(Scanner *scanner) {
  while (isdigit(peek(scanner)))
    advance(scanner);

  // Look for a fractional part.
  if (peek(scanner) == '.' && isdigit(peek_next(scanner))) {
    // Consume the ".".
    advance(scanner);

    while (isdigit(peek(scanner)))
      advance(scanner);
  }

  return make_token(scanner, TOKEN_NUMBER);
}

Token identifier(Scanner *scanner) {
  while (isalpha(peek(scanner)) || isdigit(peek(scanner)))
    advance(scanner);

  return make_token(scanner, identifier_type(scanner));
}

TokenType identifier_type(Scanner *scanner) {
  switch (scanner->start[0]) {
  case 'a':
    return check_keyword(scanner, 1, 2, "nd", TOKEN_AND);
  case 'c':
    return check_keyword(scanner, 1, 4, "lass", TOKEN_CLASS);
  case 'e':
    return check_keyword(scanner, 1, 3, "lse", TOKEN_ELSE);
  case 'f':
    if (scanner->current - scanner->start > 1) {
      switch (scanner->start[1]) {
      case 'a':
        return check_keyword(scanner, 2, 3, "lse", TOKEN_FALSE);
      case 'o':
        return check_keyword(scanner, 2, 1, "r", TOKEN_FOR);
      case 'u':
        return check_keyword(scanner, 2, 1, "n", TOKEN_FUN);
      }
    }
    break;
  case 'i':
    return check_keyword(scanner, 1, 1, "f", TOKEN_IF);
  case 'n':
    return check_keyword(scanner, 1, 2, "il", TOKEN_NIL);
  case 'o':
    return check_keyword(scanner, 1, 1, "r", TOKEN_OR);
  case 'p':
    return check_keyword(scanner, 1, 4, "rint", TOKEN_PRINT);
  case 'r':
    return check_keyword(scanner, 1, 5, "eturn", TOKEN_RETURN);
  case 's':
    return check_keyword(scanner, 1, 4, "uper", TOKEN_SUPER);
  case 't':
    if (scanner->current - scanner->start > 1) {
      switch (scanner->start[1]) {
      case 'h':
        return check_keyword(scanner, 2, 2, "is", TOKEN_THIS);
      case 'r':
        return check_keyword(scanner, 2, 2, "ue", TOKEN_TRUE);
      }
    }
    break;
  case 'v':
    return check_keyword(scanner, 1, 2, "ar", TOKEN_VAR);
  case 'w':
    return check_keyword(scanner, 1, 4, "hile", TOKEN_WHILE);
  }

  return TOKEN_IDENTIFIER;
}

TokenType check_keyword(Scanner *scanner, int start, int length,
                        const char *rest, TokenType type) {
  if (scanner->current - scanner->start == start + length &&
      memcmp(scanner->start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}

Token make_token(Scanner *scanner, TokenType type) {
  Token token = {.type = type,
                 .start = scanner->start,
                 .line = scanner->line,
                 .length = (int)(scanner->current - scanner->start)};
  return token;
}

Token error_token(Scanner *scanner, const char *msg) {
  Token token = {.type = TOKEN_ERROR,
                 .start = msg,
                 .line = scanner->line,
                 .length = (int)strlen(msg)};
  return token;
}
