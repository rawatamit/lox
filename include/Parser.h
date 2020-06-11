#ifndef PARSER_HPP
#define PARSER_HPP

#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include <memory>
#include <stdexcept>
#include <vector>

namespace lox {
// forward declarations
class ErrorHandler;

class ParseError : public std::runtime_error {
public:
  ParseError(std::string msg, Token token);
  Token token_;
};

class Parser {
public:
  Parser(const std::vector<Token> &tokens, ErrorHandler &errorHandler);
  size_t current;
  std::shared_ptr<Stmt> declaration();
  std::shared_ptr<Stmt> classDecl();
  std::shared_ptr<Function> function(const std::string &kind);
  std::shared_ptr<Stmt> varDeclaration();
  std::shared_ptr<Stmt> statement();
  std::shared_ptr<Stmt> ifStatement();
  std::shared_ptr<Stmt> printStatement();
  std::shared_ptr<Stmt> whileStatement();
  std::shared_ptr<Stmt> forStatement();
  std::shared_ptr<Stmt> blockStatement();
  std::shared_ptr<Stmt> expressionStatement();
  std::shared_ptr<Stmt> returnStatement();
  std::shared_ptr<Expr> expression();
  std::shared_ptr<Expr> assignment();
  std::shared_ptr<Expr> logic_or();
  std::shared_ptr<Expr> logic_and();
  std::shared_ptr<Expr> equality();
  std::shared_ptr<Expr> comparison();
  std::shared_ptr<Expr> term();
  std::shared_ptr<Expr> factor();
  std::shared_ptr<Expr> call();
  std::shared_ptr<Expr> finishCall(std::shared_ptr<Expr> e);
  std::shared_ptr<Expr> unary();
  std::shared_ptr<Expr> primary();
  std::vector<std::shared_ptr<Stmt>> parse();
  ParseError error(Token token, std::string message);

private:
  bool match(const std::vector<TokenType> &types);
  Token previous();
  Token advance();
  Token peek();
  bool isAtEnd();
  bool check(TokenType type);
  Token consume(TokenType type, const std::string &message);
  void synchronize();
  std::vector<Token> tokens_;
  ErrorHandler &errorHandler_;
};
} // namespace lox

#endif // PARSER_HPP
