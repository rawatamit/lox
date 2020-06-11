#ifndef _LOXINSTANCE_H_
#define _LOXINSTANCE_H_

#include <map>
#include <any>
#include <string>

#include "LoxClass.h"
#include "LoxFunction.h"
#include "RuntimeException.h"

namespace lox
{

class LoxInstance
{
private:
  LoxClass* klass;
  std::map<std::string, std::any> fields;

public:
  LoxInstance(LoxClass* klass) :
    klass(klass)
  {}

  ~LoxInstance() = default;

  std::any get(const Token& name)
  {
    auto field = fields.find(name.lexeme);
    if (field != fields.end())
    {
      return field->second;
    }

    auto method = klass->findMethod(name);
    if (method.type() != typeid(std::nullptr_t))
    {
      auto fn = std::any_cast<LoxCallable*>(method);
      static_cast<LoxFunction*>(fn)->bind(this);
      return fn;
    }

    throw RuntimeException(name,
      "Undefined property '" + name.lexeme + "'.");
  }

  void set(const Token& name, std::any v)
  {
    fields[name.lexeme] = v;
  }

  std::string str() const
  {
    return "class instance";
  }
};

} // namespace

#endif
