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
        Stmt* varDeclaration();
        Stmt* statement();
        Stmt* printStatement();
        Stmt* blockStatement();
        Stmt* expressionStatement();
        Expr* expression();
        Expr* assignment();
        Expr* equality();
        Expr* comparison();
        Expr* term();
        Expr* factor();
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
        ErrorHandler& errorHandler_;
        std::vector<Token> tokens_;
    };
}

#endif // PARSER_HPP
