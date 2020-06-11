#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <map>
#include <string>
#include <any>
#include <memory>
#include <ostream>

#include "Token.h"

namespace lox
{

class Environment
{
private:
  std::shared_ptr<Environment> enclosing;
  std::map<std::string, std::any> values;

  Environment* ancestor(int depth);

public:
  Environment() = default;
  Environment(std::shared_ptr<Environment> enclosing) :
    enclosing(enclosing)
  {}
  ~Environment() = default;

  int depth() const;
  void define(std::string k, std::any v);
  void assign(Token name, std::any v);
  std::any get(Token name);
  std::any get(int depth, const Token& name);
  std::any get(int depth, const std::string& name);
  void assign(int depth, const lox::Token& name, std::any v);
  void print(std::ostream& out);
};

} // namespace

#endif
