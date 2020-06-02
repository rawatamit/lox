#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <map>
#include <string>
#include <any>

#include "Token.h"
#include <memory>

namespace lox
{

class Environment
{
private:
  std::shared_ptr<Environment> enclosing;
  std::map<std::string, std::any> values;

public:
  Environment() = default;
  Environment(std::shared_ptr<Environment> enclosing) :
    enclosing(enclosing)
  {}
  ~Environment() = default;

  void define(std::string k, std::any v);
  void assign(Token name, std::any v);
  std::any get(Token name);
};

}; // namespace

#endif
