#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

namespace lox {
    enum class TokenType {
        // Single-character tokens.
        LEFT_PAREN = 0,
        RIGHT_PAREN,
        LEFT_BRACE,
        RIGHT_BRACE,
        COMMA,
        DOT = 5,
        MINUS,
        PLUS,
        SEMICOLON,
        SLASH,
        STAR = 10,

        // One or two character tokens.
        BANG,
        BANG_EQUAL,
        EQUAL,
        EQUAL_EQUAL,
        GREATER = 15,
        GREATER_EQUAL,
        LESS,
        LESS_EQUAL = 18,

        // Literals.
        IDENTIFIER, // user-defined (e.g. variable/type name) or
                    // language-defined (reserved keyword)
        STRING,
        NUMBER = 21,

        // Reserved Keywords.
        // Reserved keywords ARE identifiers but have seperate token types
        AND,
        CLASS,
        ELSE,
        FALSE = 25,
        FUN,
        FOR,
        IF,
        NIL,
        OR = 30,
        PRINT,
        RETURN,
        SUPER,
        THIS,
        TRUE = 35,
        VAR,
        WHILE,

        ERROR,
        END_OF_FILE = 39
    };

    class Token {
      public:
        Token(TokenType aType, const std::string& aLexeme,
              const std::string& aLiteral, int aLine);
        std::string toString() const;
        std::string lexeme;
        // @brief literal can be of 3 types: string, number, or identifier
        // number literals are tricky and it may seem odd that i'm storing them
        // in a string here, but having a "polymorphic" type for literal is more
        // work which is why i'm using a string for now and will convert to
        // number if needed.
        std::string literal;
        TokenType type;
        int line;
    };
}

#endif // TOKEN_HPP
