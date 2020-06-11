#include "lox/LoxClass.h"
#include "lox/LoxFunction.h"
#include "lox/LoxInstance.h"

LoxClass::LoxClass(
    const std::string& name,
    std::shared_ptr<LoxClass> superclass,
    std::map<std::string, std::shared_ptr<LoxObject>> methods) :
  LoxCallable(LoxObject::CLASS),
  name(name),
  superclass(superclass),
  methods(methods)
{}

std::shared_ptr<LoxObject> LoxClass::call(
  Interpreter* interpreter,
  std::vector<std::shared_ptr<LoxObject>>& args)
{
  auto instance = std::make_shared<LoxInstance>(this);

  if (auto init = findMethod("init"))
  {
    if (auto fn = std::static_pointer_cast<LoxFunction>(init))
    {
      fn->bind(instance)->call(interpreter, args);
    }
  }

  return instance;
}

unsigned LoxClass::arity() const
{
  if (auto init = findMethod("init"))
  {
    return std::static_pointer_cast<LoxCallable>(init)->arity();
  }

  return 0;
}

std::string LoxClass::getName() const
{
  return name;
}

std::shared_ptr<LoxObject> LoxClass::findMethod(const std::string& name) const
{
  auto method = methods.find(name);
  if (method != methods.end())
  {
    return method->second;
  }

  // walk up the class chain if method not found
  if (superclass != nullptr)
  {
    return superclass->findMethod(name);
  }

  return nullptr;
}

std::shared_ptr<LoxObject> LoxClass::findMethod(const Token& name) const
{
  return findMethod(name.lexeme);
}

std::string LoxClass::str() const
{
  return "<class " + name + ">";
}

bool LoxClass::isEqual(std::shared_ptr<LoxObject> arg0) const
{
  if (arg0 == nullptr)
  {
    return false;
  }
  else
  {
    return std::static_pointer_cast<LoxClass>(arg0).get() == this;
  }
}
