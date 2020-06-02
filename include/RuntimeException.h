#ifndef _REXCEPT_H_
#define _REXCEPT_H_

#include "Token.h"
#include <stdexcept>

namespace lox
{

class RuntimeException : public std::runtime_error
{
public:
  Token tok;

  RuntimeException(const std::string& msg) :
    std::runtime_error(msg), tok(Token(TokenType::ERROR, "error", "", -1))
  {}

  RuntimeException(Token tok, const std::string& msg) :
    std::runtime_error(msg), tok(tok)
  {}
};

} // namespace

#endif
