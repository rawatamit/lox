#include "Environment.h"
#include "RuntimeException.h"

void lox::Environment::define(std::string k, std::any v)
{
  values[k] = v;
}

void lox::Environment::assign(lox::Token name, std::any v)
{

  Environment* current = this;

  while (current != nullptr)
  {
    auto it = current->values.find(name.lexeme);
    if (it != current->values.end())
    {
      it->second = v;
      return;
    }
    else
    {
      current = current->enclosing.get();
    }
  }

  throw new RuntimeException(name,
      "assign to unknown variable " + name.lexeme);
}

std::any lox::Environment::get(lox::Token name)
{
  Environment* current = this;

  while (current != nullptr)
  {
    auto it = current->values.find(name.lexeme);
    if (it != current->values.end())
    {
      return it->second;
    }
    else
    {
      current = current->enclosing.get();
    }
  }

  throw new RuntimeException(name,
      "undefined variable " + name.lexeme);
}
