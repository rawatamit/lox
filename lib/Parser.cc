#include "Parser.h"
#include "ErrorHandler.h"
#include <vector>
#include <algorithm>

using namespace lox;

ParseError::ParseError(std::string msg, Token token) :
  std::runtime_error(msg),
  token_(token)
{}

Parser::Parser(const std::vector<Token>& tokens, ErrorHandler& errorHandler) :
  current(0),
  tokens_(tokens),
  errorHandler_(errorHandler)
{}

Stmt* Parser::declaration()
{
  try
  {
    if (match({TokenType::VAR}))
    {
        return varDeclaration();
    }
    else if (match({TokenType::FUN}))
    {
      return function("function");
    }
    else if (match({TokenType::CLASS}))
    {
      return classDecl();
    }
    else
    {
        return statement();
    }
  }
  catch (const ParseError& e)
  {
    synchronize();
    return nullptr;
  }
}

Stmt* Parser::classDecl()
{
  Token name =
    consume(
      TokenType::IDENTIFIER,
      "Expect class name.");
  consume(TokenType::LEFT_BRACE, "Expect '{' before class body.");

  std::vector<Function*> methods;
  while (!check(TokenType::RIGHT_BRACE) and !isAtEnd())
  {
    methods.push_back(function("method"));
  }

  consume(TokenType::RIGHT_BRACE, "Expect '}' after class body.");
  return new Class(name, methods);
}

Function* Parser::function(const std::string& kind)
{
  Token name =
    consume(
      TokenType::IDENTIFIER,
      "expected " + kind + "name.");
  
  consume(
    TokenType::LEFT_PAREN,
    "expect '(' after " + kind + " name.");
  
  std::vector<Token> params;
  if (! check(TokenType::RIGHT_PAREN))
  {
    do {
      if (params.size() >= 255)
      {
        error(peek(), "can't have >= 255 parameters.");
      }
      else
      {
        params.push_back(
          consume(
            TokenType::IDENTIFIER,
            "Expected parameter name."));
      }
    } while (match({TokenType::COMMA}));
  }

  consume(TokenType::RIGHT_PAREN,
    "Expected ')' after parameters.");
  
  consume(TokenType::LEFT_BRACE,
    "Expected '{' before function body.");
  Block* body = static_cast<Block*>(blockStatement());
  return new Function(name, params, body->stmts);
}

Stmt* Parser::varDeclaration()
{
  Token name =
    consume(
      TokenType::IDENTIFIER,
      "Expected variable name.");
  
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
  switch (peek().type)
  {
  case TokenType::PRINT:
    match({TokenType::PRINT});
    return printStatement();
  case TokenType::WHILE:
    match({TokenType::WHILE});
    return whileStatement();
  case TokenType::FOR:
    match({TokenType::FOR});
    return forStatement();
  case TokenType::LEFT_BRACE:
    match({TokenType::LEFT_BRACE});
    return blockStatement();
  case TokenType::IF:
    match({TokenType::IF});
    return ifStatement();
  case TokenType::RETURN:
    match({TokenType::RETURN});
    return returnStatement();
  default:
    return expressionStatement();
  }
}

Stmt* Parser::ifStatement()
{
  consume(TokenType::LEFT_PAREN, "need '(' in condition for if");
  Expr* condition = expression();
  consume(TokenType::RIGHT_PAREN, "need ')' in condition for if");

  Stmt* thenBranch = statement();
  Stmt* elseBranch = nullptr;

  if (match({TokenType::ELSE}))
  {
    elseBranch = statement();
  }

  return new If(condition, thenBranch, elseBranch);
}

Stmt* Parser::printStatement()
{
    Expr* val = expression();
    consume(TokenType::SEMICOLON, "expected ';' after print.");
    return new Print(val);
}

Stmt* Parser::whileStatement()
{
  consume(TokenType::LEFT_PAREN, "need '(' in condition for while");
  Expr* condition = expression();
  consume(TokenType::RIGHT_PAREN, "need ')' in condition for while");

  Stmt* body = statement();
  return new While(condition, body);
}

Stmt* Parser::forStatement()
{
  consume(TokenType::LEFT_PAREN, "expected '(' after for");

  Stmt* init = nullptr;
  switch (peek().type)
  {
  case TokenType::VAR:
    match({TokenType::VAR});
    init = varDeclaration();
    break;
  case TokenType::SEMICOLON:
    consume(TokenType::SEMICOLON, "expected ';' in init");
    break;
  default:
    init = expressionStatement();
    break;
  }

  Expr* condition = nullptr;
  if (! check(TokenType::SEMICOLON))
  {
    condition = expression();
  }
  consume(TokenType::SEMICOLON, "expected ';' after condition");

  Expr* step = nullptr;
  if (! check(TokenType::RIGHT_PAREN))
  {
    step = expression();
  }
  consume(TokenType::RIGHT_PAREN, "expected ')' after update");

  Stmt* body = statement();

  // init
  // while (condition)
  //   body
  //   step
  if (step != nullptr)
  {
    body = new Block({body, new Expression(step)});
  }

  if (condition == nullptr)
  {
    condition = new LiteralExpr(TokenType::TRUE, "true");
  }

  body = new While(condition, body);
  return (init == nullptr) ? body : new Block({init, body});
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

Stmt* Parser::returnStatement()
{
  Token keyword = previous();
  Expr* expr = nullptr;
  if (! check(TokenType::SEMICOLON))
  {
    expr = expression();
  }
  consume(TokenType::SEMICOLON, "expected ';' after return.");
  return new Return(keyword, expr);
}

Expr* Parser::expression() {
    return assignment();
}

Expr* Parser::assignment() {
  Expr* expr = logic_or();

  if (match({TokenType::EQUAL}))
  {
    Token equals = previous();
    Expr* value = assignment();

    if (auto var = dynamic_cast<Variable*>(expr))
    {
      Token name = var->name;
      return new Assign(name, value);
    }
    else if (auto get = dynamic_cast<Get*>(expr))
    {
      return new Set(get->object, get->name, value);
    }

    error(equals, "Invalid assignment target.");
  }

  return expr;
}

Expr* Parser::logic_or() {
  Expr* e = logic_and();

  while (match({TokenType::OR}))
  {
    Token op = previous();
    Expr* r = logic_and();
    e = new Logical(e, op, r);
  }

  return e;
}

Expr* Parser::logic_and() {
  Expr* e = equality();

  while (match({TokenType::AND}))
  {
    Token op = previous();
    Expr* r = equality();
    e = new Logical(e, op, r);
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
        match({TokenType::GREATER, TokenType::LESS,
              TokenType::LESS_EQUAL, TokenType::GREATER_EQUAL}))
    {
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
    return call();
}

Expr* Parser::call() {
  Expr* e = primary(); 

  while (true)
  {
    if (match({TokenType::LEFT_PAREN}))
    {
      e = finishCall(e);
    }
    else if (match({TokenType::DOT}))
    {
      Token name =
        consume(TokenType::IDENTIFIER,
          "Expect property name after '.'.");
      e = new Get(e, name);
    }
    else
    {
      break;
    }
  }

  return e;
}

Expr* Parser::finishCall(Expr* e) {
  std::vector<Expr*> args;
  if (! check(TokenType::RIGHT_PAREN))
  {
    do {
      if (args.size() >= 255)
      {
        error(peek(), "cannot have more than 255 arguments");
      }
      else
      {
        args.push_back(expression());
      }
    } while (match({TokenType::COMMA}));
  }

  Token paren = consume(TokenType::RIGHT_PAREN, "expected ')' in call");
  return new Call(e, paren, args);
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
    if (match({TokenType::THIS}))
    {
      return new This(previous());
    }
    throw error(peek(), "Expected expression.");
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
        errorHandler_.add(token.line, " at '" + token.lexeme + "'", message);
    }
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

void Parser::synchronize()
{
  advance();

  while (!isAtEnd())
  {
    if (previous().type == TokenType::SEMICOLON) return;

    switch (peek().type)
    {
    case TokenType::CLASS:
    case TokenType::FUN:
    case TokenType::VAR:
    case TokenType::FOR:
    case TokenType::IF:
    case TokenType::WHILE:
    case TokenType::PRINT:
    case TokenType::RETURN:
      return;
    default:
      break;
    }

    advance();
  }
}
