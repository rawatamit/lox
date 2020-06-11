#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include <map>
#include <string>
#include <any>
#include <memory>
#include <ostream>

#include "Token.h"
#include "lox/LoxObject.h"

namespace lox
{

class Environment
{
private:
  std::shared_ptr<Environment> enclosing;
  std::map<std::string, std::shared_ptr<LoxObject>> values;

  Environment* ancestor(int depth);

public:
  Environment() = default;
  Environment(std::shared_ptr<Environment> enclosing) :
    enclosing(enclosing)
  {}
  ~Environment() = default;

  int depth() const;
  std::shared_ptr<Environment> getEnclosing() const;
  void define(std::string k, std::shared_ptr<LoxObject> v);
  void assign(Token name, std::shared_ptr<LoxObject> v);
  std::shared_ptr<LoxObject> get(Token name);
  std::shared_ptr<LoxObject> get(int depth, const Token& name);
  std::shared_ptr<LoxObject> get(int depth, const std::string& name);
  void assign(int depth, const lox::Token& name, std::shared_ptr<LoxObject> v);
  void print(std::ostream& out);
};

} // namespace

#endif
