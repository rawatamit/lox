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
        Parser(const std::vector<Token>& tokens, ErrorHandler& errorHandler);
        size_t current;
        Stmt* declaration();
        Stmt* function(const std::string& kind);
        Stmt* varDeclaration();
        Stmt* statement();
        Stmt* ifStatement();
        Stmt* printStatement();
        Stmt* whileStatement();
        Stmt* forStatement();
        Stmt* blockStatement();
        Stmt* expressionStatement();
        Stmt* returnStatement();
        Expr* expression();
        Expr* assignment();
        Expr* logic_or();
        Expr* logic_and();
        Expr* equality();
        Expr* comparison();
        Expr* term();
        Expr* factor();
        Expr* call();
        Expr* finishCall(Expr* e);
        Expr* unary();
        Expr* primary();
        std::vector<Stmt*> parse();
        ParseError error(Token token, std::string message);

      private:
        bool match(const std::vector<TokenType>& types);
        Token previous();
        Token advance();
        Token peek();
        bool isAtEnd();
        bool check(TokenType type);
        Token consume(TokenType type, std::string message);
        void synchronize();
        std::vector<Token> tokens_;
        ErrorHandler& errorHandler_;
    };
}

#endif // PARSER_HPP
