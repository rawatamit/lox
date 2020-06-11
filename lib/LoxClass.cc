#include "LoxClass.h"
#include "LoxFunction.h"
#include "LoxInstance.h"

LoxClass::LoxClass(
  const std::string& name,
  std::map<std::string, std::any> methods) :
    name(name),
    methods(methods)
{}

std::any LoxClass::call(
  Interpreter* interpreter,
  std::vector<std::any>& args)
{
  LoxInstance* instance = new LoxInstance(this);
  auto init = findMethod("init");

  if (init.type() != typeid(std::nullptr_t))
  {
    if (LoxCallable* fn = std::any_cast<LoxCallable*>(init))
    {
      static_cast<LoxFunction*>(fn)->bind(instance)->call(interpreter, args);
    }
  }

  return instance;
}

unsigned LoxClass::arity() const
{
  auto init = findMethod("init");
  if (init.type() != typeid(std::nullptr_t))
  {
    return std::any_cast<LoxCallable*>(init)->arity();
  }

  return 0;
}

std::any LoxClass::findMethod(const std::string& name) const
{
  auto method = methods.find(name);
  return (method == methods.end()) ? nullptr : method->second;
}

std::any LoxClass::findMethod(const Token& name) const
{
  return findMethod(name.lexeme);
}

std::string LoxClass::str() const
{
  return "<class " + name + ">";
}
