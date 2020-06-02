#include "Parser.h"
#include "ErrorHandler.h"
#include <vector>

using namespace lox;

ParseError::ParseError(std::string msg, Token token)
    : std::runtime_error(msg)
    , token_(token) {}

Parser::Parser(const std::vector<Token>& tokens, ErrorHandler& errorHandler)
    : current(0)
    , tokens_(tokens)
    , errorHandler_(errorHandler) {}

Stmt* Parser::declaration()
{
  try
  {
    if (match({TokenType::VAR}))
    {
        return varDeclaration();
    }
    else
    {
        return statement();
    }
  }
  catch (const ParseError& e)
  {
    advance();
    return nullptr;
  }
}

Stmt* Parser::varDeclaration()
{
  Token name =
    consume(
      TokenType::IDENTIFIER,
      "expect variable name.");
  
  Expr* init = nullptr;
  if (match({TokenType::EQUAL}))
  {
    init = expression();
  }

  consume(TokenType::SEMICOLON, "expect ';' in var init.");
  return new Var(name, init);
}

Stmt* Parser::statement()
{
    if (match({TokenType::PRINT}))
    {
        return printStatement();
    }
    else if (match({TokenType::LEFT_BRACE}))
    {
      return blockStatement();
    }
    else
    {
        return expressionStatement();
    }
}

Stmt* Parser::printStatement()
{
    Expr* val = expression();
    consume(TokenType::SEMICOLON, "expected ';' after print.");
    return new Print(val);
}

Stmt* Parser::blockStatement()
{
  std::vector<Stmt*> stmts;

  while (! check(TokenType::RIGHT_BRACE) and ! isAtEnd())
  {
    stmts.push_back(declaration());
  }

  consume(TokenType::RIGHT_BRACE, "expected '}' after block");
  return new Block(stmts);
}

Stmt* Parser::expressionStatement()
{
    Expr* val = expression();
    consume(TokenType::SEMICOLON, "expected ';' after expression.");
    return new Expression(val);
}

Expr* Parser::expression() {
    return assignment();
}

Expr* Parser::assignment() {
  Expr* e = equality();

  if (match({TokenType::EQUAL}))
  {
    Token equals = previous();
    Expr* value = assignment();

    if (auto evar = dynamic_cast<Variable*>(e))
    {
      Token name = evar->name;
      return new Assign(name, value);
    }

    error(equals, "invalid assignment target.");
  }

  return e;
}

Expr* Parser::equality() {
    Expr* expr = comparison();
    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token Operator = previous();
        Expr* right    = comparison();
        expr           = new BinaryExpr(expr, Operator, right);
    }
    return expr;
}

Expr* Parser::comparison() {
    Expr* expr = term();
    while (
        match({TokenType::GREATER, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token Operator = previous();
        Expr* right    = term();
        expr           = new BinaryExpr(expr, Operator, right);
    }
    return expr;
}

Expr* Parser::term() {
    Expr* expr = factor();
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token Operator = previous();
        Expr* right    = factor();
        expr           = new BinaryExpr(expr, Operator, right);
    }
    return expr;
}

Expr* Parser::factor() {
    Expr* expr = unary();
    while (match({TokenType::SLASH, TokenType::STAR})) {
        Token Operator = previous();
        Expr* right    = unary();
        expr           = new BinaryExpr(expr, Operator, right);
    }
    return expr;
}

Expr* Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS})) {
        Token Operator = previous();
        Expr* right    = unary();
        return new UnaryExpr(Operator, right);
    }
    return primary();
}

Expr* Parser::primary() {
    if (match({TokenType::FALSE}))
        return new LiteralExpr(TokenType::FALSE, "false");
    if (match({TokenType::TRUE}))
        return new LiteralExpr(TokenType::TRUE, "true");
    if (match({TokenType::NIL}))
        return new LiteralExpr(TokenType::NIL, "nil");
    if (match({TokenType::NUMBER, TokenType::STRING}))
        return new LiteralExpr(previous().type, previous().literal);
    if (match({TokenType::LEFT_PAREN})) {
        Expr* expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return new GroupingExpr(expr);
    }
    if (match({TokenType::IDENTIFIER})) {
      return new Variable(previous());
    }
    throw error(peek(), "Expect expression.");
    return nullptr;
}

std::vector<Stmt*> Parser::parse() {
    std::vector<Stmt*> stmts;

    while (!isAtEnd())
    {
        stmts.push_back(declaration());
    }

    return stmts;
}

Token Parser::consume(TokenType type, std::string message) {
    if (check(type))
        return advance();
    throw error(peek(), message);
}

ParseError Parser::error(Token token, std::string message) {
    if (token.type == TokenType::END_OF_FILE) {
        errorHandler_.add(token.line, " at end", message);
    } else {
        errorHandler_.add(token.line, "at '" + token.lexeme + "'", message);
    }
    errorHandler_.report();
    return *new ParseError(message, token);
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::previous() {
    return tokens_[current - 1];
}

Token Parser::advance() {
    if (!isAtEnd())
        ++current;
    return previous();
}

Token Parser::peek() {
    return tokens_[current];
}

bool Parser::isAtEnd() {
    return peek().type == TokenType::END_OF_FILE;
}

bool Parser::check(TokenType type) {
    if (isAtEnd())
        return false;
    return peek().type == type;
}
